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

    static constexpr int MAX_NOTES = 128;

    SampleInstrument() {}
    SampleInstrument(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void init(const char* filename, float* left, float* right, int sample_rate, unsigned long length);
    void render(float** output, int frames, Voice& voice);

    Sample sample;

    int note_sample[MAX_NOTES];  // note → sample_id (0->119)
    int sample_ids[16]; // sample ids used by this instrument
};

class SynthInstrument
{
public:
    SynthInstrument() {}
    void init();
    void render(float** output, int frames, Voice& voice);

    WaveTable wave_table;   
};


// struct Instrument
// {
//     void (*render)(Instrument*, float**, int, Voice&);

//     union
//     {
//         SampleInstrument sample;
//         SynthInstrument synth;
//     };
// };

// void render_synth(Instrument* instr, float** output, int frames, Voice& voice) 
// {
//     auto& sample = instr->sample;
//     sample.render(output, frames, voice);
// }

// void render_synth(Instrument* instr, float** output, int frames, Voice& voice) 
// {
//     auto& synth = instr->synth;
//     synth.render(output, frames, voice);
// }


#endif