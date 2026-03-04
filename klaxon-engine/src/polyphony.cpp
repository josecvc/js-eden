#include "polyphony.h"
#include "midi.h"

void Polyphony::init()
{

}

void Polyphony::add_voice(int channel_id, int note_id, int instrument_id, float volume, int root_note)
{
    if(curr == MAX_VOICES) return; // reached limit (in the future add note stealing if necessary)

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

    voices[i].note_id = note_id;
    voices[i].channel_id = channel_id;
    voices[i].instrument_id = instrument_id;
    voices[i].rate = powf(2, static_cast<float>(note_id - root_note)/12);
    voices[i].active = true;
    voices[i].finished = false;
    voices[i].volume = volume;
    voices[i].position = 0;

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

int Polyphony::get_current_voices() const
{
    return curr;
}