#ifndef POLYPHONY_H
#define POLYPHONY_H

#include "instrument.h"
#include "sample.h"

struct Voice
{
    int note_id;
    int channel_id;
    int instrument_id;

    int env_tick;
    int env_pos;
    float env_val;

    float frequency{440.f};
    float phase{.0f};
    float rate{1.0f};
    
    float volume{0};
    double position{0.0f};

    bool active{false};
    bool releasing{false};
    bool finished{false};

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
    int get_current_voices() const;
    void add_voice(Instrument* samp_inst, Sample* sample, int channel_id, int note_id, int instrument_id, float volume, int root_note);
    void remove_voice(int channel_id, int note_id, int instrument_id);
    void render_voices(float** output, int frames);
    void render_synth(Voice& voice, float** output, int frames);
    void render_sample(Voice& voice, float** output, int frames);

    void advance_env_tick();

    Voice voices[MAX_VOICES];
    int curr{0};
    int sample_rate;
};

#endif