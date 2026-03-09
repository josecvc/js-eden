#ifndef SAMPLE_H
#define SAMPLE_H

#include <vector>

class Sample {
public:
    Sample() {}

    void load_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length);

    // raw PCM
    float* left; 
    float* right;

    const char* filename;
    int channels;
    int sample_rate;
    int root_note{72}; // Assume C-5
    unsigned long length;
};

#endif