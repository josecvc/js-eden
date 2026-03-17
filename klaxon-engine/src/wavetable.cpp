#include <emscripten/emscripten.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "wavetable.h"
#include "polyphony.h"

void WaveTable::init(float sample_rate)
{
    this->sample_rate = sample_rate;

    generate_sine();
    generate_saw();
    generate_triangle();
    generate_square();
}

void WaveTable::generate_sine() 
{
    for(int i=0; i < WAVETABLE_SIZE; i++)
    {
        this->table[static_cast<int>(Waveform::SINE)][i] = sinf(2.0f * M_PI * i / WAVETABLE_SIZE);
    }
}

void WaveTable::generate_saw() 
{
    for(int i=0; i < WAVETABLE_SIZE; i++)
    {  
        float period_part = static_cast<float>(i) / WAVETABLE_SIZE;
        this->table[static_cast<int>(Waveform::SAW)][i] = period_part * 2 - 1;
    }
}

void WaveTable::generate_triangle()
{
    for(int i = 0; i < WAVETABLE_SIZE; i++)
    {
        float period_part = static_cast<float>(i) / WAVETABLE_SIZE;

        if (period_part < 0.5f)
            this->table[static_cast<int>(Waveform::TRIANGLE)][i] = -1.0f + (period_part * 4.0f);

        else
            this->table[static_cast<int>(Waveform::TRIANGLE)][i] = 3.0f - (period_part * 4.0f);
    }
}

void WaveTable::generate_square()
{
    int half = WAVETABLE_SIZE / 2;

    for(int i = 0; i < half; i++)
    {
        this->table[static_cast<int>(Waveform::SQUARE)][i] = 1.f;
    }

    for(int i = half; i < WAVETABLE_SIZE; i++)
    {
        this->table[static_cast<int>(Waveform::SQUARE)][i] = -1.f;
    }
}