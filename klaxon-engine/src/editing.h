#ifndef EDITING_H
#define EDITING_H

#include <cstdint>
#include <vector>
#include <variant>
#include <memory>

#include "sample.h"

enum class SampleOperation : int
{
    CUT,
    PASTE,
    CROP,
    CLEAR,
    REVERSE,
    NORMALISE
};

struct CopyBuffer
{
    int32_t sample_id{-1};
    uint32_t from{0};
    uint32_t to{0};
};

struct CutBuffer
{
    std::unique_ptr<float[]> left;
    std::unique_ptr<float[]> right;
    uint32_t length{0};
};

using Clipdata = std::variant<std::monostate, CopyBuffer, CutBuffer>;

struct Clipboard
{
    Clipdata data;

    bool empty() { return std::holds_alternative<std::monostate>(data);}
    bool is_copy() { return std::holds_alternative<CopyBuffer>(data);}
    bool is_cut() { return std::holds_alternative<CutBuffer>(data);}
};

struct SampleSnapshot
{
    SampleOperation op;

    uint32_t from{0};
    uint32_t to{0};

    std::unique_ptr<float[]> left;
    std::unique_ptr<float[]> right;
    uint32_t length{0};
};

struct History
{
    static constexpr int MAX_UNDO = 16;
    std::vector<SampleSnapshot> history;
    int cursor{-1};

    void push(SampleSnapshot snap);
    void undo();
    void redo();
};

class SampleEditor
{
public:
    SampleEditor(){}

    void cut(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard);
    void copy(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard);
    void paste(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard);
    void crop(int sample_id, Sample* sample, uint32_t from, uint32_t to);
    void clear(int sample_id, Sample* sample);

    void reverse(int sample_id, Sample* sample, uint32_t from, uint32_t to);
    void normalise(int sample_id, Sample* sample, uint32_t from, uint32_t to);

    void undo();
    void redo();

    History edit_history;
};

#endif