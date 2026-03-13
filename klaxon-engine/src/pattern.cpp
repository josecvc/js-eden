#include "pattern.h"
#include <algorithm>
#include <cstring>
#include <cstdint>

int Pattern::init()
{
    delete[] cells;
    num_rows = NUM_ROWS; // init with 64 rows
    cells = new Cell[NUM_CHANNELS * NUM_ROWS];
    std::memset(cells, 0, sizeof(Cell) * NUM_CHANNELS * NUM_ROWS);

    return 0;
}

PatternInfo::PatternInfo()
{
    for(auto& pattern : patterns)
    {
        pattern.init();
    }

    this->pattern_order[0] = 0;

    for(int i = 1; i < MAX_ORDER; i++)
    {
        this->pattern_order[i] = 255;
    }

    this->num_orders = 1;
    this->num_channels = NUM_CHANNELS;
}

Cell* PatternInfo::get_cell(int pattern_id, int row_id, int channel_id)
{
    if (pattern_id >= MAX_PATTERNS || pattern_id < 0 || channel_id >= this->num_channels || channel_id < 0) return nullptr;

    auto& pattern = this->patterns[pattern_id];

    if (row_id >= pattern.num_rows || row_id < 0) return nullptr;

    return &pattern.cells[row_id * this->num_channels + channel_id];
}

int PatternInfo::clear_cell(int pattern_id , int row_id, int channel_id)
{
    Cell* cell = get_cell(pattern_id, row_id, channel_id);

    if (!cell) return -1;

    cell->noteId = 0;
    cell->instrumentId = 0;
    cell->volume = 0;
    cell->effect = 0;
    cell->param = 0;

    return 0;
}

int PatternInfo::set_note(int pattern_id, int channel_id, int row_id, int note)
{
    Cell* cell = get_cell(pattern_id, row_id, channel_id);

    if(!cell) return -1;

    cell->noteId = static_cast<uint8_t>(note);

    return 0;
}

int PatternInfo::set_instrument(int pattern_id, int channel_id, int row_id, int instrument)
{
    Cell* cell = get_cell(pattern_id, row_id, channel_id);

    if(!cell) return -1;

    cell->instrumentId = static_cast<uint8_t>(instrument);

    return 0;
}

int PatternInfo::set_volume(int pattern_id, int channel_id, int row_id, int volume)
{
    Cell* cell = get_cell(pattern_id, row_id, channel_id);

    if(!cell) return -1;

    cell->volume = static_cast<uint8_t>(volume);

    return 0;
}

int PatternInfo::set_effect(int pattern_id, int channel_id, int row_id, int effect)
{
    Cell* cell = get_cell(pattern_id, row_id, channel_id);

    if(!cell) return -1;

    cell->effect = static_cast<uint8_t>(effect);

    return 0;
}

int PatternInfo::set_param(int pattern_id, int channel_id, int row_id, int param)
{
    Cell* cell = get_cell(pattern_id, row_id, channel_id);

    if(!cell) return -1;

    cell->param = static_cast<uint8_t>(param);

    return 0;
}

int PatternInfo::insert_order(int position, int pattern_id)
{
    if (position < 0 || pattern_id < 0 || position >= MAX_ORDER || pattern_id >= MAX_PATTERNS || this->num_orders >= MAX_ORDER) return -1;

    // if we're lucky to have that open at the position we asked for
    if (this->pattern_order[position] == 255)
    {
        this->pattern_order[position] = pattern_id;
        this->num_orders++;
        return 0;
    }

    // make space for the order
    for (int i = num_orders; i > position; i--)
    {  
        // set next to prev
        this->pattern_order[i] = this->pattern_order[i - 1];
    }

    this->pattern_order[position] = pattern_id;
    this->num_orders++;

    return 0;
}

int PatternInfo::remove_order(int position)
{
    if (position < 0 || position >= this->num_orders || this->num_orders <= 0) return -1;

    // shift order list back by one and set last element to 255
    for (int i = position; i < this->num_orders - 1; i++)
    {  
        // set prev to next
        this->pattern_order[i] = this->pattern_order[i + 1];
    }

    this->pattern_order[num_orders - 1] = 255;
    this->num_orders--;

    return 0; 
}

