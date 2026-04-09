#ifndef ENGINE_H
#define ENGINE_H

#include <vector>
#include <array>
#include <memory>

#include "instrument.h"
#include "pattern.h"
#include "polyphony.h"
#include "editing.h"

class Engine
{
public:
    static constexpr int MAX_INSTRUMENTS = 128;
    static constexpr int MAX_SAMPLES = 256;
    static constexpr int ROW = 0;
    static constexpr int PATTERN = 1;
    static constexpr int ORDER = 2;

    Engine() : sample_rate(44100), bpm(140), ticks_per_row(6) {}
    Engine(int sample_rate, int bpm, int ticks_per_row) : sample_rate(sample_rate), bpm(bpm), ticks_per_row(ticks_per_row) {}

    void init(
        int sample_rate, 
        int bpm, 
        int ticks_per_row
    );

    // playback
    int process(float** output, int frames);
    void mix_instruments(float** output, int frames);
    void hard_clip(float** output, int frames);
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
    int register_sample(const char *filename, float *left, float *right, int sample_rate, unsigned long length, int sample_id);
    int register_synth();

    void remove_instrument(int instrument_id);
    int clear_sample(int sample_id);
    int switch_instrument_type(int instrument_id, InstrumentType type);

    Sample* get_sample(Instrument& instrument, int note_id);

    // timing attributes
    int bpm;
    int sample_rate;
    double samples_per_tick;
    int ticks_per_row;

    // playback state
    bool is_playing{false};

    double current_samples{0};
    double current_env_samples{0};
    int current_ticks{0};
    short current_row{0};
    short current_order{0};
    short current_pattern{0};

    int current_instrument{0};
    int current_sample{0};

    int sample_playheads[Polyphony::MAX_VOICES]{-1};
    int envelope_playheads[Polyphony::MAX_VOICES]{-1};

    int instrument_count{0};
    
    PatternInfo pattern_info;

    // History and cut/copy
    SampleEditor editor;
    Clipboard clipboard;

    // Channel data (last command used, channel audio information)
    ChannelData channels[MAX_CHANNELS];

    Instrument instruments[MAX_INSTRUMENTS];

    std::array<std::unique_ptr<Sample>, MAX_SAMPLES> sample_pool;

    Polyphony poly;
};

#endif