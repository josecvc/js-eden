const importObject = {
    env: {
        memory: new WebAssembly.Memory({
                    initial: 256,
                    maximum: 256,
                    shared: false
                }),
        table: new WebAssembly.Table({ initial: 0, element: "anyfunc" }),
        abort: () => { throw new Error("WASM abort"); },
    }
};

const ROW = 0;
const PATTERN = 1;
const ORDER = 2;
const IS_PLAYING = 3;

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
            } else if (msg.type == "speed") {
                this.setSpeed(msg);
            } else if (msg.type == "playstart") {
                this.setPlay(msg);
            } else if (msg.type == "playpause") {
                this.setPause(msg);
            } else if (msg.type == "note_set") {
                this.emplaceNote(msg);
            } else if (msg.type == "instr_set") {
                this.emplaceInstr(msg);
            } else if (msg.type == "vol_set") {
                this.emplaceVol(msg);
            } else if (msg.type == "effect_set") {
                this.emplaceEffect(msg);
            } else if (msg.type == "param_set") {
                this.emplaceParam(msg);
            } else if (msg.type == "insert_order") {
                this.insertOrder(msg);
            } else if (msg.type == "delete_order") {
                this.deleteOrder(msg);
            }
        };
    }

    initProcessor(msg) {
        WebAssembly.compile(msg.wasm)
        .then((module) => {
            WebAssembly.instantiate(msg.wasm, importObject)
            .then((obj) => {
                this.wasm = obj.instance
                
                this.HEAPU8 = new Uint8Array(this.wasm.exports.memory.buffer); 
                this.HEAPU32 = new Uint32Array(this.wasm.exports.memory.buffer); 
                this.HEAPF32 = new Float32Array(this.wasm.exports.memory.buffer);

                const frames = 128;
                const channels = 2; // stereo audio
                const bytes = frames * Float32Array.BYTES_PER_ELEMENT;

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

                /*
                void init_engine(
                    Engine* engine, 
                    int sample_rate, 
                    int bpm, 
                    int ticks_per_row,
                )
                */
                this.wasm.exports.init_engine(
                    this.enginePtr,
                    msg.sampleRate, 
                    msg.bpm, 
                    msg.ticksPerRow,
                );

                this.sampleRate = msg.sampleRate;
                this.bpm = msg.bpm;
                this.ticksPerRow = msg.ticksPerRow;

                this.playing = false;
                this.rowNo = 0;
                this.patternNo = 0;
                this.b = 0;
                
                this.playbackBuffer = new SharedArrayBuffer(4 * Int32Array.BYTES_PER_ELEMENT);
                this.playbackArray = new Int32Array(
                    this.playbackBuffer   
                );

                this.port.postMessage({
                    type: "sab",
                    sharedBuffer: this.playbackBuffer
                });

                this.samplePool = [];
            })
        }).catch((e) => {
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

        this.port.postMessage({
            type: "sample",
            id: msg.id,
            filename: msg.filename,
            leftPtr: lAudioPtr,
            rightPtr: rAudioPtr,
            sampleRate: msg.sampleRate,
            channels: msg.channels,
            length: msg.duration
        })

        this.wasm.exports.free(stringPtr);
    }

    emplaceNote(msg) {
        this.wasm.exports.set_note(this.enginePtr, msg.id, msg.calcIndex);
    }

    emplaceInstr(msg) {
        this.wasm.exports.set_instrument(this.enginePtr, msg.id, msg.calcIndex);
    }

    emplaceVol(msg) {
        this.wasm.exports.set_volume(this.enginePtr, msg.id, msg.calcIndex);
    }
    
    emplaceEffect(msg) {
        this.wasm.exports.set_effect(this.enginePtr, msg.id, msg.calcIndex);
    }
    
    emplaceParam(msg) {
        this.wasm.exports.set_param(this.enginePtr, msg.id, msg.calcIndex);
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
        this.wasm.exports.set_tempo(this.enginePtr, msg.value);
    }

    setSpeed(msg) {
        this.wasm.exports.set_ticks_per_row(this.enginePtr, msg.value);
    }

    setPlay(msg) {
        // void play_track(Engine* engine, int order, int row)
        this.wasm.exports.play_track(this.enginePtr, 0, 0);
    }

    setPause(msg) {
        this.wasm.exports.stop_track(this.enginePtr);
    }

    insertOrder(msg) {
        this.wasm.exports.insert_order(this.enginePtr, msg.pos, msg.patternId);
    }

    deleteOrder(msg) {
        this.wasm.exports.delete_order(this.enginePtr, msg.pos);
    }

    process(ins, outs, parameters) {
        if(!this.wasm) return true;
        
        const n = outs[0][0].length;

        // int process(Engine* engine, float** output, int length), tick is used for debugging timing issues
        const tick = this.wasm.exports.process(this.enginePtr, this.outputTablePtr, n);

        const voices = this.wasm.exports.get_num_voices(this.enginePtr);

        const currRow = this.wasm.exports.get_current_row(this.enginePtr);
        const currPattern = this.wasm.exports.get_current_pattern(this.enginePtr);
        const currOrder = this.wasm.exports.get_current_order(this.enginePtr);
        const playbackState = this.wasm.exports.get_playback_state(this.enginePtr);
        
        if(voices >= 1) {
            console.log(voices + " voice(s)");
        }

        outs[0][0].set(this.leftHeap);
        outs[0][1].set(this.rightHeap);

        Atomics.store(this.playbackArray, ROW, currRow);
        Atomics.store(this.playbackArray, PATTERN, currPattern);
        Atomics.store(this.playbackArray, ORDER, currOrder);
        Atomics.store(this.playbackArray, IS_PLAYING, playbackState);

        return true;
    }
}

registerProcessor("mixer-processor", MixerProcessor);