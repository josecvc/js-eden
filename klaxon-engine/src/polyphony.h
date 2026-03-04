#ifndef POLYPHONY_H
#define POLYPHONY_H

// TODO: Add Sampler polyphony and Synth polyphony
struct SynthVoice 
{
    int note_id;
    int channel_id;
    float frequency{440.f};
    float phase{.0f};
    bool active{false};
    bool releasing{false};
    float velocity{1.0f};
};

struct SampleVoice 
{
    int note_id;
    int channel_id;
    float rate{1.0f};
    bool active{false};
    float gain{1.0f};

    double position{0};
};

struct Unison
{
    int instances;
};

class Polyphony 
{
public:
    static constexpr int MAX_VOICES = 16;

    virtual ~Polyphony() = default;
    virtual void init() = 0;
    virtual int get_current_voices() const = 0;
    virtual void add_voice(int channel_id, int note_id, float volume, int param) = 0;
    virtual void remove_voice(int channel_id, int note_id) = 0;
};


class SamplePolyphony : public Polyphony
{
public:
    SamplePolyphony() {}

    void init() override;
    int get_current_voices() const override;
    void add_voice(int channel_id, int note_id, float gain, int root_note) override;
    void remove_voice(int channel_id, int note_id) override;

    SampleVoice voices[MAX_VOICES];
    int curr{0};
};

class SynthPolyphony : public Polyphony
{
public:
    SynthPolyphony() {}

    void init() override;
    int get_current_voices() const override;
    void add_voice(int channel_id, int note_id, float vel, int param) override;
    void remove_voice(int channel_id, int note_id) override;
    void set_unison_count(int instances);

    Unison uni;
    SynthVoice voices[MAX_VOICES];
    int curr{0};
};

#endif