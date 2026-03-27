#ifndef SAMPLE_H
#define SAMPLE_H

#include <memory>
#include <string>
#include <cstdint>
#include "editing.h"

enum class LoopType : int
{
    NONE,
    FORWARD,
    BIDI
};

class Sample 
{
public:
    Sample() {}

    void init(uint32_t length);
    void load_sample(const char* filename, float* left, float* right, int sample_rate, uint32_t length);

    std::string filename;

    // raw PCM
    std::unique_ptr<float[]> left;
    std::unique_ptr<float[]> right;

    uint32_t length;
    int channels;
    int sample_rate;

    float volume{1.f};
    int root_note{72}; // Assume C-5 at first
    int rel_note{0};       
    float finetune{0.f};    

    LoopType loop_type{LoopType::NONE};
    uint32_t loop_from;
    uint32_t  loop_to;

    History edit_history;
};

#endif