int PatternInfo::resize_channel_count_by_two(bool up_down)
{
    // go through every pattern and copy data

    int old_channel_count = this->num_channels;
    int new_channel_count = up_down ? (old_channel_count + 2) : (old_channel_count - 2);

    if (new_channel_count > MAX_CHANNELS || new_channel_count < 2) return -1;

    int channel_ride = std::min(old_channel_count, new_channel_count);

    for(int p = 0; p < MAX_PATTERNS; p++)
    {
        auto& pattern = this->patterns[p];

        // make a new cell array
        Cell* new_cells = new Cell[new_channel_count * pattern.num_rows];
        std::memset(new_cells, 0, sizeof(Cell) * pattern.num_rows * new_channel_count);

        // TODO: Do memcpy instead
        for(int r = 0; r < pattern.num_rows; r++)
        {
            int start_row_old = r * old_channel_count;
            int start_row_new = r * new_channel_count;

            for(int ch = 0; ch < channel_ride; ch++)
            {
                new_cells[start_row_new + ch] = pattern.cells[start_row_old + ch];
            }
        }

        delete[] pattern.cells;
        pattern.cells = new_cells;
    }

    this->num_channels = new_channel_count;

    return 0;
}

int PatternInfo::resize_row_by_one(int pattern_id, bool up_down)
{
    auto& pattern = this->patterns[pattern_id];

    if ((!up_down && pattern.num_rows <= 1) || (up_down && pattern.num_rows >= MAX_ROWS))
        return -1;
    
    int new_row_count = up_down ? (pattern.num_rows + 1) : (pattern.num_rows - 1);
    
    int row_min = std::min(new_row_count, pattern.num_rows);
    
    Cell* new_cells = new Cell[new_row_count * this->num_channels];
    std::memset(new_cells, 0, sizeof(Cell) * new_row_count * this->num_channels);

    for(int r = 0; r < row_min; r++)
    {
        int start_row = r * this->num_channels;
        for(int ch = 0; ch < this->num_channels; ch++)
        {
            int idx = start_row + ch;
            new_cells[idx] = pattern.cells[idx];
        }
    }

    delete[] pattern.cells;

    pattern.cells = new_cells;
    pattern.num_rows = new_row_count;

    return 0;
}

int PatternInfo::expand_pattern(int pattern_id)
{
    // increase row count by a factor of 2

    auto& pattern = this->patterns[pattern_id];
    int new_row_count = pattern.num_rows * 2;

    if (new_row_count > MAX_ROWS)
        return -1;

    Cell* new_cells = new Cell[new_row_count * this->num_channels];
    std::memset(new_cells, 0, sizeof(Cell) * new_row_count * this->num_channels);

    for(int r = 0; r < pattern.num_rows; r++)
    {
        int start_row = r * this->num_channels;
        int start_row_2 = r * 2 * this->num_channels;
        for(int ch = 0; ch < this->num_channels; ch++)
        {   
            // fill every other row
            int idx = start_row + ch;
            int idx_2 = start_row_2 + ch;
            new_cells[idx_2] = pattern.cells[idx];
        }
    }

    delete[] pattern.cells;

    pattern.cells = new_cells;
    pattern.num_rows = new_row_count;

    return 0;
}

int PatternInfo::shrink_pattern(int pattern_id)
{
    // shrink row count by a factor of 2

    auto& pattern = this->patterns[pattern_id];
    int new_row_count = pattern.num_rows / 2; // integer division

    if (new_row_count <= 0)
        return -1;

    Cell* new_cells = new Cell[new_row_count * this->num_channels];
    std::memset(new_cells, 0, sizeof(Cell) * new_row_count * this->num_channels);

    for(int r = 0; r < pattern.num_rows; r += 2)
    {
        int start_row = r * this->num_channels;
        int start_row_2 = r / 2 * this->num_channels;

        for(int ch = 0; ch < this->num_channels; ch++)
        {   
            // delete every other row
            int idx = start_row + ch;
            int idx_2 = start_row_2 + ch;
            new_cells[idx_2] = pattern.cells[idx];
        }
    }

    delete[] pattern.cells;

    pattern.cells = new_cells;
    pattern.num_rows = new_row_count;

    return 0;
}
