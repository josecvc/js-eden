#ifndef POLYPHONY_H
#define POLYPHONY_H

// TODO: Add Sampler polyphony and Synth polyphony
struct Voice 
{
    int note_id;
    int channel_id;
    float frequency{440.f};
    float phase{.0f};
    bool active{false};
    bool releasing{false};
    float velocity{1.0f};
};

struct Unison
{
    int instances;
};

class Polyphony 
{
public:
    static constexpr int MAX_VOICES = 64;

    Polyphony() {}

    void init();
    int get_current_voices();
    void add_voice(int note_id);
    void remove_voice(int note_id);
    void set_unison_count(int instances);
    
    Unison uni;
    Voice voices[MAX_VOICES];
    int curr{0};
};

#endif