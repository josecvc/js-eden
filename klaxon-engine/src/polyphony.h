#ifndef POLYPHONY_H
#define POLYPHONY_H

struct Voice 
{
    int note_id;
    float frequency{440.f};
    float phase{.0f};
    bool active{false};
    bool releasing{false};
    float velocity{1.0f};
};

class Polyphony 
{
public:
    static constexpr int MAX_VOICES = 16;

    Polyphony() {}

    void init();
    int get_current_voices();
    void add_voice(int note_id);
    void remove_voice(int note_id);

    Voice voices[MAX_VOICES];
    int curr{0};
};

#endif