#include "instrument.h"

SampleInstrument::SampleInstrument(const char* filename, float* left, float* right, int sample_rate, unsigned long length) 
{
    init(filename, left, right, sample_rate, length);
}

void SampleInstrument::init(const char* filename, float* left, float* right, int sample_rate, unsigned long length) 
{
    sample.load_sample(filename, left, right, sample_rate, length);
}

void SampleInstrument::render(float** output, int frames)
{    
    if (poly.curr <= 0) return;

    for (int v = 0; v < poly.MAX_VOICES; v++) { // go through voices
        
        if(!poly.voices[v].active) continue;
        for (int i = 0; i < frames; i++)
        {
            if (sample.channels == 1) {
                output[0][i] += sample.left[poly.voices[v].position];
                output[1][i] += sample.left[poly.voices[v].position];
            } else {
                output[0][i] += sample.left[poly.voices[v].position];
                output[1][i] += sample.right[poly.voices[v].position];

                // output[0][i] += sinf(1.5f * 3.14159f * i/frames);
                // output[1][i] += sinf(1.5f * 3.14159f * i/frames);
            }

            ++poly.voices[v].position;

            if (poly.voices[v].position >= sample.length)
            {
                poly.remove_voice(poly.voices[v].channel_id, poly.voices[v].note_id);
            }
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

// simply call the polyphony
void SampleInstrument::add_voice(int channel_id, int note_id)
{
    poly.add_voice(channel_id, note_id, 1.f);
}

void SampleInstrument::remove_voice(int channel_id, int note_id)
{
    poly.remove_voice(channel_id, note_id);
}

void SynthInstrument::add_voice(int channel_id, int note_id)
{
    poly.add_voice(channel_id, note_id, 1.f);
}

void SynthInstrument::remove_voice(int channel_id, int note_id)
{
    poly.remove_voice(channel_id, note_id);
}