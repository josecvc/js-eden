#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <memory>

#include "instrument.h"
#include "pattern.h"
#include "polyphony.h"

class Engine
{
public:
    static constexpr int MAX_INSTRUMENTS = 32;

    Engine() {}
    Engine(int sample_rate, int bpm, int ticks_per_row) : sample_rate(sample_rate), bpm(bpm), ticks_per_row(ticks_per_row) {}

    void init(int sample_rate, int bpm, int ticks_per_row);

    // playback
    int process(float** output, int frames);
    void mix_instruments(float** output, int frames);
    void clear(float** output, int frames);
    int step(int frames);
    void process_row();
    void advance_row();

    // playback mutation
    void insert_order(int* patterns, int num_patterns, int* sequence, int num_indices);
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
    double current_samples;
    int current_ticks;
    int bpm;
    int sample_rate;
    double samples_per_tick;
    int ticks_per_row;

    // playback state
    bool is_playing{false};
    short current_row;
    short current_order;
    short current_pattern;

    int instrument_count; // use this if you intend on switching to Instrument[MAX_INSTRUMENTS]
    
    Order order;
    std::vector<std::unique_ptr<Instrument>> instruments;
    Polyphony poly;
    
};

#endif