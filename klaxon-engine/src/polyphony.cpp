#include "polyphony.h"
#include "midi.h"

void Polyphony::init() 
{
    uni.instances = 1;
}

void Polyphony::add_voice(int note_id) 
{
    if(curr == MAX_VOICES) return; // reached limit (in the future add note stealing)

    bool flag = false;
    for(int v = 0; v < MAX_VOICES; v++) // check if note is already there
    {
        if(voices[v].note_id == note_id) {
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
        voices[i + v].velocity = 1.0f;
        voices[i + v].phase = i * 10;
        curr++;
    }
}

int Polyphony::get_current_voices()
{
    return curr;
}

void Polyphony::remove_voice(int note_id)
{
    if(curr <= 0) return; // no active voices

    for(int v = 0; v < MAX_VOICES; v++)
    {
        if(voices[v].note_id == note_id && voices[v].active)
        { // note is still releasing
            voices[v].releasing = true;
            voices[v].active = false;
            voices[v].note_id = -1;
            curr--;
        }
    }
}

void Polyphony::set_unison_count(int instances)
{
    uni.instances = instances;
}