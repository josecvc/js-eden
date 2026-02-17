const importObject = {
    env: {
        memory: new WebAssembly.Memory({ initial: 256 }),
        table: new WebAssembly.Table({ initial: 0, element: "anyfunc" }),
        abort: () => { throw new Error("WASM abort"); }
    }
};

class TestProcessor extends AudioWorkletProcessor {
    constructor() {
        super();
        this.port.onmessage = (e) => this.initProcessor(e.data)
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

                this.wasm.exports.init_wavetable(message.sampleRate);

            })
            .catch((e) => {
                console.log("Error: Failed to instantiate WebAssembly module.\n" + e);
            })
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

registerProcessor("test-processor", TestProcessor);