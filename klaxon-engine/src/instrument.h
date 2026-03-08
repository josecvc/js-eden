#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "sample.h"
#include "wavetable.h"
#include "polyphony.h"

// TODO: Finish the instruments, adding them, modifying parameters etc. Remove virtual and either use templates or std::variant
class Instrument
{
public:
    Instrument() {}
    virtual ~Instrument() = default;
    virtual void render(float** output, int frames, Voice& voice) = 0;
};

class SampleInstrument : public Instrument
{
public:
    SampleInstrument(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void init(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void render(float** output, int frames, Voice& voice) override;

    Sample sample;
};

class SynthInstrument : public Instrument
{
public:
    void init();
    void render(float** output, int frames, Voice& voice) override;

    WaveTable wave_table;   
};

#endif