#include "engine.h"

void Engine::init(int sample_rate, 
        int bpm, 
        int ticks_per_row,
        uint8_t* noteIds,
        uint8_t* instrumentIds,
        uint8_t* volume,
        uint8_t* effectIds,
        uint8_t* params,
        uint16_t* pattern_rows,
        uint32_t* pattern_offset,
        uint8_t* pattern_order,
        int num_patterns,
        int num_channels,
        int num_cells,
        int num_orders
    )
{
    this->sample_rate = sample_rate;
    this->bpm = bpm;
    this->ticks_per_row = ticks_per_row;
    this->ticks_per_row = ticks_per_row;
        
    this->samples_per_tick = static_cast<float>(sample_rate) * 60.f  / (bpm * 24); // 24 ticks per minute tempo base

    // Init pattern pointers
    this->pattern_info.noteIds = noteIds;
    this->pattern_info.instrumentIds = instrumentIds;
    this->pattern_info.volume = volume;
    this->pattern_info.effectIds = effectIds;
    this->pattern_info.params = params;
    this->pattern_info.pattern_rows = pattern_rows;
    this->pattern_info.pattern_offset = pattern_offset;
    this-> pattern_info.pattern_order = pattern_order;
    this->pattern_info.num_orders = num_orders;

    this->current_samples = 0;
    this->current_ticks = 0;
    this->current_row = 0;
    this->current_pattern = 0;
    this->current_order = 0;
}

int Engine::process(float** output, int frames)
{
    clear(output, frames);

    int is_hit = -1; // assume no playback

    if(is_playing) {
        is_hit = step(frames);

        if(is_hit && current_ticks % ticks_per_row == 0) { // only process if the step encounters a hit and we have hit ticks_per_row ticks
            process_row();
        }
    }

    mix_instruments(output, frames);
    
    return is_hit;
}

void Engine::mix_instruments(float** output, int frames)
{
    // render each voice that is active

    if(poly.get_current_voices() == 0 || instruments.size() == 0) return; // edge case, without this you get null calls

    for(int v = 0; v < poly.MAX_VOICES; v++) {
        if (poly.voices[v].finished)
            poly.remove_voice(poly.voices[v].channel_id, poly.voices[v].note_id, poly.voices[v].instrument_id);
        if(!poly.voices[v].active) continue;
        
        instruments[poly.voices[v].instrument_id]->render(output, frames, poly.voices[v]);
    }
}

void Engine::clear(float** output, int frames)
{
    for (int i = 0; i < frames; i++) // clear buffer
    {
        output[0][i] = 0;
        output[1][i] = 0;
    }
}

int Engine::step(int frames)
{
    int is_hit = 0;

    int start_sample = current_samples;
    int end_sample = current_samples + frames;

    int start_tick = start_sample / samples_per_tick;
    int end_tick = (end_sample - 1) / samples_per_tick;

    if (end_tick / ticks_per_row > start_tick / ticks_per_row)
    {
        is_hit = 1;
        advance_row();
        
    }

    current_samples = end_sample;
    current_ticks = end_tick;

    return is_hit;
}

void Engine::process_row()
{
    // find which row to process based on current_row, current_pattern

    int pat = pattern_info.pattern_offset[current_pattern];

    for (int ch = 0; ch < pattern_info.num_channels; ch++)
    {
        // auto& cell = ptrn.rows[current_row][ch];
        int cell = pat + current_row*pattern_info.num_channels + ch;
        auto& noteId = pattern_info.noteIds[cell];
        auto& instrumentId = pattern_info.instrumentIds[cell];
        auto& volume = pattern_info.volume[cell];
        auto& effectId = pattern_info.effectIds[cell];
        auto& param = pattern_info.params[cell];

        if(instrumentId > instruments.size() || noteId < 12 || noteId > 119 || instrumentId < 1) continue;

        // channels are 1-indexed (MIDI is taking channel 0)
        // instruments are 1-indexed (0 means no instrument in cell)
        poly.add_voice(ch + 1, noteId, instrumentId - 1, volume, 72); // "72" needs to be changed afterwards
    }
}

void Engine::advance_row()
{   
    current_row++;
    if (current_row > MAX_ROWS - 1) { // 0 -> (MAX_ROWS - 1)
        current_row = 0;

        if(++current_order >= pattern_info.num_orders) {
            is_playing = false;
            current_order = 0;
            current_samples = 0;
            current_pattern = pattern_info.pattern_order[0];
            return;
        }
        
        current_pattern = pattern_info.pattern_order[0];
    }
}

void Engine::play(int order_num, int row_num)
{   
    int apparent_samples = samples_per_tick * ticks_per_row * row_num + MAX_ROWS * samples_per_tick * ticks_per_row * order_num;
    if (apparent_samples != current_samples) 
        current_samples = apparent_samples;

    is_playing = true;

    current_order = order_num;
    current_row = row_num;
    current_pattern = pattern_info.pattern_order[0];

    process_row();
}

void Engine::pause()
{
    is_playing = false;
}

void Engine::stop()
{
    current_samples = 0;
    is_playing = false;
}

