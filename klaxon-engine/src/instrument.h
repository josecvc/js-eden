#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "sample.h"
#include "wavetable.h"
#include "polyphony.h"

// TODO: Finish the instruments, adding them, modifying parameters etc.
class Instrument
{
public:
    Instrument() {}
    virtual ~Instrument() = default;
    virtual void render(float** output, int frames) = 0;
    virtual void add_voice(int channel_id, int note_id) = 0;
    virtual void remove_voice(int channel_id, int note_id) = 0;
};

class SampleInstrument : public Instrument
{
public:
    SampleInstrument(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void init(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void add_voice(int channel_id, int note_id) override;
    void remove_voice(int channel_id, int note_id) override;
    void render(float** output, int frames) override;

    Sample sample;
    SamplePolyphony poly;
};

class SynthInstrument : public Instrument
{
public:
    void init();
    void add_voice(int channel_id, int note_id) override;
    void remove_voice(int channel_id, int note_id) override;
    void render(float** output, int frames) override;

    WaveTable wave_table;
    SynthPolyphony poly;    
};

#endif