#include "editing.h"
#include <cstring>
#include <memory>
#include "sample.h"

EditResult SampleEditor::cut(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard)
{
    if (!sample || !sample->left || !sample->right) return EditResult::INVALID_SAMPLE;

    if (from > to || from == to) return EditResult::INVALID_RANGE;

    if (from < 0 || to >= sample->length) return EditResult::OUT_OF_BOUNDS;

    uint32_t cut_length = to - from;
    uint32_t new_length = sample->length - cut_length;

    // perform the cut

    std::unique_ptr<float[]> cut_left = std::make_unique<float[]>(cut_length);
    std::unique_ptr<float[]> cut_right = std::make_unique<float[]>(cut_length);
   
    std::memcpy(cut_left.get(), sample->left.get() + from, cut_length * sizeof(float));
    std::memcpy(cut_right.get(), sample->right.get() + from, cut_length * sizeof(float));

    // snapshot old sample
    
    std::unique_ptr<float[]> snap_left = std::make_unique<float[]>(sample->length);
    std::unique_ptr<float[]> snap_right = std::make_unique<float[]>(sample->length);

    std::memcpy(snap_left.get(), sample->left.get(), sample->length * sizeof(float));
    std::memcpy(snap_right.get(), sample->right.get(), sample->length * sizeof(float));

    // save snapshot of operation and insert into history

    SampleSnapshot snap;
    snap.from = from;
    snap.to = to;
    snap.length = cut_length;
    snap.op = SampleOperation::CUT;

    snap.left = std::move(snap_left); 
    snap.right = std::move(snap_right);

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

    // adjust loop points
    

    sample->left = std::move(new_left);
    sample->right = std::move(new_right);

    sample->length -= cut_length;

    // set clipboard to hold pcm data
    clipboard.left = std::move(cut_left);
    clipboard.right = std::move(cut_right);
    clipboard.length = cut_length;

    sample->edit_history.push(std::move(snap));

    return EditResult::OK;
}

EditResult SampleEditor::copy(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard)
{
    if (!sample || !sample->left || !sample->right) return EditResult::INVALID_SAMPLE;

    if (from < 0 || to >= sample->length) return EditResult::OUT_OF_BOUNDS;

    if (from == to || from > to) return EditResult::INVALID_RANGE;

    uint32_t copy_length = to - from;

    // perform the copy

    // TODO: Put this sequence of operations inside the clipboard
    std::unique_ptr<float[]> copy_left = std::make_unique<float[]>(copy_length);
    std::unique_ptr<float[]> copy_right = std::make_unique<float[]>(copy_length);
   
    std::memcpy(copy_left.get(), sample->left.get() + from, copy_length * sizeof(float));
    std::memcpy(copy_right.get(), sample->right.get() + from, copy_length * sizeof(float));

    // set clipboard to hold pcm data
    clipboard.left = std::move(copy_left);
    clipboard.right = std::move(copy_right);
    clipboard.length = copy_length;

    return EditResult::OK;
}

EditResult SampleEditor::paste(int sample_id, Sample* sample, uint32_t from, uint32_t to, Clipboard& clipboard)
{
    /**
     * 1. Paste into empty sample (0, clipboard.length)
     * 2. Paste at a specific sample point (from == to)
     * 3. Paste in window (from, from + clipboard.length, to)
     */

    if (!sample) return EditResult::INVALID_SAMPLE;

    if (from > to) return EditResult::INVALID_RANGE;

    if (from < 0 || to >= sample->length) return EditResult::OUT_OF_BOUNDS;

    if (clipboard.empty()) return EditResult::CLIPBOARD_EMPTY;
    
    SampleSnapshot snap;
    snap.op = SampleOperation::PASTE;

    // (1) do a simple paste if empty
    if (!sample->left || !sample->right || sample->length == 0)
    {
        // default to empty
        snap.length = 0;
        snap.from = 0;
        snap.to = 0;
        snap.left = nullptr;
        snap.right = nullptr;

        // paste all clip data into sample
        sample->init(clipboard.length);
        std::memcpy(sample->left.get(), clipboard.left.get(), clipboard.length * sizeof(float));
        std::memcpy(sample->right.get(), clipboard.right.get() , clipboard.length * sizeof(float));
    }
    // (2)(3) paste into existing sample
    else
    {
        // grow/shrink the pcm pointers
        uint32_t old_length = sample->length;
        uint32_t new_length = sample->length - (to - from) + clipboard.length;

        // save snap
        snap.length = old_length;
        snap.from = from;
        snap.to = to;

        std::unique_ptr<float[]> left_temp = std::move(sample->left);
        std::unique_ptr<float[]> right_temp = std::move(sample->right);

        sample->init(new_length);

        // shift the pcm by clipboard.length samples starting at "from"
        // memcpy starting at from
        // then memcpy from "to" up to "sample->length" (it doesn't matter if "to" and "from" are equal or not)

        std::memcpy(sample->left.get(), left_temp.get(), from * sizeof(float));
        std::memcpy(sample->right.get(), right_temp.get(), from * sizeof(float));

        std::memcpy(sample->left.get() + from + clipboard.length, left_temp.get() + to, (old_length - to) * sizeof(float));
        std::memcpy(sample->right.get() + from + clipboard.length, right_temp.get() + to, (old_length - to) * sizeof(float));

        // append clip data to empty space
        std::memcpy(sample->left.get() + from, clipboard.left.get(), clipboard.length * sizeof(float));
        std::memcpy(sample->right.get() + from, clipboard.right.get(), clipboard.length * sizeof(float));

        sample->length = new_length;

        snap.left = std::move(left_temp);
        snap.right = std::move(right_temp);
    }

    // snap!
    sample->edit_history.push(std::move(snap));

    return EditResult::OK;
}

