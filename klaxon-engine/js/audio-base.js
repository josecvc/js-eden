const importObject = {
    env: {
        memory: new WebAssembly.Memory({
                    initial: 256,
                    maximum: 256,
                    shared: true
                }),
        table: new WebAssembly.Table({ initial: 0, element: "anyfunc" }),
        abort: () => { throw new Error("WASM abort"); }
    }
};

class MixerProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.port.onmessage = (e) => {
            const msg = e.data;
            console.log(msg)
            if (msg.type === "wasm") {
                this.initProcessor(msg)
            } else if (msg.type == "sample") {
                this.addSample(msg);
            } else if (msg.type == "note_on") {
                this.playMidi(msg);
            } else if (msg.type == "note_off") {
                this.stopMidi(msg);
            } else if (msg.type == "bpm") {
                this.setBPM(msg);
            } else if (msg.type == "rows-per-beat") {
                this.setRPB(msg);
            } else if (msg.type == "playstart") {
                this.setPlay(msg);
            } else if (msg.type == "playpause") {
                this.setPause(msg);
            }
        };
        this.patterns = {};
    }

    initProcessor(msg) {
        WebAssembly.instantiate(msg.wasm, importObject)
            .then((obj) => {
                this.wasm = obj.instance
                
                this.HEAPU8 = new Uint8Array(this.wasm.exports.memory.buffer); 
                this.HEAPU32 = new Uint32Array(this.wasm.exports.memory.buffer); 
                this.HEAPF32 = new Float32Array(this.wasm.exports.memory.buffer);

                const frames = 128;
                const channels = 2; // stereo audio
                const bytes = frames * 4;

                this.enginePtr = this.wasm.exports.create_engine(msg.sample_rate, msg.bpm, msg.rows_per_beat);

                this.outputTablePtr = this.wasm.exports.malloc(channels * 4);

                this.leftPtr = this.wasm.exports.malloc(bytes);
                this.rightPtr = this.wasm.exports.malloc(bytes);
                
                this.leftHeap = new Float32Array(
                    this.wasm.exports.memory.buffer,
                    this.leftPtr,
                    frames
                );

                this.rightHeap = new Float32Array(
                    this.wasm.exports.memory.buffer,
                    this.rightPtr,
                    frames
                );

                this.HEAPU32[(this.outputTablePtr >> 2) + 0] = this.leftPtr;
                this.HEAPU32[(this.outputTablePtr >> 2) + 1] = this.rightPtr;
                
                // create data views for WebAssembly and JS
                const BYTES_PER_CELL = 5;
                const totalCells = msg.numPatterns * msg.numRows * msg.numChannels;
                const totalPatternBytes = totalCells * BYTES_PER_CELL;

                const rowBytes = msg.numPatterns * Uint16Array.BYTES_PER_ELEMENT;
                const offsetBytes = msg.numPatterns * Uint32Array.BYTES_PER_ELEMENT;
                const orderBytes = 255 * Uint8Array.BYTES_PER_ELEMENT;
                const playbackBytes = 3 * Uint16Array.BYTES_PER_ELEMENT;
                

                this.patternPtr = this.wasm.exports.malloc(totalPatternBytes);
                this.patternArray = new Uint8Array(
                    this.wasm.exports.memory.buffer, 
                    this.patternPtr, 
                    totalCells
                );

                this.rowPtr = this.wasm.exports.malloc(
                    msg.numPatterns * Uint16Array.BYTES_PER_ELEMENT
                );

                this.rowArray = new Uint16Array(
                    this.wasm.exports.memory.buffer, 
                    this.rowPtr, 
                    msg.numPatterns
                );
  
                this.offsetPtr = this.wasm.exports.malloc(
                    msg.numPatterns * Uint32Array.BYTES_PER_ELEMENT
                );
                this.offsetArray = new Uint32Array(
                    this.wasm.exports.memory.buffer, 
                    this.offsetPtr, 
                    msg.numPatterns
                );

                this.orderPtr = this.wasm.exports.malloc(
                    msg.numPatterns * Uint8Array.BYTES_PER_ELEMENT
                );
                this.orderArray = new Uint8Array(
                    this.wasm.exports.memory.buffer, 
                    this.orderPtr, 
                    255
                );

                this.playbackPtr = this.wasm.exports.malloc(
                    3 * Uint16Array.BYTES_PER_ELEMENT
                )
                this.playbackArray = new Uint16Array(
                    this.wasm.exports.memory.buffer, 
                    this.playbackPtr, 
                    3
                );
                
                // init patterns with 64 rows
                for(let p = 0; p < msg.numPatterns; p++) {
                    this.rowArray[p] = msg.numRows;
                }
                console.log(this.rowArray);

                let offset = 0;

                for(let p = 0; p < msg.numPatterns; p++) {
                    this.offsetArray[p] = offset;
                    offset += this.rowArray[p] * msg.numChannels;
                }

                this.wasm.exports.init_engine(
                    this.enginePtr,
                    msg.sampleRate, 
                    msg.bpm, 
                    msg.ticksPerRow,

                    this.patternPtr,
                    this.rowPtr,
                    this.offsetPtr,
                    this.orderPtr,
                    this.playbackPtr,

                    msg.numPatterns,
                    msg.numChannels,
                    totalCells,
                    1 // start with one order
                );

                this.sampleRate = msg.sampleRate;
                this.bpm = msg.bpm;
                this.ticksPerRow = msg.ticksPerRow;

                this.playing = false;
                this.rowNo = 0;
                this.patternNo = 0;
                this.b = 0;

                this.samplePool = [];

                this.port.postMessage({
                    type: "shared",
                    mem: this.wasm.exports.memory.buffer,
                    patternPtr: this.patternPtr,
                    patternLength: this.patternArray.length,
                    rowPtr: this.rowPtr,
                    rowLength: this.rowArray.length,
                    offsetPtr: this.offsetPtr,
                    offsetLength: this.offsetArray.length,
                    orderPtr: this.orderPtr,
                    orderLength: this.orderArray.length,
                    playbackPtr: this.playbackPtr,
                    playbackLength: this.playbackArray.length,
                })
            })
            .catch((e) => {
                console.log("Error: Failed to instantiate WebAssembly module.\n" + e);
            })
    }

    stringToCharPointer(encoded) {
        // need null terminator too
        const stringPtr = this.wasm.exports.malloc((encoded.length + 1) * Int8Array.BYTES_PER_ELEMENT);

        const stringHeap = new Int8Array(
            this.wasm.exports.memory.buffer,
            stringPtr,
            encoded.length + 1
        );

        stringHeap.set(encoded); // these need to be int8_t
        stringHeap[encoded.length] = 0; // null terminator
        return stringPtr;
    }

    addSample(msg) {
        console.log(msg);
        // pointers and heaps to memory
        const lAudioPtr = this.wasm.exports.malloc(msg.data[0].length * Float32Array.BYTES_PER_ELEMENT);
        const rAudioPtr = this.wasm.exports.malloc(msg.data[0].length * Float32Array.BYTES_PER_ELEMENT);

        const lAudioHeap = new Float32Array(
            this.wasm.exports.memory.buffer,
            lAudioPtr,
            msg.data[0].length
        );

        const rAudioHeap = new Float32Array(
            this.wasm.exports.memory.buffer,
            rAudioPtr,
            msg.data[1].length
        );

        lAudioHeap.set(msg.data[0]);
        rAudioHeap.set(msg.data[1]);

        const stringPtr = this.stringToCharPointer(msg.encoded);

        // int add_sample(Engine* engine, const char* filename, float* left, float* right, int length, int sample_rate)
        const res = this.wasm.exports.register_sample(this.enginePtr, stringPtr, lAudioPtr, rAudioPtr, msg.duration, msg.sampleRate);

        console.log(res);

        this.samplePool.push(
            {
                leftPtr: lAudioPtr,
                rightPtr: rAudioPtr,
                sampleRate: msg.sampleRate,
                channels: msg.channels,
                length: msg.duration
            }
        )

        this.port.postMessage({
            type: "sample",
            filename: msg.filename,
            leftPtr: lAudioPtr,
            rightPtr: rAudioPtr,
            sampleRate: msg.sampleRate,
            channels: msg.channels,
            length: msg.duration,
        })

        this.wasm.exports.free(stringPtr);
    }

    playMidi(msg) {
        if(msg.instrumentId <= 0) return;
        // int play_from_midi(Engine* engine, int instrument_id, int note_id)
        const res = this.wasm.exports.play_from_midi(this.enginePtr, msg.instrumentId, msg.note);
    }

    stopMidi(msg) {
        const res = this.wasm.exports.stop_from_midi(this.enginePtr, msg.instrumentId, msg.note);
    }

    setBPM(msg) {
        this.wasm.exports.set_bpm(msg.value);
    }

    setRPB(msg) {
        this.wasm.exports.set_rpb(msg.value);
    }

    setPlay(msg) {
        // void play_track(Engine* engine, int order, int row)
        this.wasm.exports.play_track(this.enginePtr, 0, 0);
    }

    setPause(msg) {

    }

    process(ins, outs, parameters) {
        if(!this.wasm) return true;
        
        const n = outs[0][0].length;

        // int process(Engine* engine, float* output, int length)
        const beat = this.wasm.exports.process(this.enginePtr, this.outputTablePtr, n);
        const voices = this.wasm.exports.get_num_voices(this.enginePtr);
        const curr_row = this.wasm.exports.get_current_row(this.enginePtr);
        
        if(voices >= 1) {
            console.log(voices + " voice(s)");
        }

        // turn off
        if(beat == -1) this.playing = false;

        // we detect a beat and so update row count
        if(beat == 1) {
            if(this.b % 4 == 0)
                console.log("beat")
            this.b++;
            this.rowNo++;
        }

        if (this.rowNo >= 64) {
            this.patternNo++;
            this.rowNo = 0;
        }
        
        outs[0][0].set(this.leftHeap);
        outs[0][1].set(this.rightHeap);

        return true;
    }
}

registerProcessor("mixer-processor", MixerProcessor);