#ifndef UTILS_H
#define UTILS_H

#include <math.h>
#include <cstdint>
#include <algorithm>

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

inline int get_bins(float* left, float* right, float* dst, uint32_t length)
{
    if (!left || !right) return -1;

    const int DOWNSAMPLE = 1024;

    const int block_size = static_cast<int>(floor(length / DOWNSAMPLE));

    for (int i = 0; i < DOWNSAMPLE; i++) {
        float sum = 0;
        const int start = i * block_size;
        const int end = std::min(static_cast<uint32_t>(start + block_size), length);
        
        for (int j = start; j < end; j++) sum += (left[j] + right[j]) * 0.5f;

        dst[i] = sum / static_cast<float>(end - start);
    }

    return 0;
}

#endif