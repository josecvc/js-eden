#ifndef INSTRUMENT_H
#define INSTRUMENT_H

#include "sample.h"
#include "wavetable.h"

constexpr int MAX_POINTS = 8;

enum class InstrumentType : int
{
    SAMPLE,
    SYNTH,
    NONE
};

struct Point
{
    bool active{false};
    uint8_t tick{0};
    uint8_t vol{0};
};

class Envelope 
{
public:
    Envelope()
    {
        points[0].active = true;
        points[0].tick = 0;
        points[0].vol = 100;
        
        points[1].active = true;
        points[1].tick = 16;
        points[1].vol = 100;
    }

    int add_point(int pos);
    int delete_point(int point_id);

    int sustain_at{0};
    int loop_from{0};
    int loop_to{2};
    int fadeout{200};

    bool enabled{false};
    bool sustain{false};
    bool loop{false};
    
    int count{2};
    Point points[MAX_POINTS];
};

struct SampleInstrument
{
    static constexpr int MAX_NOTES = 128;
    int note_sample[MAX_NOTES];  // note -> sample_id (0->119)
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