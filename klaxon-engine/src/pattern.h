#ifndef PATTERN_H
#define PATTERN_H

//TODO: Patterns, rows, triggers. Create and test a basic pattern with a preloaded sample, with play and stop functionality.

struct Row
{

};

struct Trigger
{

};

class Pattern
{
    static constexpr int MAX_ROWS = 64;
    static constexpr int MAX_CHANNELS = 8;
    Row rows[MAX_ROWS * MAX_CHANNELS];
};

#endif