EditResult SampleEditor::crop(int sample_id, Sample* sample, uint32_t from, uint32_t to)
{
    if (!sample || !sample->left || !sample->right) return EditResult::INVALID_SAMPLE;

    if (from < 0 || to >= sample->length) return EditResult::OUT_OF_BOUNDS;

    if (from == to || from > to || to - from > sample->length) return EditResult::INVALID_RANGE;

    if (to - from == sample->length) return EditResult::NO_EDIT;

    uint32_t old_length = sample->length;
    uint32_t new_length = to - from;
    uint32_t cropped_out_length = from + (old_length - to);

    // snapshot the old data
    SampleSnapshot snap;
    
    std::unique_ptr<float[]> snap_left = std::make_unique<float[]>(sample->length);
    std::unique_ptr<float[]> snap_right = std::make_unique<float[]>(sample->length);

    std::memcpy(snap_left.get(), sample->left.get(), sample->length * sizeof(float));
    std::memcpy(snap_right.get(), sample->right.get(), sample->length * sizeof(float));

    snap.from = from;
    snap.to = to;
    snap.length = sample->length;
    snap.op = SampleOperation::CROP;
    snap.left = std::move(snap_left);
    snap.right = std::move(snap_right);

    // get old data
    std::unique_ptr<float[]> left_temp = std::move(sample->left);
    std::unique_ptr<float[]> right_temp = std::move(sample->right);

    sample->init(new_length);

    // perform the crop
    std::memcpy(sample->left.get(), left_temp.get() + from, new_length * sizeof(float));
    std::memcpy(sample->right.get(), right_temp.get() + from, new_length * sizeof(float));

    // snap!
    sample->edit_history.push(std::move(snap));
    return EditResult::OK;
}

EditResult SampleEditor::reverse(int sample_id, Sample* sample, uint32_t from, uint32_t to)
{
    if (!sample || !sample->left || !sample->right) return EditResult::INVALID_SAMPLE;

    if (from < 0 || to >= sample->length) return EditResult::OUT_OF_BOUNDS;

    if (from == to || from > to || to - from > sample->length) return EditResult::INVALID_RANGE;

    // snapshot the original region
    uint32_t rev_tween = to - from;

    SampleSnapshot snap;
    
    std::unique_ptr<float[]> snap_left = std::make_unique<float[]>(sample->length);
    std::unique_ptr<float[]> snap_right = std::make_unique<float[]>(sample->length);

    std::memcpy(snap_left.get(), sample->left.get() + from, sample->length * sizeof(float));
    std::memcpy(snap_right.get(), sample->right.get() + from, sample->length * sizeof(float));

    snap.from = from;
    snap.to = to;
    snap.length = sample->length;
    snap.op = SampleOperation::REVERSE;
    snap.left = std::move(snap_left); 
    snap.right = std::move(snap_right);
    
    // perform the reverse
    for (int i = 0; i < rev_tween/2; i++)
    {
        std::swap(sample->left[i + from], sample->left[to - i - 1]);
        std::swap(sample->right[i + from], sample->right[to - i - 1]);
    }

    // snap!
    sample->edit_history.push(std::move(snap));

    return EditResult::OK;
}

EditResult SampleEditor::normalise(int sample_id, Sample* sample, uint32_t from, uint32_t to)
{
    return EditResult::OK;
}

EditResult SampleEditor::undo(int sample_id, Sample* sample)
{
    return EditResult::OK;
}

EditResult SampleEditor::redo(int sample_id, Sample* sample)
{
    return EditResult::OK;
}