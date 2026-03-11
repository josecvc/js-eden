#include "engine.h"
#include "pattern.h"

void Engine::init(
        int sample_rate, 
        int bpm, 
        int ticks_per_row
    )
{
    this->sample_rate = sample_rate;
    this->bpm = bpm;
    this->ticks_per_row = ticks_per_row;
    this->ticks_per_row = ticks_per_row;
        
    this->samples_per_tick = static_cast<float>(sample_rate) * 60.f  / (bpm * 24); // 24 ticks per minute tempo base

    this->pattern_info.cells = reinterpret_cast<Cell*>(new uint8_t[MAX_PATTERNS * MAX_ROWS * MAX_CHANNELS * 5]);

    for(int i = 0; i < MAX_ORDER;i++)
    {
        this->pattern_info.pattern_order[i] = 0;
    }

    for(int i = 0; i < MAX_PATTERNS; i++)
    {
        this->pattern_info.pattern_rows[i] = 64;
    }

    int offset = 0;

    for(int i = 0; i < MAX_PATTERNS; i++) 
    {
        this->pattern_info.pattern_offset[i] = offset;
        offset = this->pattern_info.pattern_offset[i] + this->pattern_info.pattern_rows[i];
    }

    this->pattern_info.num_cells = MAX_PATTERNS * MAX_ROWS * MAX_CHANNELS * 5;
    this->pattern_info.num_patterns = MAX_PATTERNS;
    this->pattern_info.num_orders = 1;
    this->pattern_info.num_channels = MAX_CHANNELS;

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
    }

    mix_instruments(output, frames);
    
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
        
        std::visit([&](auto& i) {
            using T = std::decay_t<decltype(i)>;

            if constexpr (std::is_same_v<T, SampleInstrument>) {
                int note_id = poly.voices[v].note_id;
                int sample_id = i.note_sample[note_id];

                if (sample_id < 0 || sample_id >= sample_pool.size()) return;

                const auto& sample = sample_pool[sample_id].get();
                i.render(output, frames, poly.voices[v], sample);
            } else if constexpr (std::is_same_v<T, SynthInstrument>) {
                // Synth generates its own waveform
                i.render(output, frames, poly.voices[v]);
            }
        }, instruments[poly.voices[v].instrument_id]);
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

    // do any command effects in here
}

void Engine::process_row()
{
    // find which row to process based on current_row, current_pattern
    int pat = pattern_info.pattern_offset[current_pattern];

    int rowStart = pat + current_row * pattern_info.num_channels;

    for (int ch = 0; ch < pattern_info.num_channels; ch++)
    {
        // auto& cell = ptrn.rows[current_row][ch];
        int idx = rowStart + ch;
        Cell& cell = pattern_info.cells[idx];

        if(cell.instrumentId > instrument_count || cell.noteId < 12 || cell.noteId > 119 || cell.instrumentId < 1) continue;

        // channels are 1-indexed (MIDI is taking channel 0)
        // instruments are 1-indexed (0 means no instrument in cell)
        poly.add_voice(ch + 1, cell.noteId, cell.instrumentId - 1, cell.volume, 72); // "72" needs to be changed afterwards
    }
}

void Engine::advance_row()
{   
    process_row();
    current_row++;
    if (current_row >= MAX_ROWS) { // 0 -> (MAX_ROWS - 1)
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
    return;
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
    auto sample = std::make_shared<Sample>();
    sample->load_sample(filename, left, right, sample_rate, length);

    sample_pool.push_back(sample);

    int sample_id = sample_pool.size() - 1;

    int id = 0;

    for (int i = 0; i < MAX_INSTRUMENTS; i++)
    {
        if (std::holds_alternative<std::monostate>(instruments[i]))
        {
            instruments[i] = SampleInstrument();
            instrument_count++;
            id = i;
            break;
        }
    }

    auto& inst = instruments[id];

    if (auto sample_inst = std::get_if<SampleInstrument>(&inst))
    {
        for (int n = 0; n < SampleInstrument::MAX_NOTES; n++)
        {
            sample_inst->note_sample[n] = sample_id;
        }
    }

    return id;
}

void Engine::remove_instrument(int instrument_id)
{
    if(instrument_id < 0 && instrument_id >= MAX_INSTRUMENTS)
        return;

    auto& inst = instruments[instrument_id];

    if (auto sampleInst = std::get_if<SampleInstrument>(&inst))
    {
        for (int note = 0; note < SampleInstrument::MAX_NOTES; note++)
        {
            if (sampleInst->note_sample[note] == instrument_id)
            {
                sampleInst->note_sample[note] = -1;
            }
        }
    }

    inst = std::monostate{};
    instrument_count--;
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
        engine->poly.add_voice(0, note_id, instrument_id - 1, 1.0f, 72);

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

    // DEBUG FUNCTIONS
    int get_instrument_id_from_channel(Engine* engine, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;
        if(pattern_id < 0 || pattern_id >= engine->pattern_info.num_patterns) return -1;
         return engine->pattern_info.cells[
            engine->pattern_info.pattern_offset[pattern_id] + row_id * engine->pattern_info.num_channels + channel_id
        ].instrumentId;
    }

    int get_note_id_from_channel(Engine* engine, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;
        if(pattern_id < 0 || pattern_id >= engine->pattern_info.num_patterns) return -1;


        return engine->pattern_info.cells[
            engine->pattern_info.pattern_offset[pattern_id] + row_id * engine->pattern_info.num_channels + channel_id].noteId;
    }

    int get_num_patterns(Engine* engine)
    {
        if (!engine) return -2;

        return engine->pattern_info.num_patterns;
    }

    int get_pattern_offset(Engine* engine, int pattern_id)
    {
        if (!engine) return -2;
        if(pattern_id < 0 || pattern_id >= engine->pattern_info.num_patterns) return -1;


        return engine->pattern_info.pattern_offset[pattern_id];
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

        return engine->pattern_info.cells[0].instrumentId;
    }

    int set_note(Engine* engine, int noteId, int index)
    {
        if (!engine) return -2;

        engine->pattern_info.cells[index].noteId = static_cast<uint8_t>(noteId);

        return 0;
    }

    int set_instrument(Engine* engine, int instrumentId, int index)
    {
        if (!engine) return -2;

        engine->pattern_info.cells[index].instrumentId = static_cast<uint8_t>(instrumentId);

        return 0;
    }

    int set_volume(Engine* engine, int volume, int index)
    {
        if (!engine) return -2;

        engine->pattern_info.cells[index].volume = static_cast<uint8_t>(volume);

        return 0;
    }

    int set_effect(Engine* engine, int effect, int index)
    {
        if (!engine) return -2;

        engine->pattern_info.cells[index].effect = static_cast<uint8_t>(effect);

        return 0;
    }

    int set_param(Engine* engine, int param, int index)
    {
        if (!engine) return -2;

        engine->pattern_info.cells[index].param = static_cast<uint8_t>(param);

        return 0;
    }

    int get_bpm(Engine* engine) {
        if (!engine) return -2;

        return engine->bpm;
    }

    //TODO: Finish WebAssembly functions
}

// int Engine::step(int frames)
// {
//     int is_hit = 0;

//     int start_sample = current_samples;
//     int end_sample = current_samples + frames;

//     int start_tick = start_sample / samples_per_tick;
//     int end_tick = (end_sample - 1) / samples_per_tick;

//     if (end_tick / ticks_per_row > start_tick / ticks_per_row)
//     {
//         is_hit = 1;
//         advance_row();
        
//     }

//     current_samples = end_sample;
//     current_ticks = end_tick;

//     return is_hit;
// }
