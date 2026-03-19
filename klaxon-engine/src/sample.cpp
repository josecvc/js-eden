#include "sample.h"
#include <memory>

//TODO: TEST THIS PLEASE
void Sample::load_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length)
{   
    this->filename = std::string(filename);

    this->left = std::make_unique<float[]>(length);
    this->right = std::make_unique<float[]>(length);

    std::memcpy(this->left.get(), left, length * sizeof(float));
    std::memcpy(this->right.get(), right, length * sizeof(float));
    
    this->sample_rate = sample_rate;
    this->length = length;

    std::free(left);
    std::free(right);
}