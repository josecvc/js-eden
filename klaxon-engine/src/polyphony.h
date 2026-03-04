#ifndef POLYPHONY_H
#define POLYPHONY_H

// TODO: Add Sampler polyphony and Synth polyphony

struct Voice
{
    int note_id;
    int channel_id;
    int instrument_id;
    float frequency{440.f};
    float phase{.0f};
    float rate{1.0f};
    bool active{false};
    bool releasing{false};
    bool finished{false};
    float volume{0};
    double position{0.0f};
};

struct Unison
{
    int instances;
};

class Polyphony 
{
public:
    static constexpr int MAX_VOICES = 32; // must be the at least the number of channels

    Polyphony() {}
    void init();
    int get_current_voices() const;
    void add_voice(int channel_id, int note_id, int instrument_id, float volume, int param);
    void remove_voice(int channel_id, int note_id, int instrument_id);

    Voice voices[MAX_VOICES];
    int curr{0};
};

#endif