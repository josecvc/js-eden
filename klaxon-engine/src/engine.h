#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <memory>

#include "instrument.h"

class Engine
{
public:
    static constexpr int MAX_INSTRUMENTS = 32;

    Engine() {}
    Engine(int sample_rate, int bpm, int rows_per_beat) : sample_rate(sample_rate), bpm(bpm), rows_per_beat(rows_per_beat) {}

    void init(int sample_rate, int bpm, int rows_per_beat);

    int process(float** output, int frames);
    void mix_instruments(float** output, int frames);
    void clear(float** output, int frames);
    int step(int frames);

    void play(int total_rows);
    void pause();
    void stop();

    void set_bpm(int bpm);
    void set_sample_rate(int sample_rate);
    void set_rows_per_beat(int rows_per_beat);

    void add_sample(const char* filename, const void* data, unsigned long length);
    void add_synth();
    void remove_instrument(int instrument_id);

    double current_samples;
    int bpm;
    int sample_rate;
    double samples_per_row;
    int rows_per_beat;
    int instrument_count;
    bool is_playing;

    std::vector<std::unique_ptr<Instrument>> instruments;
    
};

#endif