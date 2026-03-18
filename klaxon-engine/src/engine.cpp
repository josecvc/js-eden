#include "engine.h"
#include "pattern.h"

void Engine::init(int sample_rate, int bpm, int ticks_per_row)
{
    this->sample_rate = sample_rate;
    this->bpm = bpm;
    this->ticks_per_row = ticks_per_row;
        
    this->samples_per_tick = static_cast<float>(sample_rate) * 60.f  / (bpm * 24); // 24 ticks per minute tempo base

    this->current_samples = 0;
    this->current_ticks = 0;
    this->current_row = 0;
    this->current_pattern = 0;
    this->current_order = 0;
}

Sample* Engine::get_sample(Instrument& inst, int note_id)
{
    if (note_id < 0 || note_id > 119) return nullptr;

    if (inst.type != InstrumentType::SAMPLE) return nullptr;

    int sample_id = inst.sample.note_sample[note_id];

    return sample_pool[sample_id].get();
}

int Engine::process(float** output, int frames)
{
    clear(output, frames);

    int is_hit = -1; // assume no playback

    if(is_playing) {
        is_hit = step(frames);
    }

    mix_instruments(output, frames);
    poly.dump_playheads(sample_playheads, envelope_playheads, current_instrument, current_sample);
    return is_hit;
}

void Engine::mix_instruments(float** output, int frames)
{
    // render each voice that is active

    if(poly.get_current_voices() == 0 || instrument_count == 0) return; // edge case, without this you get null calls

    for(int v = 0; v < poly.MAX_VOICES; v++) 
    {
        if (poly.voices[v].finished)
            poly.remove_voice(poly.voices[v].channel_id, poly.voices[v].note_id, poly.voices[v].instrument_id);
        if(!poly.voices[v].active) continue;
    }

    poly.render_voices(output, frames);
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

    current_samples += frames;

    while (current_samples >= samples_per_tick)
    {
        current_samples -= samples_per_tick;
        advance_tick();
        is_hit++;
    }

    return is_hit;
}

void Engine::advance_tick()
{
    current_ticks++;

    int row_tick = current_ticks % ticks_per_row;

    if (row_tick > 0) // run effects after a row is fired (not on, row_tick = 0 is only for triggering notes)
    {
        process_effects(row_tick);
    }

    if (row_tick == 0) {
        advance_row();
    }

    poly.advance_env_tick();
}

void Engine::process_row()
{
    // find which row to process based on current_row, current_pattern
    auto& pat = pattern_info.patterns[current_pattern];

    int rowStart = current_row * pattern_info.num_channels;

    for (int ch = 0; ch < pattern_info.num_channels; ch++)
    {
        int idx = rowStart + ch;
        Cell& cell = pat.cells[idx];

        if(cell.instrumentId > instrument_count || cell.noteId < 12 || cell.noteId > 119 || cell.instrumentId < 1) continue;

        // channels are 1-indexed (MIDI is taking channel 0)
        // instruments are 1-indexed (0 means no instrument in cell)

        auto& inst = instruments[cell.instrumentId - 1];
        if(inst.type == InstrumentType::NONE) continue;

        Sample* smp = get_sample(inst, cell.noteId);

        int sample_id;

        if (!smp)
            sample_id = -1;
        else {
            sample_id = inst.sample.note_sample[cell.noteId];
        }

        poly.add_voice(&inst, smp, ch + 1, cell.noteId, cell.instrumentId - 1, sample_id, cell.volume, 72); // "72" needs to be changed afterwards
    }
}

void Engine::advance_row()
{   
    process_row();
    current_row++;
    if (current_row >= this->pattern_info.patterns[this->current_pattern].num_rows) { // 0 -> (current num_rows - 1)
        current_row = 0;
    
        if(++current_order >= pattern_info.num_orders) {
            is_playing = false;
            current_order = 0;
            current_samples = 0;
        }
        
        current_pattern = pattern_info.pattern_order[current_order];
    }
}

void Engine::process_effects(int row_tick)
{
    // do any command effects in here (probably with a switch case directing to effect methods)
    return;
}

void Engine::play(int order_num, int row_num)
{   
    is_playing = true;

    current_order = order_num;
    current_row = row_num;
    current_pattern = pattern_info.pattern_order[0];
}

void Engine::pause()
{
    is_playing = false;
}

void Engine::stop()
{
    current_samples = 0;
    current_row = 0;
    current_pattern = 0;
    current_order = 0;
    current_ticks = 0;

    for(int i=0; i < Polyphony::MAX_VOICES; i++) {
        poly.remove_voice(poly.voices[i].channel_id, poly.voices[i].note_id, poly.voices[i].instrument_id);
    }

    is_playing = false;
}

