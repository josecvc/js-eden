#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include <variant>
#include "sample.h"
#include "wavetable.h"

constexpr int MAX_POINTS = 12;

enum class InstrumentType : int
{
    SAMPLE,
    SYNTH,
    NONE
};

struct Point
{
    bool active{false};
    uint8_t tick;
    uint8_t vol;
};

class Envelope 
{
public:
    Envelope(){};

    int add_point();
    int delete_point(int point_id);


    int sustain_at;
    int loop_from;
    int loop_to;
    int fadeout;
    bool enabled{false};
    Point points[MAX_POINTS];
};

struct SampleInstrument
{
    static constexpr int MAX_NOTES = 128;
    int note_sample[MAX_NOTES];  // note → sample_id (0->119)
};

struct SynthInstrument
{
    WaveTable wave_table;
    Waveform waveform;
};

struct Instrument
{
    InstrumentType type{InstrumentType::NONE};

    SampleInstrument sample;
    SynthInstrument synth;

    Envelope envelope;
};

#endif 