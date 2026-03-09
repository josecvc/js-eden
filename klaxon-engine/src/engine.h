#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <memory>
#include <atomic>
#include <variant>

#include "instrument.h"
#include "pattern.h"
#include "polyphony.h"

using Instrument = std::variant<SynthInstrument, SampleInstrument>;

class Engine
{
public:
    static constexpr int MAX_INSTRUMENTS = 32;
    static constexpr int ROW = 0;
    static constexpr int PATTERN = 1;
    static constexpr int ORDER = 2;

    Engine() {}
    Engine(int sample_rate, int bpm, int ticks_per_row) : sample_rate(sample_rate), bpm(bpm), ticks_per_row(ticks_per_row) {}

    void init(
        int sample_rate, 
        int bpm, 
        int ticks_per_row,
        uint8_t* patterns,
        uint16_t* pattern_rows,
        uint32_t* pattern_offset,
        uint8_t* pattern_order,
        uint16_t* playback,
        int num_patterns,
        int num_channels,
        int num_cells,
        int num_orders
    );

    // playback
    int process(float** output, int frames);
    void mix_instruments(float** output, int frames);
    void clear(float** output, int frames);
    int step(int frames);
    void process_row();
    void advance_row();

    // playback mutation
    void play(int order, int row);
    void pause();
    void stop();
    
    // timing
    void set_bpm(int bpm);
    void set_sample_rate(int sample_rate);
    void set_ticks_per_row(int ticks_per_row);
    
    // instruments
    void register_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void add_synth();
    void remove_instrument(int instrument_id);

    // timing attributes
    int bpm;
    int sample_rate;
    double samples_per_tick;
    int ticks_per_row;

    // playback state
    bool is_playing{false};
    double current_samples;
    int current_ticks;
    short current_row;
    short current_order;
    short current_pattern;

    std::atomic<uint16_t>* playback;

    int instrument_count; // use this to switch to Instrument[MAX_INSTRUMENTS]
    
    // Pattern information
    PatternInfo pattern_info;

    std::vector<Instrument> instruments;
    std::vector<std::shared_ptr<Sample>> sample_pool;

    Polyphony poly;
    // Channel channels; maybe?? 
    
};

/*

JS:
const sampleLeftPtr = this.wasm.exports.malloc(msg.length * Float32.BYTES_PER_ELEMENT);
const sampleRightPtr = this.wasm.exports.malloc(msg.length * Float32.BYTES_PER_ELEMENT);
const sampleLeftArray = new Float32Array(
    this.wasm.exports.memory.buffer,
    sampleLeftPtr,
    msg.length
);

const sampleRightArray = new Float32Array(
    this.wasm.exports.memory.buffer,
    sampleRightPtr,
    msg.length
);

this.wasm.exports.register_sample(this.enginePtr, sampleLeftPtr, sampleRightPtr, msg.filename, msg.sample_rate, msg.length, msg.numChannels);

this.samples.push_back([sampleLeftPtr, sampleRightPtr, sampleLeftArray, sampleRightArray, msg.filename, msg.length, msg.numChannels]); // unlikely to be like this

C++:

void register_sample(Engine* engine, float* left, right, const char* filename, int sample_rate, uint64_t length, int channels)
{
    if (!engine) return -2;
    if(!left || !right) return -1;

    std::shared_ptr<Sample> smp = std::make_shared<Sample>();
    smp->load_sample(filename, left, right, sample_rate, length))

    engine->sample_pool.push_back(smp)
}

*/
#endif