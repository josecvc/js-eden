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
            msg.data[1].length
        );

        const rAudioHeap = new Float32Array(
            this.wasm.exports.memory.buffer,
            rAudioPtr,
            msg.data[1].length
        );

        lAudioHeap.set(msg.data[0]);
        rAudioHeap.set(msg.data[1]);

        const res = this.wasm.exports.add_sample(this.enginePtr, msg.filename, lAudioPtr, rAudioPtr, msg.duration, msg.sampleRate);
        console.log(res);

        this.wasm.exports.free(lAudioPtr);
        this.wasm.exports.free(rAudioPtr);
    }

    playMidi(msg) {
        const res = this.wasm.exports.play_from_midi(this.enginePtr, msg.instrumentId, msg.note);
        console.log(res);
    }

    stopMidi(msg) {
        console.log(msg);
    }

    process(ins, outs, parameters) {
        if(!this.wasm) return true;
        
        const n = outs[0][0].length;

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