#ifndef SAMPLE_H
#define SAMPLE_H

#include <vector>
#include "lib/miniaudio.h"

class Sample {
public:
    Sample() {}

    void load_sample(const char* filename, const void* data, unsigned long length);

    const char* filename;

    // raw PCM
    std::vector<float> left; 
    std::vector<float> right;

    int channels;
    ma_uint64 duration;

};

#endif