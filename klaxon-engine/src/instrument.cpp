#include "instrument.h"

void SampleInstrument::render(float** output, int frames)
{
    // TODO: Account for each voice please
    
    if (poly.curr <= 0) return;

    for (int v = 0; v < poly.MAX_VOICES; v++) { // go through voices
        
        if(!poly.voices[v].active) continue;

        for (int i = 0; i < frames; i++)
        {
            if (sample->channels == 1) {
                output[0][i] += sample->left[position];
                output[1][i] += sample->left[position];
            } else {
                output[0][i] += sample->left[position];
                output[1][i] += sample->right[position];
            }

            position++; // needs to be unique to each voice

            if (position >= sample->duration) position = 0;
        }
    }
}

void SynthInstrument::render(float** output, int frames)
{
    if(poly.curr <= 0) return;

    // for each voice

    for(int v = 0; v < poly.MAX_VOICES; v++)
    {
        float phaseI = poly.voices[v].frequency * WaveTable::WAVETABLE_SIZE / wave_table.sample_rate;
        
        for (int i = 0; i < frames; i++) // go through frames
        {
            if (poly.voices[v].releasing) { // release gracefully
                output[0][i] += wave_table.table[static_cast<int>(poly.voices[v].phase)] * poly.voices[v].velocity;
                output[1][i] += wave_table.table[static_cast<int>(poly.voices[v].phase)] * poly.voices[v].velocity;

                poly.voices[v].velocity *= 0.999f;

                if(poly.voices[v].velocity <= 0.0f) {
                    poly.voices[v].releasing = false;
                    poly.voices[v].active = false;
                } 

            } else if(poly.voices[v].active) {
                // use (1.0f / static_cast<float>(poly.curr)) for no clipping
                output[0][i] += wave_table.table[static_cast<int>(poly.voices[v].phase)];
                output[1][i] += wave_table.table[static_cast<int>(poly.voices[v].phase)];
            }

            poly.voices[v].phase += phaseI;
            if (poly.voices[v].phase >= WaveTable::WAVETABLE_SIZE) poly.voices[v].phase -= WaveTable::WAVETABLE_SIZE;
        }
    }
}