void Engine::set_bpm(int bpm)
{
    this->bpm = bpm;
    this->samples_per_tick = static_cast<float>(sample_rate) * 60.0  / 24 / bpm;
}

void Engine::set_sample_rate(int sample_rate)
{
    this->sample_rate = sample_rate;
    this->samples_per_tick = static_cast<float>(sample_rate) * 60 / 24 / bpm ;
}

void Engine::set_ticks_per_row(int ticks_per_row)
{
    this->ticks_per_row = ticks_per_row;
    this->samples_per_tick = static_cast<float>(sample_rate) * 60.0  / 24 / bpm;
}

void Engine::add_synth()
{
    std::unique_ptr<Instrument> instrument = std::make_unique<SynthInstrument>(SynthInstrument());

    instruments.push_back(std::move(instrument));
}

void Engine::add_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length)
{
    std::unique_ptr<Instrument> instrument = std::make_unique<SampleInstrument>(SampleInstrument(filename, left, right, sample_rate, length));

    instruments.push_back(std::move(instrument));
}

void Engine::remove_instrument(int instrument_id)
{
    if(instrument_id >= 0 && instrument_id < instruments.size())
        instruments.erase(instruments.begin() + instrument_id);
}

/**
 * Exported WebAssembly Functions (put everything in a main file when finished)
 */
extern "C"
{
    Engine* create_engine()
    {
        return new Engine();
    }

    void init_engine(
        Engine* engine, 
        int sample_rate, 
        int bpm, 
        int ticks_per_row,
        uint8_t* noteIds,
        uint8_t* instrumentIds,
        uint8_t* volume,
        uint8_t* effectIds,
        uint8_t* params,
        uint16_t* pattern_rows,
        uint32_t* pattern_offset,
        uint8_t* pattern_order,
        int num_patterns,
        int num_channels,
        int num_cells,
        int num_orders
    )
    {
        if(!engine) return;
        engine->init(sample_rate, bpm, ticks_per_row, noteIds, instrumentIds, volume, effectIds, params, pattern_rows, pattern_offset, pattern_order, num_patterns, num_channels, num_cells, num_orders);
    }

    bool destroy_engine(Engine* engine)
    {
        if(!engine) return false;
        delete engine;
        return true;
    }

    int process(Engine* engine, float** output, int frames)
    {
        if(!engine) return -2; // engine doesn't exist
        return engine->process(output, frames);
    }

    void play_track(Engine* engine, int order, int row)
    {
        if (!engine) return;
        engine->play(order, row);
    }

    void pause_track(Engine* engine)
    {
        if (!engine) return;
        engine->pause();
    }

    void stop_track(Engine* engine)
    {
        if (!engine) return;
        engine->stop();
    }

    void set_bpm(Engine* engine, int bpm)
    {
        if (!engine) return;
        engine->set_bpm(bpm);
    }

    void set_sample_rate(Engine* engine, int sample_rate)
    {
        if (!engine) return;
        engine->set_sample_rate(sample_rate);
    }

    void set_ticks_per_row(Engine* engine, int ticks_per_row)
    {
        if (!engine) return;
        engine->set_ticks_per_row(ticks_per_row);
    }

    int add_sample(Engine* engine, const char* filename, float* left, float* right, unsigned long length, int sample_rate)
    {
        if (!engine) return -1;
        engine->add_sample(filename, left, right, sample_rate, length);

        return 0;
    }

    void remove_instrument(Engine* engine, int instrument_id)
    {
        if (!engine) return;
        engine->remove_instrument(instrument_id);
    }

    int play_from_midi(Engine* engine, int instrument_id, int note_id)
    {
        if (!engine) return -2;
        if(instrument_id > engine->instruments.size() || instrument_id < 1) return -1;
        engine->poly.add_voice(0, note_id, instrument_id - 1, 1.0f, 72);

        return 0;
    }

    int stop_from_midi(Engine* engine, int instrument_id, int note_id)
    {
        if (!engine) return -2;
        if(instrument_id > engine->instruments.size() || instrument_id < 1) return -1;
        // engine->poly.add_voice(0, note_id, instrument_id - 1, 1.0f, 72);

        return 0;
    }

    int get_num_voices(Engine* engine)
    {
        if (!engine) return -2;

        return engine->poly.get_current_voices();
    }

    // DEBUG FUNCTIONS
    int get_instrument_id_from_channel(Engine* engine, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;
        if(pattern_id < 0 || pattern_id >= engine->pattern_info.num_patterns) return -1;
         return engine->pattern_info.instrumentIds[
            engine->pattern_info.pattern_offset[pattern_id] + row_id * engine->pattern_info.num_channels + channel_id
        ];
    }

    int get_note_id_from_channel(Engine* engine, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;
        if(pattern_id < 0 || pattern_id >= engine->pattern_info.num_patterns) return -1;
        return engine->pattern_info.noteIds[
            engine->pattern_info.pattern_offset[pattern_id] + row_id * engine->pattern_info.num_channels + channel_id
        ];
    }
    
    int get_current_row(Engine* engine)
    {
        if (!engine) return -2;
        return engine->current_row;
    }

    //TODO: Finish WebAssembly functions
}