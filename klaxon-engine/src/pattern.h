#ifndef PATTERN_H
#define PATTERN_H

//TODO: Patterns, rows, triggers. Create and test a basic pattern with a preloaded sample, with play and stop functionality.

constexpr int MAX_ROWS = 64;
constexpr int MAX_CHANNELS = 8;
constexpr int MAX_PATTERNS = 255;
constexpr int MAX_ORDER = 127;

struct Pattern {
    Cell* cells;
    int num_rows;
};

struct Cell {
    uint8_t noteId;
    uint8_t instrumentId;
    uint8_t volume;
    uint8_t effect;
    uint8_t param;
};

struct PatternInfo {
    Cell* cells;

    uint16_t pattern_rows[MAX_PATTERNS];
    uint32_t pattern_offset[MAX_PATTERNS];
    uint8_t  pattern_order[MAX_ORDER];

    int num_patterns;
    int num_channels;
    int num_cells;
    int num_orders; 
};

// struct PatternInfo {
//     Pattern patterns[MAX_PATTERNS];

//     uint16_t pattern_rows[MAX_PATTERNS];
//     uint8_t  pattern_order[MAX_ORDER];

//     int num_patterns;
//     int num_channels;
//     int num_cells;
//     int num_orders; 
// };

struct ChannelData {
    uint8_t last_command;
};

#endif