#define _USE_MATH_DEFINES
#include <math.h>
#include "dsp.h"

static WaveTable wave_table;

void WaveTable::init(float sample_rate)
{
    this->sample_rate = sample_rate;
}

void WaveTable::generate_sine() 
{
    for(int i=0; i < WAVETABLE_SIZE; i++)
    {
        this->table[i] = sinf(2.0f * M_PI * i / WAVETABLE_SIZE);
    }
}

extern "C" {
    void init_wavetable(int sample_rate) 
    {
        wave_table.init(sample_rate);
        wave_table.generate_sine();
    }

    float lerp(float a, float b, float t)
    {
        return (1 - t) * a + t * b;
    }

    void process(float* input, float* output, int frames) 
    {
        float phaseI = 523.2511  * WaveTable::WAVETABLE_SIZE / wave_table.sample_rate;
        for (int i = 0; i < frames; i ++)
        {
            output[i] = wave_table.table[static_cast<int>(wave_table.phase)];
            wave_table.phase += phaseI;
            if (wave_table.phase >= WaveTable::WAVETABLE_SIZE) wave_table.phase -= WaveTable::WAVETABLE_SIZE;
        }
    }
}


