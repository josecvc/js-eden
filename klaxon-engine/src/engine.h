#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <memory>
#include <atomic>

#include "instrument.h"
#include "pattern.h"
#include "polyphony.h"

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
        uint8_t* playback,
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
    void add_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
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

    std::vector<std::unique_ptr<Instrument>> instruments;
    Polyphony poly;
    // Channel channels; maybe?? 
    
};

#endif