#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <variant>
#include "sample.h"
#include "wavetable.h"
#include "polyphony.h"

// TODO: Finish the instruments, adding them, modifying parameters etc. Remove virtual and either use templates or std::variant

class SampleInstrument
{
public:
    SampleInstrument() {}
    SampleInstrument(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void init(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void render(float** output, int frames, Voice& voice);

    Sample sample;
};

class SynthInstrument
{
public:
    SynthInstrument() {}
    void init();
    void render(float** output, int frames, Voice& voice);

    WaveTable wave_table;   
};

#endif