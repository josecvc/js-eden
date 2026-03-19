#include "editing.h"
#include <cstring>
#include <memory>
#include "sample.h"

void SampleEditor::cut(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard)
{
    if (!sample || from == to || sample_id < 0 || from > to || to > sample->length) return;

    uint32_t cut_length = to - from;
    uint32_t new_length = sample->length - cut_length;

    CutBuffer cb = {nullptr, nullptr, cut_length};

    // perform the cut

    std::unique_ptr<float[]> cut_left = std::make_unique<float[]>(cut_length);
    std::unique_ptr<float[]> cut_right = std::make_unique<float[]>(cut_length);
   
    std::memcpy(cut_left.get(), sample->left.get() + from, cut_length * sizeof(float));
    std::memcpy(cut_right.get(), sample->right.get() + from, cut_length * sizeof(float));

    std::unique_ptr<float[]> snap_left = std::make_unique<float[]>(cut_length);
    std::unique_ptr<float[]> snap_right = std::make_unique<float[]>(cut_length);

    std::memcpy(snap_left.get(), cut_left.get(), cut_length * sizeof(float));
    std::memcpy(snap_right.get(), cut_right.get(), cut_length * sizeof(float));

    // save snapshot of operation and insert into history

    SampleSnapshot snap;
    snap.from = from;
    snap.to = to;
    snap.length = cut_length;
    snap.op = SampleOperation::CUT;

    snap.left = std::move(snap_left); 
    snap.right = std::move(snap_right);
    edit_history.push(std::move(snap));

    // reshape the sample
    // 0 -> from
    // to -> length

    std::unique_ptr<float[]> new_left = std::make_unique<float[]>(new_length);
    std::unique_ptr<float[]> new_right = std::make_unique<float[]>(new_length);

    std::memset(new_left.get(), 0, new_length * sizeof(float));
    std::memset(new_right.get(), 0, new_length * sizeof(float));

    std::memcpy(new_left.get(), sample->left.get(), from * sizeof(float));
    std::memcpy(new_left.get() + from, sample->left.get() + to, (sample->length - to) * sizeof(float));

    std::memcpy(new_right.get(), sample->right.get(), from * sizeof(float));
    std::memcpy(new_right.get() + from, sample->right.get() + to, (sample->length - to) * sizeof(float));

    sample->left = std::move(new_left);
    sample->right = std::move(new_right);

    sample->length -= cut_length;

    // set clipboard to hold pcm data

    cb.left = std::move(cut_left);
    cb.right = std::move(cut_right);

    clipboard.data = std::move(cb);
}

void SampleEditor::copy(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard)
{
    if (!sample || from == to || sample_id < 0 || from > to || to > sample->length) return;

    // copy is just a reference to the sample
    CopyBuffer cb = {sample_id, from, to};

    clipboard.data = cb;
}
void SampleEditor::paste(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard)
{
    if (!sample || from == to || sample_id < 0 || from > to || to > sample->length) return;

}
void SampleEditor::crop(int sample_id, Sample* sample, uint32_t from, uint32_t to)
{

}

void SampleEditor::reverse(int sample_id, Sample* sample, uint32_t from, uint32_t to)
{

}
void SampleEditor::normalise(int sample_id, Sample* sample, uint32_t from, uint32_t to)
{

}

void SampleEditor::undo()
{

}

void SampleEditor::redo()
{

}