#ifndef POLYPHONY_H 
#define POLYPHONY_H

#include "instrument.h"
#include "sample.h"

struct Voice
{
    int note_id;
    int channel_id;
    int instrument_id;
    int sample_id;

    int env_tick{0};
    int env_pos{0};
    float env_val{0.f};
    float env_fade{0.f};

    float frequency{440.f};
    float phase{.0f};
    float rate{1.0f};
    
    float volume{0};
    double position{0.0f};

    bool active{false};
    bool releasing{false};
    bool finished{false};
    bool backwards{false};

    InstrumentType type;
    Instrument* instrument;
    Sample* sample;
};

struct Unison
{
    int instances;
};

class Polyphony 
{
public:
    static constexpr int MAX_VOICES = 64; // must be the at least the number of channels

    Polyphony() {}
    void init();
    
    void add_voice(Instrument* samp_inst, Sample* sample, int channel_id, int note_id, int instrument_id, int sample_id, float volume, int root_note);
    void remove_voice(int channel_id, int note_id, int instrument_id);

    void render_voices(float** output, int frames);
    void render_synth(Voice& voice, float** output, int frames);
    void render_sample(Voice& voice, float** output, int frames);
    void advance_env_tick();

    void dump_playheads(int* sample_playhead_arr, int* envelope_playhead_arr, int sample_id, int instrument_id);

    int get_current_voices() const;

    Voice voices[MAX_VOICES];
    int curr{0};
    int sample_rate;
};

#endif