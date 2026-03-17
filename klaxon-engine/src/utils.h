#ifndef UTILS_H
#define UTILS_H

#include <math.h>

inline float equal_temperament(int root, int note)
{
    return exp2f(static_cast<float>(note - root)/12);
}

inline float calculate_frequency(int note_id)
{
    return 440.0f * equal_temperament(note_id, 69);
}

inline float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

inline float sinc(float x)
{
    if (x == 0.f) return 0.f;

    return sinf(x)/x;
}

#endif