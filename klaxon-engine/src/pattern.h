#ifndef PATTERN_H
#define PATTERN_H

//TODO: Patterns, rows, triggers. Create and test a basic pattern with a preloaded sample, with play and stop functionality.

constexpr int MAX_ROWS = 64;
constexpr int MAX_CHANNELS = 8;

struct Cell {
    uint8_t noteId;
    uint8_t instrumentId;
    uint8_t volume;
    uint8_t effect;
    uint8_t param;
};

struct PatternInfo {
    Cell* cells;

    uint16_t* pattern_rows;
    uint32_t* pattern_offset;
    uint8_t*  pattern_order;

    int num_patterns;
    int num_channels;
    int num_cells;
    int num_orders; 
};

#endif