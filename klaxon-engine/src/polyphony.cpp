#include "polyphony.h"
#include "instrument.h"
#include "utils.h"

void Polyphony::init()
{

}

void Polyphony::add_voice(Instrument* instrument, Sample* sample, int channel_id, int note_id, int instrument_id, float volume, int root_note)
{
    int old = -1;

    for (int v = 0; v < MAX_VOICES; v++) 
    {
        if(voices[v].channel_id == channel_id)
        {
            old = v;
            break;
        }
    }

    if (old != -1 && voices[old].note_id == note_id && voices[old].channel_id == channel_id && voices[old].instrument_id == instrument_id)
    {
        voices[old].volume = 1.0f;
        voices[old].position = 0;
        voices[old].finished = false;
        return;
    }

    if (old != -1)
    {
        voices[old].finished = true;
    }

    // find the first inactive voice
    int i = 0;
    while(i < MAX_VOICES && voices[i].active) i++;

    if(i == MAX_VOICES) return;

    Voice& v = voices[i];
    v = Voice{}; 

    v.note_id = note_id;
    v.channel_id = channel_id;
    v.instrument_id = instrument_id;
    v.rate = equal_temperament(root_note, note_id);
    v.frequency = calculate_frequency(note_id);
    v.active = true;
    v.finished = false;
    v.volume = static_cast<float>(volume)/100.f;
    v.position = 0;
    v.env_val = (instrument->envelope.enabled && instrument->envelope.points[0].active) ? instrument->envelope.points[0].vol : 1.0f;

    v.type = instrument->type;
    v.instrument = instrument;
    v.sample = (instrument->type == InstrumentType::SAMPLE) ? sample : nullptr;

    curr++;
}

void Polyphony::remove_voice(int channel_id, int note_id, int instrument_id)
{
    if(curr <= 0) return; // no active voices

    int v = 0;
    while(v < MAX_VOICES && (voices[v].channel_id != channel_id || voices[v].note_id != note_id || !voices[v].active)) v++;

    if(v < MAX_VOICES) 
    {
        if(voices[v].channel_id == channel_id && voices[v].note_id == note_id && voices[v].active)
        {   
            // just reset everything
            voices[v].active = false;
            voices[v].releasing = true;
            voices[v].note_id = -1;
            voices[v].channel_id = -1;
            voices[v].instrument_id = 0;
            voices[v].volume = 0;
            voices[v].position = 0;
            voices[v].finished = false;
            curr--;
        }
    }
}

void Polyphony::render_voices(float** output, int frames)
{
    for(Voice& voice : voices)
    {
        if (!voice.active) continue;

        if (voice.finished) 
        {
            voice.active = false;
            continue;
        }

        switch (voice.type)
        {
        case InstrumentType::SAMPLE:
            render_sample(voice, output, frames);
            break;
        
        case InstrumentType::SYNTH:
            render_synth(voice, output, frames);
            break;

        default:
            break;
        }        
    }
}

