#ifndef PATTERN_H
#define PATTERN_H

#include <cstdint>

constexpr int NUM_ROWS = 64;
constexpr int MAX_ROWS = 256;

constexpr int NUM_CHANNELS = 8;
constexpr int MAX_CHANNELS = 64;

constexpr int MAX_PATTERNS = 255;
constexpr int MAX_ORDER = 127;

struct Cell 
{
    uint8_t noteId;
    uint8_t instrumentId;
    uint8_t volume;
    uint8_t effect;
    uint8_t param;
};

struct ChannelData 
{
    uint8_t last_command;
};

class Pattern 
{
public:
    Pattern() : cells(nullptr), num_rows(0) {}
    ~Pattern() { delete[] cells; };

    int init();

    Cell* cells;
    int num_rows;
};

class PatternInfo 
{
public:
    PatternInfo();

    // global data mutation
    int insert_order(int position, int pattern_id);
    int remove_order(int position);
    int resize_channel_count_by_two(bool up_down);

    // local data mutation
    int set_note(int pattern_id, int channel_id, int row_id, int note);
    int set_instrument(int pattern_id, int channel_id, int row_id, int instrument);
    int set_volume(int pattern_id, int channel_id, int row_id, int volume);
    int set_effect(int pattern_id, int channel_id, int row_id, int effect);
    int set_param(int pattern_id, int channel_id, int row_id, int param);

    // pattern mutation
    int resize_row_by_one(int pattern_id, bool up_down);
    int expand_pattern(int pattern_id);
    int shrink_pattern(int pattern_id);

    // helpers
    Cell* get_cell(int pattern_id, int row_id, int channel_id);
    int clear_cell(int pattern_id, int row_id, int channel_id);

    // pattern information
    Pattern patterns[MAX_PATTERNS];
    uint8_t  pattern_order[MAX_ORDER];

    int num_channels;
    int num_orders; 
};

#endif