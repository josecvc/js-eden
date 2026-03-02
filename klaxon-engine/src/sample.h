#ifndef SAMPLE_H
#define SAMPLE_H

#include <vector>

class Sample {
public:
    Sample() {}

    void load_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length);

    

    // raw PCM
    std::vector<float> left; 
    std::vector<float> right;

    const char* filename;
    int channels;
    int sample_rate;
    unsigned long length;
};

#endif