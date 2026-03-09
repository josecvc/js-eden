#include "sample.h"

//TODO: TEST THIS PLEASE
void Sample::load_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length)
{   
    this->filename = filename;

    this->left = left;
    this->right = right; 
    
    this->sample_rate = sample_rate;
    this->length = length;
}