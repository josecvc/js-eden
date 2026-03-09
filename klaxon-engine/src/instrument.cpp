#include "instrument.h"

void SampleInstrument::render(float** output, int frames, Voice& voice, const Sample* sample)
{    
    for (int i = 0; i < frames; i++)
    {   
        // Add linear interpolaton for different rates
        
        // calculate time difference for interpolation
        int p = static_cast<int>(voice.position);

        // static cast could cause p to be larger than the length
        if (p >= sample->length - 1) {
            voice.finished = true;
            break;
        }

        double frac = voice.position - p;
        

        if (sample->channels == 1) {
            // linear interpolate

            float L0 = sample->left[p];
            float L1 = sample->left[p + 1];

            float out_lerp = L0 + (L1 - L0) * frac;

            output[0][i] += out_lerp;
            output[1][i] += out_lerp;
        } else {
            float L0 = sample->left[p];
            float L1 = sample->left[p + 1];

            float R0 = sample->right[p];
            float R1 = sample->right[p + 1];

            float L_out_lerp = L0 + (L1 - L0) * frac;
            float R_out_lerp = R0 + (R1 - R0) * frac;

            output[0][i] += L_out_lerp;
            output[1][i] += R_out_lerp;
        }

        voice.position += voice.rate;

        if (voice.position >= sample->length)
        {
            voice.finished = true;
            break;
        }
    }
}

void SynthInstrument::render(float** output, int frames, Voice& voice)
{
    float phaseI = voice.frequency * WaveTable::WAVETABLE_SIZE / wave_table.sample_rate;
    
    for (int i = 0; i < frames; i++) 
    {
    
        if (voice.releasing)  // release gracefully
        {    
            output[0][i] += wave_table.table[static_cast<int>(voice.phase)] * voice.volume;
            output[1][i] += wave_table.table[static_cast<int>(voice.phase)] * voice.volume;

            voice.volume *= 0.999f;

            if(voice.volume <= 0.0f) 
            {
                voice.releasing = false;
                voice.finished = true;
            } 

            else if(voice.active) 
            {
            // use (1.0f / static_cast<float>(poly.curr)) for no clipping
                output[0][i] += wave_table.table[static_cast<int>(voice.phase)];
                output[1][i] += wave_table.table[static_cast<int>(voice.phase)];
            }
        }

        voice.phase += phaseI;
        if (voice.phase >= WaveTable::WAVETABLE_SIZE) voice.phase -= WaveTable::WAVETABLE_SIZE;
    }
}
