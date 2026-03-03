const importObject = {
    env: {
        memory: new WebAssembly.Memory({ initial: 256 }),
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
            } else if (msg.type == "patten-order") {
                this.setOrder(msg);
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

                // TODO: Add pattern data
                
                this.HEAPU8 = new Uint8Array(this.wasm.exports.memory.buffer); 
                this.HEAPU32 = new Uint32Array(this.wasm.exports.memory.buffer); 
                this.HEAPF32 = new Float32Array(this.wasm.exports.memory.buffer);

                const frames = 128;
                const channels = 2; // stereo audio
                const max_incoming = 16;
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
                
                // void init_engine(Engine* engine, int sample_rate, int bpm, int rows_per_beat)
                this.wasm.exports.init_engine(this.enginePtr, msg.sampleRate, msg.bpm, msg.rowsPerBeat);

                this.sampleRate = msg.sampleRate;
                this.bpm = msg.bpm;
                this.rowsPerBeat = msg.rowsPerBeat;

                this.playing = false;
                this.rowNo = 0;
                this.patternNo = 0;

                this.test = false;
            })
            .catch((e) => {
                console.log("Error: Failed to instantiate WebAssembly module.\n" + e);
            })
    }

    addSample(msg) {
        const bytes = msg.length * 4;
        
        console.log(msg);
        // pointers and heaps to memory
        const lAudioPtr = this.wasm.exports.malloc(msg.data[0].length * 4);
        const rAudioPtr = this.wasm.exports.malloc(msg.data[0].length * 4);

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

        // int add_sample(Engine* engine, const char* filename, float* left, float* right, int length, int sample_rate)
        const res = this.wasm.exports.add_sample(this.enginePtr, msg.filename, lAudioPtr, rAudioPtr, msg.duration, msg.sampleRate);
        console.log(res);

        this.wasm.exports.free(lAudioPtr);
        this.wasm.exports.free(rAudioPtr);
    }

    playMidi(msg) {

        // int play_from_midi(Engine* engine, int instrument_id, int note_id)
        const res = this.wasm.exports.play_from_midi(this.enginePtr, msg.instrumentId, msg.note);
    }

    stopMidi(msg) {
        console.log(msg);
    }

    setOrder(msg) {        
        // pointers and heaps to memory
        const sequencePtr = this.wasm.exports.malloc(msg.sequence.length * 4);
        const patternPtr = this.wasm.exports.malloc(msg.patterns.length * 4);

        const sequenceHeap = new Int32Array(
            this.wasm.exports.memory.buffer,
            sequencePtr,
            msg.sequence.length
        );

        const patternHeap = new Int32Array(
            this.wasm.exports.memory.buffer,
            patternPtr,
            msg.patterns.length
        );

        sequenceHeap.set(msg.sequence);
        patternHeap.set(msg.patterns);

        // int read_order(Engine* engine, int* sequence, int sequence_length, int* patterns, int pattern_length)
        const res = this.wasm.exports.read_order(this.enginePtr, sequencePtr, msg.sequence.length, patternPtr, msg.patterns.length);

        this.wasm.exports.free(sequencePtr);
        this.wasm.exports.free(patternPtr);
    }

    setBPM(msg) {

    }

    setRPB(msg) {

    }

    setPlay(msg) {

    }

    setPause(msg) {

    }

    process(ins, outs, parameters) {
        if(!this.wasm) return true;
        
        const n = outs[0][0].length;

        // int process(Engine* engine, float* output, int length)
        const beat = this.wasm.exports.process(this.enginePtr, this.outputTablePtr, n);

        // turn off
        if(beat == -1) this.playing = false;

        // we detect a beat and so update row count
        if(beat == 1) {
            this.rowNo++;
        }

        if (this.rowNo >= 64) {
            this.patternNo++;
            this.rowNo = 0;
        }
        
        outs[0][0].set(this.leftHeap);
        outs[0][1].set(this.rightHeap);

        if( outs[0][0][0] != 0 && outs[0][1][0] != 0 && !this.test) {
            console.log({left: outs[0][0], right: outs[0][1]});
            this.test = true;
        }

            

        return true;
    }
}

registerProcessor("mixer-processor", MixerProcessor);