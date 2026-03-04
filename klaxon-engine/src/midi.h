#ifndef MIDI_H
#define MIDI_H

#include <math.h>

inline float calculate_frequency(int note_id)
{
    return 440.0f * exp2f((note_id - 69)/12.0f);
}

#endif