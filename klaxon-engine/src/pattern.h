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
    Cell rows[MAX_ROWS * MAX_CHANNELS];
};

struct Order {
    unsigned short num_patterns;
    unsigned short sequence_length;

    Pattern* patterns;
    unsigned short* sequence;
};

#endif