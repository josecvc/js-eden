const importObject = {
    env: {
        memory: new WebAssembly.Memory({ initial: 256 }),
        table: new WebAssembly.Table({ initial: 0, element: "anyfunc" }),
        abort: () => { throw new Error("WASM abort"); }
    }
};

class SynthProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.port.onmessage = (e) => {
            const msg = e.data;
            console.log(msg)
            if (msg.type === "wasm") {
                this.initProcessor(msg)
            } else if (msg.type === "note_on") {
                this.noteOn(msg.note);
            } else if (msg.type === "note_off") {
                this.noteOff(msg.note);
            } else if (msg.type === "unison") {
                this.changeUnison(msg.instances);
            }
            
        };
    }

    initProcessor(message) {
        WebAssembly.instantiate(message.wasm, {})
            .then((obj) => {
                this.wasm = obj.instance
                
                
                const frames = 128;

                const bytes = frames * 4;

                this.inputPtr = this.wasm.exports.malloc(bytes);
                this.outputPtr = this.wasm.exports.malloc(bytes);

                this.inputHeap = new Float32Array(
                    this.wasm.exports.memory.buffer,
                    this.inputPtr,
                    frames
                );

                this.outputHeap = new Float32Array(
                    this.wasm.exports.memory.buffer,
                    this.outputPtr,
                    frames
                );

                this.wasm.exports.init_synth(message.sampleRate);

            })
            .catch((e) => {
                console.log("Error: Failed to instantiate WebAssembly module.\n" + e);
            })
    }

    noteOn(note) {
        // Send this to a WASM export
        console.log(note);
        this.wasm.exports.add_note(note);
    }

    noteOff(note) {
        this.wasm.exports.remove_note(note);
    }

    changeUnison(instances) {
        this.wasm.exports.set_unison_count(instances);
    }

    process(ins, outs, parameters) {
        if(!this.wasm) return true;

        const output = outs[0][0];

        const n = output.length

        this.wasm.exports.process(this.inputPtr, this.outputPtr, n);

        output.set(this.outputHeap);

        return true;
    }
}

registerProcessor("synth-processor", SynthProcessor);