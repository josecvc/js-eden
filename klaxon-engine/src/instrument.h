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
    virtual void init() = 0;
};

class SampleInstrument : public Instrument
{
public:
    SampleInstrument();
    void init() override;    

    Sample* sample;
    int position = 0;
    Polyphony poly;

    void render(float** output, int frames) override;
};

class SynthInstrument : public Instrument
{
public:
    void init() override;

    WaveTable wave_table;
    Polyphony poly;
    void render(float** output, int frames) override;
};

#endif