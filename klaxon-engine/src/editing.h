#ifndef EDITING_H 
#define EDITING_H

#include <cstdint>
#include <vector>
#include <memory>

#include "sample.h"

enum class EditResult : int
{
    OK,
    INVALID_SAMPLE,
    INVALID_RANGE,
    OUT_OF_BOUNDS,
    CLIPBOARD_EMPTY,
    NO_EDIT
};

struct Clipboard
{
    // TODO: Remove variant, switch to destructive editing
    std::unique_ptr<float[]> left;
    std::unique_ptr<float[]> right;
    uint32_t length{0};

    bool empty() const { return  length == 0 || !left || !right; }
};



class SampleEditor 
{
public:
    SampleEditor(){}

    EditResult cut(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard);
    EditResult copy(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard);
    EditResult paste(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard);
    EditResult crop(int sample_id, Sample* sample, uint32_t from, uint32_t to);
    EditResult clear(int sample_id, Sample* sample);

    EditResult reverse(int sample_id, Sample* sample, uint32_t from, uint32_t to);
    EditResult normalise(int sample_id, Sample* sample, uint32_t from, uint32_t to);

    EditResult undo(int sample_id, Sample* sample);
    EditResult redo(int sample_id, Sample* sample);
};

#endif