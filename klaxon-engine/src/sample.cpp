#include "sample.h"
#include "lib/miniaudio.h"

#include <vector>

#define MA_NO_FLAC
//TODO: TEST THIS PLEASE
void Sample::load_sample(const char* filename, const void* data, unsigned long length)
{
    ma_decoder decoder;
    ma_result result = ma_decoder_init_memory(data, length, nullptr, &decoder);

    if(result != MA_SUCCESS)
    {
        return;
    }

    ma_uint64 total_frames;
    
    ma_result result_1 = ma_decoder_get_length_in_pcm_frames(&decoder, &total_frames);

    if(result_1 != MA_SUCCESS)
    {
        return;
    }

    duration = total_frames;
    channels = decoder.outputChannels;

    std::vector<float> pcm(total_frames * decoder.outputChannels);

    ma_uint64 read_frames;

    ma_result result_2 = ma_decoder_read_pcm_frames(&decoder, pcm.data(), total_frames, &read_frames);

    if (result_2 != MA_SUCCESS)
    {
        return;
    }

    for (int i = 0; i < total_frames; i++)
    {

        if(channels == 1) 
        {
            left[i] = pcm[i * 2];
            right[i] = pcm[i * 2];
        } else if (channels == 2)
        {
            left[i] = pcm[i * 2];
            right[i] = pcm[i * 2 + 1];
        } else // max 2 channels
        {
            return;
        }
        
    }

    ma_decoder_uninit(&decoder);
}