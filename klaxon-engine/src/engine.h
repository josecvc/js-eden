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
    Engine(int sample_rate, int bpm, int rows_per_beat) : sample_rate(sample_rate), bpm(bpm), rows_per_beat(rows_per_beat) {}

    void init(int sample_rate, int bpm, int rows_per_beat);

    // playback
    int process(float** output, int frames);
    void mix_instruments(float** output, int frames);
    void clear(float** output, int frames);
    int step(int frames);
    void process_row();
    void advance_row();

    // playback mutation
    void insert_order(int* patterns, int num_patterns, int* sequence, int num_indices);
    void play(int total_rows);
    void pause();
    void stop();
    
    // timing
    void set_bpm(int bpm);
    void set_sample_rate(int sample_rate);
    void set_rows_per_beat(int rows_per_beat);
    
    // instruments
    void add_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void add_synth();
    void remove_instrument(int instrument_id);

    // timing attributes
    double current_samples;
    int bpm;
    int sample_rate;
    double samples_per_row;
    int rows_per_beat;

    // playback state
    bool is_playing{false};
    unsigned short current_row;
    unsigned short current_order;
    unsigned short current_pattern;

    int instrument_count; // use this if you intend on switching to Instrument[MAX_INSTRUMENTS]
    
    Order order;
    std::vector<std::unique_ptr<Instrument>> instruments;
    Polyphony poly;
    
};

#endif