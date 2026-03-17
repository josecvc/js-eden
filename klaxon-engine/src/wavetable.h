#ifndef WAVETABLE_H
#define WAVETABLE_H

enum class Waveform : int
{
    SINE = 0,
    SAW = 1,
    TRIANGLE = 2,
    SQUARE = 3
};

class WaveTable 
{
public:
    static constexpr int WAVETABLE_SIZE = 2048;
    static constexpr int NUM_WAVES = 4;

    WaveTable() : sample_rate(44100.0f) {}
    
    void init(float sample_rate);

    void generate_sine();
    void generate_saw();
    void generate_triangle();
    void generate_square();

    float table[NUM_WAVES][WAVETABLE_SIZE];
    float phase = 0.0f;
    float sample_rate;
};

#endif