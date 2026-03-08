#ifndef PATTERN_H
#define PATTERN_H

//TODO: Patterns, rows, triggers. Create and test a basic pattern with a preloaded sample, with play and stop functionality.

constexpr int MAX_ROWS = 64;
constexpr int MAX_CHANNELS = 8;

struct Cell {
    char noteId;
    char instrumentId;
    char volume;
    char effectId;
};

struct Pattern
{
    int num_rows;
    int num_channels;
    
    Cell rows[MAX_ROWS][MAX_CHANNELS]; // this needs to change, [MAX_ROWS] can be variable, [MAX_CHANNELS] can be variable
    // Cell** rows; // change to this in the end
};

struct Order {
    unsigned short num_patterns;
    unsigned short sequence_length;

    Pattern* patterns;
    int* sequence;
};


struct PatternInfo {
    uint8_t* noteIds;
    uint8_t* instrumentIds;
    uint8_t* volume;
    uint8_t* effectIds;
    uint8_t* params;

    uint16_t* pattern_rows;
    uint32_t* pattern_offset;
     uint8_t* pattern_order;

    int num_patterns;
    int num_channels;
    int num_cells;
    int num_orders; 
};

#endif