void Polyphony::render_sample(Voice& voice, float** output, int frames)
{
    if(!voice.instrument || voice.type != InstrumentType::SAMPLE || !voice.sample) return;

    for (int i = 0; i < frames; i++)
    {   
        int p = static_cast<int>(voice.position);

        if (p >= voice.sample->length - 1) {
            voice.finished = true;
            break;
        }

        double frac = voice.position - p;
        
        float L0 = voice.sample->left[p];
        float L1 = voice.sample->left[p + 1];
        float L_out_lerp = lerp(L0, L1, frac);

        if (voice.sample->channels == 1) {
            // linear interpolate
            // TODO: Add fast sinc

            output[0][i] += L_out_lerp * voice.env_val * voice.volume;
            output[1][i] += L_out_lerp * voice.env_val * voice.volume;
        } else {
            float R0 = voice.sample->right[p];
            float R1 = voice.sample->right[p + 1];
            float R_out_lerp = lerp(R0, R1, frac);

            output[0][i] += L_out_lerp * voice.env_val * voice.volume;
            output[1][i] += R_out_lerp * voice.env_val * voice.volume;
        }

        voice.position += (voice.backwards) ? -voice.rate : voice.rate;

        if (voice.sample->loop_type == LoopType::NONE) 
        {
            if (voice.position >= voice.sample->length)
            {
                voice.finished = true;
                break;
            }
        } else if (voice.sample->loop_type == LoopType::FORWARD)
        {
            if(voice.position >= voice.sample->loop_to) 
            {
                double overshoot = voice.position - voice.sample->loop_to;
                voice.position = voice.sample->loop_from + overshoot;
            }    
        } else if (voice.sample->loop_type == LoopType::BIDI)
        {
            if (!voice.backwards && voice.position >= voice.sample->loop_to) 
            {
                double overshoot = voice.position - voice.sample->loop_to;
                voice.position = voice.sample->loop_to - overshoot;
                voice.backwards = true;
            } else if (voice.backwards && voice.position <= voice.sample->loop_from) 
            {
                double overshoot =voice.sample->loop_from - voice.position;
                voice.position = voice.sample->loop_from + overshoot;
                voice.backwards = false;
            }
        }
    }
}

void Polyphony::render_synth(Voice& voice, float** output, int frames)
{
    if(!voice.instrument || voice.type != InstrumentType::SYNTH) return;

    auto& synth_instrument = voice.instrument->synth;

    auto& table = synth_instrument.wave_table.table;
    float phaseI = voice.frequency * WaveTable::WAVETABLE_SIZE / sample_rate;
    int wf = static_cast<int>(synth_instrument.waveform);

    for (int i = 0; i < frames; i++)
    {
        int phase = static_cast<int>(voice.phase);
        float frac = voice.phase - phase;

        float S0 = table[wf][phase];
        float S1 = table[wf][(phase + 1) % WaveTable::WAVETABLE_SIZE];

        float S_out = lerp(S0, S1, frac);

        output[0][i] += S_out * voice.volume;
        output[1][i] += S_out * voice.volume;
        
        if (voice.releasing) 
        {
            voice.volume *= 0.999f;

            if(voice.volume <= 0.0f) 
            {
                voice.releasing = false;
                voice.active = false;
            } 

        }

        voice.phase += phaseI;
        if (voice.phase >= WaveTable::WAVETABLE_SIZE) voice.phase -= WaveTable::WAVETABLE_SIZE;
    }
}

void Polyphony::advance_env_tick()
{
    for(Voice& voice : voices)
    {
        if (!voice.active) continue;

        auto& env = voice.instrument->envelope;

        if (!env.enabled) continue;

        voice.env_tick++;

        if (!voice.releasing && env.sustain_at == voice.env_pos) continue;

        if(!voice.releasing && voice.env_tick >= env.points[env.loop_to].tick)
        {
            voice.env_pos = env.loop_from;
            voice.env_tick = env.points[voice.env_pos].tick;
        }

        if (voice.env_pos + 1 >= MAX_POINTS || !env.points[voice.env_pos + 1].active && voice.env_fade == 0.0f)
        {
            voice.env_val = env.points[voice.env_pos].vol / 100.f;

            if (voice.env_val == 0.f) {
                voice.finished = true;
            }

            continue;
        }

        Point& P0 = env.points[voice.env_pos];
        Point& P1 = env.points[voice.env_pos + 1];

        if (voice.env_tick == P1.tick) {
            voice.env_pos++;
            voice.env_val = P1.vol / 100.f;
            continue;
        }
        
        int tick_diff = P1.tick - P0.tick;
        float frac = static_cast<float>(voice.env_tick - P0.tick) / tick_diff;

        voice.env_val = lerp(P0.vol / 100.0f, P1.vol / 100.0f, frac);
    }
}

int Polyphony::get_current_voices() const
{
    return curr;
}