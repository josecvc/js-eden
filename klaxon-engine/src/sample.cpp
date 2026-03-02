#include "sample.h"

#include <vector>
#include <algorithm>

//TODO: TEST THIS PLEASE
void Sample::load_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length)
{   
    this->filename = filename;

    this->left.resize(length); 
    this->right.resize(length); 

    std::copy(left, left + length, this->left.begin()); 
    std::copy(right, right + length, this->right.begin());
    
    this->sample_rate = sample_rate;
    this->length = length;
}