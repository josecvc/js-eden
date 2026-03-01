#ifndef DSP_H
#define DSP_H

// TODO: Total refactor, this needs to be a synth.h instead
class WaveTable 
{
public:
    static constexpr int WAVETABLE_SIZE = 2048;

    WaveTable() : sample_rate(44100.0f) {}
    
    void init(float sample_rate);
    void generate_sine();
    void generate_saw();
    void generate_triangle();

    float table[WAVETABLE_SIZE];
    float phase = 0.0f;
    float sample_rate;
};

#endif