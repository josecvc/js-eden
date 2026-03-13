#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <memory>
#include <variant>


#include "instrument.h"
#include "pattern.h"
#include "polyphony.h"

using Instrument = std::variant<std::monostate, SynthInstrument, SampleInstrument>;

class Engine
{
public:
    static constexpr int MAX_INSTRUMENTS = 128;
    static constexpr int ROW = 0;
    static constexpr int PATTERN = 1;
    static constexpr int ORDER = 2;

    Engine() {}
    Engine(int sample_rate, int bpm, int ticks_per_row) : sample_rate(sample_rate), bpm(bpm), ticks_per_row(ticks_per_row) {}

    void init(
        int sample_rate, 
        int bpm, 
        int ticks_per_row
    );

    // playback
    int process(float** output, int frames);
    void mix_instruments(float** output, int frames);
    void clear(float** output, int frames);
    int step(int frames);
    void advance_tick();
    void process_row();
    void advance_row();
    void process_effects(int row_tick);

    // playback mutation
    void play(int order, int row);
    void pause();
    void stop();
    
    // timing
    void set_bpm(int bpm);
    void set_sample_rate(int sample_rate);
    void set_ticks_per_row(int ticks_per_row);
    
    // instruments
    int register_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
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

    uint16_t* playback;

    int instrument_count{0}; // use this to switch to Instrument[MAX_INSTRUMENTS]
    
    // Pattern information
    PatternInfo pattern_info;

    // Channel data (last command used, channel audio information)
    ChannelData channels[128];

    Instrument instruments[MAX_INSTRUMENTS];
    std::vector<std::shared_ptr<Sample>> sample_pool;

    Polyphony poly;
    // Channel channels; maybe?? 
    
};

#endif