void Engine::set_bpm(int bpm)
{
    this->bpm = bpm;
    this->samples_per_tick = static_cast<float>(sample_rate) * 60.0f  / 24 / bpm;
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

int Engine::register_sample(const char* filename, float* left, float* right, int sample_rate, unsigned long length)
{
    auto sample = std::make_unique<Sample>();
    sample->load_sample(filename, left, right, sample_rate, length);

    sample_pool.push_back(std::move(sample));

    int sample_id = sample_pool.size() - 1;

    int id = 0;

    for (int i = 0; i < MAX_INSTRUMENTS; i++)
    {
        if (instruments[i].type == InstrumentType::NONE)
        {
            instruments[i].sample = SampleInstrument();
            instruments[i].type = InstrumentType::SAMPLE;
            instrument_count++;
            id = i;
            break;
        }
    }

    for (int n = 0; n < SampleInstrument::MAX_NOTES; n++)
    {
        instruments[id].sample.note_sample[n] = sample_id;
    }

    return id;
}

int Engine::register_synth()
{
    int id = 0;

    for (int i = 0; i < MAX_INSTRUMENTS; i++)
    {
        if (instruments[i].type == InstrumentType::NONE)
        {
            instruments[i].synth = SynthInstrument();
            instruments[i].type = InstrumentType::SYNTH;
            instrument_count++;
            id = i;
            break;
        }
    }

    return id;
}

void Engine::remove_instrument(int instrument_id)
{
    if(instrument_id < 0 || instrument_id >= MAX_INSTRUMENTS)
        return;

    instruments[instrument_id].type = InstrumentType::NONE;

    instrument_count--;
}

int Engine::switch_instrument_type(int instrument_id, InstrumentType new_type)
{
    if (instrument_id < 0 || instrument_id >= MAX_INSTRUMENTS) return -1;

    auto& inst = instruments[instrument_id];

    if (inst.type == new_type) return instrument_id;

    if (new_type == InstrumentType::SAMPLE || new_type == InstrumentType::SYNTH)
    {
        inst.type = new_type;
    }

    for (Voice& voice : poly.voices)
    {
        if (voice.active && voice.instrument_id == instrument_id)
            voice.finished = true;
    }

    return instrument_id;
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

    void init_engine(Engine* engine, int sample_rate, int bpm, int ticks_per_row)
    {
        if(!engine) return;
        engine->init(sample_rate, bpm, ticks_per_row);
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

    void set_tempo(Engine* engine, int bpm)
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

    int register_sample(Engine* engine, const char* filename, float* left, float* right, unsigned long length, int sample_rate)
    {
        if (!engine) return -1;
        return engine->register_sample(filename, left, right, sample_rate, length);
    }

    void remove_instrument(Engine* engine, int instrument_id)
    {
        if (!engine) return;
        engine->remove_instrument(instrument_id);
    }

    int play_from_midi(Engine* engine, int instrument_id, int note_id)
    {
        if (!engine) return -2;
        if(instrument_id > engine->instrument_count || instrument_id < 1) return -1;

        auto& inst = engine->instruments[instrument_id - 1];
        if(inst.type == InstrumentType::NONE) return -1;

        Sample* smp = engine->get_sample(inst, note_id);

        int sample_id;

        if (!smp)
            sample_id = -1;
        else {
            sample_id = inst.sample.note_sample[note_id];
        }

        engine->poly.add_voice(&inst, smp, 0, note_id, instrument_id - 1, sample_id, 100.f, 72);

        return 0;
    }

    int stop_from_midi(Engine* engine, int instrument_id, int note_id)
    {
        if (!engine) return -2;
        if(instrument_id > engine->instrument_count || instrument_id < 1) return -1;
        engine->poly.remove_voice(0, note_id, instrument_id - 1);

        return 0;
    }

    int get_num_voices(Engine* engine)
    {
        if (!engine) return -2;

        return engine->poly.get_current_voices();
    }
    
    int get_instrument_id_from_channel(Engine* engine, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(row_id < 0 || row_id >= engine->pattern_info.patterns[pattern_id].num_rows) return -1;
        if(channel_id < 0 || channel_id >= engine->pattern_info.num_channels) return -1;

        return engine->pattern_info.patterns[pattern_id].cells[
             row_id * engine->pattern_info.num_channels + channel_id
        ].instrumentId;
    }

    int get_note_id_from_channel(Engine* engine, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(row_id < 0 || row_id >= engine->pattern_info.patterns[pattern_id].num_rows) return -1;
        if(channel_id < 0 || channel_id >= engine->pattern_info.num_channels) return -1;

         return engine->pattern_info.patterns[pattern_id].cells[
             row_id * engine->pattern_info.num_channels + channel_id
        ].noteId;
    }
    
    int get_current_row(Engine* engine)
    {
        if (!engine) return -2;
        return engine->current_row;
    }

    int get_current_pattern(Engine* engine)
    {
        if (!engine) return -2;
        return engine->current_pattern;
    }

    int get_current_order(Engine* engine)
    {
        if (!engine) return -2;
        return engine->current_order;
    }

    int get_playback_state(Engine* engine) 
    {
        if (!engine) return -2;
        return static_cast<int>(engine->is_playing);
    }

    int get_first_cell(Engine* engine)
    {
        if (!engine) return -2;

        return engine->pattern_info.patterns[0].cells[0].noteId;
    }

    int set_note(Engine* engine, int noteId, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;
        
        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(row_id < 0 || row_id >= engine->pattern_info.patterns[pattern_id].num_rows) return -1;
        if(channel_id < 0 || channel_id >= engine->pattern_info.num_channels) return -1;
        
        engine->pattern_info.set_note(pattern_id, channel_id, row_id, noteId);

        return 0;
    }

    int set_instrument(Engine* engine, int instrument_id, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(row_id < 0 || row_id >= engine->pattern_info.patterns[pattern_id].num_rows) return -1;
        if(channel_id < 0 || channel_id >= engine->pattern_info.num_channels) return -1;

        engine->pattern_info.set_instrument(pattern_id, channel_id, row_id, instrument_id);

        return 0;
    }

    int set_volume(Engine* engine, int volume, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(row_id < 0 || row_id >= engine->pattern_info.patterns[pattern_id].num_rows) return -1;
        if(channel_id < 0 || channel_id >= engine->pattern_info.num_channels) return -1;

        engine->pattern_info.set_volume(pattern_id, channel_id, row_id, volume);

        return 0;
    }

    int set_effect(Engine* engine, int effect, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(row_id < 0 || row_id >= engine->pattern_info.patterns[pattern_id].num_rows) return -1;
        if(channel_id < 0 || channel_id >= engine->pattern_info.num_channels) return -1;

        engine->pattern_info.set_effect(pattern_id, channel_id, row_id, effect);

        return 0;
    }

    int set_param(Engine* engine, int param, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(row_id < 0 || row_id >= engine->pattern_info.patterns[pattern_id].num_rows) return -1;
        if(channel_id < 0 || channel_id >= engine->pattern_info.num_channels) return -1;

        engine->pattern_info.set_param(pattern_id, channel_id, row_id, param);

        return 0;
    }

    int set_instrument_sample(Engine* engine, int instrument_id)
    {
        if (!engine) return -2;
        if (instrument_id < 0 || instrument_id >= Engine::MAX_INSTRUMENTS) return -1;

        
        return engine->switch_instrument_type(instrument_id, InstrumentType::SAMPLE);
    }

    int set_instrument_synth(Engine* engine, int instrument_id)
    {
        if (!engine) return -2;
        if (instrument_id < 0 || instrument_id >= Engine::MAX_INSTRUMENTS) return -1;

        
        return engine->switch_instrument_type(instrument_id, InstrumentType::SYNTH);
    }

    int insert_order(Engine* engine, int position, int pattern_id)
    {
        if (!engine) return -2;

        if (position < 0 || position >= MAX_ORDER || pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;

        return engine->pattern_info.insert_order(position, pattern_id);
    }

    int remove_order(Engine* engine, int position)
    {
        if (!engine) return -2;

        if (position < 0 || position >= MAX_ORDER) return -1;

        return engine->pattern_info.remove_order(position);
    }

    int increase_channel_count(Engine* engine)
    {
        if (!engine) return -2;

        if (engine->pattern_info.num_channels == MAX_CHANNELS) return -1;

        return engine->pattern_info.resize_channel_count_by_two(true);
    }

    int decrease_channel_count(Engine* engine)
    {
        if (!engine) return -2;

        if (engine->pattern_info.num_channels == 2) return -1;

        return engine->pattern_info.resize_channel_count_by_two(false);
    }

    int increase_row_count(Engine* engine, int pattern_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(engine->pattern_info.patterns[pattern_id].num_rows == 256) return -1;

        return engine->pattern_info.resize_row_by_one(pattern_id, true);
    }

    int decrease_row_count(Engine* engine, int pattern_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(engine->pattern_info.patterns[pattern_id].num_rows == 1) return -1;

        return engine->pattern_info.resize_row_by_one(pattern_id, false);
    }

    int expand_pattern(Engine* engine, int pattern_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(engine->pattern_info.patterns[pattern_id].num_rows * 2 > MAX_PATTERNS) return -1;

        return engine->pattern_info.expand_pattern(pattern_id);
    }

    int shrink_pattern(Engine* engine, int pattern_id)
    {
        if (!engine) return -2;

        if(pattern_id < 0 || pattern_id >= MAX_PATTERNS) return -1;
        if(engine->pattern_info.patterns[pattern_id].num_rows / 2 < 1) return -1;

        return engine->pattern_info.shrink_pattern(pattern_id);
    }

    int* get_sample_playhead_ptr(Engine* engine)
    {
        if (!engine) return nullptr;

        return engine->sample_playheads;
    }

    int* get_envelope_playhead_ptr(Engine* engine)
    {
        if (!engine) return nullptr;

        return engine->envelope_playheads;
    }

    int get_bpm(Engine* engine) {
        if (!engine) return -2;

        return engine->bpm;
    }

    int set_current_view(Engine* engine, int sample_id, int instrument_id)
    {
        if (!engine) return -2;

        engine->current_sample = sample_id;
        engine->current_instrument = instrument_id;
    }

    //TODO: Finish WebAssembly functions
}