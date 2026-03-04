#include "engine.h"

void Engine::init(int sample_rate, int bpm, int rows_per_beat)
{
    this->sample_rate = sample_rate;
    this->bpm = bpm;
    this->rows_per_beat = rows_per_beat;

    this->samples_per_row = static_cast<float>(sample_rate) * 60.f / bpm / rows_per_beat;
}

int Engine::process(float** output, int frames)
{
    int is_hit = step(frames);
    clear(output, frames);
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

    // calculate time
    int start_sample = current_samples;
    int end_sample = current_samples + frames;

    int start_row = start_sample / samples_per_row;
    int end_row = (end_sample - 1) / samples_per_row;

    if (end_row > start_row)
    {
        is_hit = 1; // we have hit a row, time to start signalling!
    }

    current_samples = end_sample;

    return is_hit;
}

void Engine::play(int row_no)
{   
    if (int apparent_samples = row_no * samples_per_row; apparent_samples != current_samples) 
        current_samples = apparent_samples;

    is_playing = true;
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

    void play_track(Engine* engine, int elapsed_rows)
    {
        if (!engine) return;
        engine->play(elapsed_rows);
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

        // free(left);
        // free(right);

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
        if(instrument_id >= engine->instruments.size()) return -1;
        engine->poly.add_voice(0, note_id, instrument_id, 1.0f, 72);

        return 0;
    }

    int read_order(Engine* engine, int* patterns, int num_patterns, int* sequence, int num_indices)
    {
        
    }

    //TODO: Finish WebAssembly functions
}