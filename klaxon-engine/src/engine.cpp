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
    float* left_channel = output[0];
    float* right_channel = output[1];

    for(int i = 0; i < instrument_count; i++)
    {
        instruments[i]->render(output, frames);
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

void Engine::add_sample(const char* filename, const void* data, unsigned long length)
{
    std::unique_ptr<Instrument> instrument = std::make_unique<SampleInstrument>(SampleInstrument());

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

    void add_sample(Engine* engine, const char* filename, const void* data, unsigned long length)
    {
        if (!engine) return;
        engine->add_sample(filename, data, length);
    }

    void remove_instrument(Engine* engine, int instrument_id)
    {
        if (!engine) return;
        engine->remove_instrument(instrument_id);
    }

    //TODO: Finish WebAssembly functions
}