#include "engine.h"

void Engine::init(int sample_rate, int bpm, int rows_per_beat)
{
    this->sample_rate = sample_rate;
    this->bpm = bpm;
    this->rows_per_beat = rows_per_beat;

    this->samples_per_row = static_cast<float>(sample_rate) * 60.f / bpm / rows_per_beat;
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

        if(is_hit) { // only process if the step encounters a hit
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
    int is_hit = 0; // signal for front-end (this could easily be written to a shared buffer)

    // calculate time
    int start_sample = current_samples;
    int end_sample = current_samples + frames;

    int start_row = start_sample / samples_per_row;
    int end_row = (end_sample - 1) / samples_per_row;

    if (end_row > start_row)
    {
        is_hit = 1;
        advance_row();
    }

    current_samples = end_sample;
    
    return is_hit;
}

void Engine::process_row()
{
    // find which row to process based on current_row, current_pattern

    auto& ptrn = order.patterns[current_pattern];

    for (int ch = 0; ch < 1; ch++)
    {
        auto& cell = ptrn.rows[current_row][ch];
        if(cell.instrumentId > instruments.size() || cell.noteId < 12 || cell.noteId > 119 || cell.instrumentId < 1) continue;

        // channels are 1-indexed (MIDI is taking channel 0)
        // instruments are 1-indexed (0 means no instrument in cell)
        poly.add_voice(ch + 1, cell.noteId, cell.instrumentId - 1, cell.volume, 72); // "72" needs to be changed afterwards
    }
}

void Engine::advance_row()
{   
    current_row++;
    if (current_row > MAX_ROWS - 1) { // 0 -> (MAX_ROWS - 1)
        current_row = 0;

        if(++current_order >= order.sequence_length) {
            is_playing = false;
            current_order = 0;
            current_samples = 0;
            current_pattern = order.sequence[0];
            return;
        }
        
        current_pattern = order.sequence[current_order];
    }
}

void Engine::insert_order(int* sequence, int num_indices, int* patterns, int num_patterns)
{   
    // instantiate patterns and sequences

    order.patterns = new Pattern[num_patterns];
    order.sequence = new int[num_indices];

    for(int i = 0; i < num_indices; i++)
    {
        order.sequence[i] = sequence[i];
    }

    // retrieve pattern from MAX_CHANNELS * MAX_ROWS * p + MAX_CHANNELS * r + ch
    for(int p = 0; p < num_patterns; p++)
    {
        Pattern pat;
        // for each pattern fill the grid
        for(int r = 0; r < MAX_ROWS; r++)
        {   
            for (int ch = 0; ch < MAX_CHANNELS; ch++)
            {   int loc = MAX_CHANNELS * MAX_ROWS * p + MAX_CHANNELS * r + ch;

                auto& cell_data = patterns[loc];
                pat.rows[r][ch].noteId =         cell_data        & 0xFF;
                pat.rows[r][ch].instrumentId =  (cell_data >> 8)  & 0xFF;
                pat.rows[r][ch].volume =        (cell_data >> 16) & 0xFF;
                pat.rows[r][ch].effectId =      (cell_data >> 24) & 0xFF;
            }
        }

        order.patterns[p] = pat;
    }

    order.num_patterns = num_patterns;
    order.sequence_length = num_indices;
}

void Engine::play(int order_num, int row_num)
{   
    int apparent_samples = samples_per_row * row_num + MAX_ROWS * samples_per_row * order_num;
    if (apparent_samples != current_samples) 
        current_samples = apparent_samples;

    is_playing = true;

    current_order = order_num;
    current_row = row_num;
    current_pattern = order.sequence[order_num];

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
    this->samples_per_row = static_cast<float>(sample_rate) * 60.0 / bpm / rows_per_beat;
}

void Engine::set_sample_rate(int sample_rate)
{
    this->sample_rate = sample_rate;
    this->samples_per_row = static_cast<float>(sample_rate) * 60 / bpm / rows_per_beat;
}

void Engine::set_rows_per_beat(int rows_per_beat)
{
    this->rows_per_beat = rows_per_beat;
    this->samples_per_row = static_cast<float>(sample_rate) * 60.0 / bpm / rows_per_beat;
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

    void init_engine(Engine* engine, int sample_rate, int bpm, int rows_per_beat)
    {
        if(!engine) return;
        engine->init(sample_rate, bpm, rows_per_beat);
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

    void set_rows_per_beat(Engine* engine, int rows_per_beat)
    {
        if (!engine) return;
        engine->set_rows_per_beat(rows_per_beat);
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

    int read_order(Engine* engine, int* sequence, int num_indices, int* patterns, int num_patterns)
    {
        if (!engine) return -2;

        engine->insert_order(sequence, num_indices, patterns, num_patterns);

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
        if(pattern_id < 0 || pattern_id >= engine->order.num_patterns) return -1;
        return engine->order.patterns->rows[row_id][channel_id].instrumentId;
    }

    int get_note_id_from_channel(Engine* engine, int pattern_id, int row_id, int channel_id)
    {
        if (!engine) return -2;
        if(pattern_id < 0 || pattern_id >= engine->order.num_patterns) return -1;
        return engine->order.patterns->rows[row_id][channel_id].noteId;
    }
    
    int get_current_row(Engine* engine)
    {
        if (!engine) return -2;
        return engine->current_row;
    }

    //TODO: Finish WebAssembly functions
}