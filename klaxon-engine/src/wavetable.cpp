#include <emscripten/emscripten.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include "wavetable.h"
#include "polyphony.h"

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

// extern "C" {

//     void init_synth(int sample_rate) 
//     {
//         wave_table.init(sample_rate);
//         wave_table.generate_saw();
//         poly.init();
//     }

//     float lerp(float a, float b, float t)
//     {
//         return (1 - t) * a + t * b;
//     }

//     void add_note(int note_id)
//     {
//         poly.add_voice(note_id);
//     }

//     void remove_note(int note_id)
//     {
//         poly.remove_voice(note_id);
//     }

//     void set_unison_count(int instances)
//     {
//         poly.set_unison_count(instances);
//     }


// }


