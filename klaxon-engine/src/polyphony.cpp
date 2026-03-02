#include "polyphony.h"
#include "midi.h"

// SYNTH POLYPHONY //

void SynthPolyphony::init() 
{
    uni.instances = 1;
}

void SynthPolyphony::add_voice(int channel_id, int note_id, float vel) 
{
    if(curr == MAX_VOICES) return; // reached limit (in the future add note stealing)

    bool flag = false;
    for(int v = 0; v < MAX_VOICES; v++) // check if note is already there
    {
        if(voices[v].note_id == note_id && voices[v].channel_id == channel_id) {
            flag = true;
            voices[v].velocity = 1.0f; // reignite the note
        }
        
    }

    if(flag) return;

    // find the first inactive voice
    int v = 0;
    while(voices[v].active) v++;

    // find the next set of inactive voices 
    // would be better if only one voice had a set of frequencies to iterate through
    // would require a vector of frequencies according to the number of unison instances
    for(int i = 0; i < uni.instances && i + v < MAX_VOICES; i++)
    {
        voices[i + v].active = true;
        voices[i + v].releasing = false;
        voices[i + v].note_id = note_id;
        voices[i + v].frequency = calculate_frequency(note_id) + 2*i;
        voices[i + v].velocity = vel;
        voices[i + v].phase = i * 10;
        curr++;
    }
}

int SynthPolyphony::get_current_voices() const
{
    return curr;
}

void SynthPolyphony::remove_voice(int channel_id, int note_id)
{
    if(curr <= 0) return; // no active voices

    for(int v = 0; v < MAX_VOICES; v++)
    {
        if(voices[v].channel_id == channel_id && voices[v].note_id == note_id && voices[v].active)
        { // note is still releasing
            voices[v].releasing = true;
            voices[v].active = false;
            voices[v].note_id = -1;
            curr--;
        }
    }
}

void SynthPolyphony::set_unison_count(int instances)
{
    uni.instances = instances;
}

// SAMPLE POLYPHONY //

void SamplePolyphony::init()
{

}

void SamplePolyphony::add_voice(int channel_id, int note_id, float gain)
{
    if(curr == MAX_VOICES) return; // reached limit (in the future add note stealing)

    bool flag = false;

    // check if it has already been fired in that channel
    int v = 0;
    while(v < MAX_VOICES && (voices[v].note_id != note_id || voices[v].channel_id != channel_id || !voices[v].active)) v++;

    if (v < MAX_VOICES) {
        if(voices[v].active && (voices[v].note_id == note_id && voices[v].channel_id == channel_id)) {
            flag = true;
            voices[v].gain = 1.0f; // reignite the note
            voices[v].position = 0;
            return;
        }
    }

    // find the first inactive voice
    int i = 0;
    while(i < MAX_VOICES && voices[i].active) i++;

    if(i == MAX_VOICES) return;

    voices[i].note_id = note_id;
    voices[i].channel_id = channel_id;
    voices[i].active = true;
    voices[i].gain = gain;
    voices[i].position = 0;

    curr++;
}

void SamplePolyphony::remove_voice(int channel_id, int note_id)
{
    if(curr <= 0) return; // no active voices

    int v = 0;
    while(v < MAX_VOICES && (voices[v].channel_id != channel_id || voices[v].note_id != note_id || !voices[v].active)) v++;

    if(v < MAX_VOICES) {
        if(voices[v].channel_id == channel_id && voices[v].note_id == note_id && voices[v].active)
        {
            voices[v].active = false;
            voices[v].note_id = -1;
            voices[v].channel_id = -1;
            voices[v].gain = 0.f;
            voices[v].position = 0;
            curr--;
        }
    }
}

int SamplePolyphony::get_current_voices() const
{
    return curr;
}