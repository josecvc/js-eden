#include "sample.h"
#include <cstdint>
#include <memory>

void Sample::init(uint32_t length)
{
    this->left = std::make_unique<float[]>(length);
    this->right = std::make_unique<float[]>(length);

    std::memset(left.get(), 0, length * sizeof(float));
    std::memset(right.get(), 0, length * sizeof(float));
}

void Sample::load_sample(const char* filename, float* left, float* right, int sample_rate, uint32_t length)
{   
    this->filename = std::string(filename);

    this->init(length);

    std::memcpy(this->left.get(), left, length * sizeof(float));
    std::memcpy(this->right.get(), right, length * sizeof(float));
    
    this->sample_rate = sample_rate;
    this->length = length;

    std::free(left);
    std::free(right);
}

void History::push(SampleSnapshot snap)
{
    history.push_back(std::move(snap));
}