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
            }
        };
        this.patterns = {};
    }

    initProcessor(message) {
        WebAssembly.instantiate(message.wasm, importObject)
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

                this.mixer_ptr = this.wasm.exports.create_engine(message.sample_rate, message.bpm, message.rows_per_beat);

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

                this.wasm.exports.init_engine(this.mixer_ptr, message.sampleRate, message.bpm, message.rowsPerBeat);

                this.sampleRate = message.sampleRate;
                this.bpm = message.bpm;
                this.rowsPerBeat = message.rowsPerBeat;

                this.playing = false;
                this.rowNo = 0;
                this.patternNo = 0;
            })
            .catch((e) => {
                console.log("Error: Failed to instantiate WebAssembly module.\n" + e);
            })
    }

    process(ins, outs, parameters) {
        if(!this.wasm) return true;
        
        const n = outs[0][0].length;

        const beat = this.wasm.exports.process(this.mixer_ptr, this.outputTablePtr, n);

        // turn off
        if(beat == -1) this.playing = false;

        // we detect a beat and so update row count
        if(beat == 1) {
            this.port.postMessage({
                type: "elapsed",
                pattern: this.patternNo,
                row: this.rowNo
            });
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