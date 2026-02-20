#include <emscripten/emscripten.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "dsp.h"
#include "polyphony.h"

static WaveTable wave_table;
static Polyphony poly;

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

void WaveTable::generate_saw() 
{
    for(int i=0; i < WAVETABLE_SIZE; i++)
    {  
        float periodPart = static_cast<float>(i) / WAVETABLE_SIZE;
        this->table[i] = periodPart * 2 - 1;
    }
}

extern "C" {

    void init_synth(int sample_rate) 
    {
        wave_table.init(sample_rate);
        wave_table.generate_saw();
        poly.init();
    }

    float lerp(float a, float b, float t)
    {
        return (1 - t) * a + t * b;
    }

    void add_note(int note_id)
    {
        poly.add_voice(note_id);
    }

    void remove_note(int note_id)
    {
        poly.remove_voice(note_id);
    }

    void set_unison_count(int instances)
    {
        poly.set_unison_count(instances);
    }

    void process(float* input, float* output, int frames) 
    {   
        for (int i = 0; i < frames; i++) // clear buffer
        {
            output[i] = 0;
        }

        if(poly.curr <= 0) return;

        // for each voice

        for(int v = 0; v < poly.MAX_VOICES; v++)
        {
            float phaseI = poly.voices[v].frequency * WaveTable::WAVETABLE_SIZE / wave_table.sample_rate;
            
            for (int i = 0; i < frames; i++) // go through frames
            {
                if (poly.voices[v].releasing) { // release gracefully
                    output[i] += wave_table.table[static_cast<int>(poly.voices[v].phase)] * poly.voices[v].velocity;

                    poly.voices[v].velocity *= 0.999f;

                    if(poly.voices[v].velocity <= 0.0f) {
                        poly.voices[v].releasing = false;
                        poly.voices[v].active = false;
                    } 

                } else if(poly.voices[v].active) {
                    // use (1.0f / static_cast<float>(poly.curr)) for no clipping
                     output[i] += wave_table.table[static_cast<int>(poly.voices[v].phase)]; 
                }

                poly.voices[v].phase += phaseI;
                if (poly.voices[v].phase >= WaveTable::WAVETABLE_SIZE) poly.voices[v].phase -= WaveTable::WAVETABLE_SIZE;
            }
        }
    }
}


