#include "polyphony.h"
#include "midi.h"

void Polyphony::init() 
{

}

void Polyphony::add_voice(int note_id) 
{
    if(curr == MAX_VOICES) return; // reached limit (in the future add note stealing)

    for(int v = 0; v < MAX_VOICES; v++) // check if note is already there
    {
        if(voices[v].note_id == note_id) {
            voices[v].velocity = 1.0f; // reignite the note
            return;
        }
            
    }

    // find the first inactive voice
    for(int v = 0; v < MAX_VOICES; v++)
    {
        if(!voices[v].active) 
        {
            voices[v].active = true;
            voices[v].releasing = false;
            voices[v].note_id = note_id;
            voices[v].frequency = calculate_frequency(note_id);
            voices[v].velocity = 1.0f;
            curr++;
            return;
        }
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
            return;
        }
    }
}