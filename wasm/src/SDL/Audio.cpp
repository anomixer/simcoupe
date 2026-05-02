// Part of SimCoupe - A SAM Coupe emulator
#include "SimCoupe.h"
#include "Audio.h"
#include "Options.h"
#include "Sound.h"

#ifdef __EMSCRIPTEN__
#include <SDL.h>
#endif

constexpr auto MIN_LATENCY_FRAMES = 
#ifdef __EMSCRIPTEN__
    12;
#else
    4;
#endif

SDL_AudioDeviceID dev = 0;

bool Audio::Init()
{
    Exit();
    SDL_AudioSpec desired{};
    desired.freq = SAMPLE_FREQ;
    desired.format = AUDIO_S16LSB;
    desired.channels = SAMPLE_CHANNELS;
#ifdef __EMSCRIPTEN__
    desired.samples = 2048; // Lower latency for WASM
#else
    desired.samples = 512;
#endif
    dev = SDL_OpenAudioDevice(nullptr, 0, &desired, nullptr, 0);
    if (!dev) return false;
    SDL_PauseAudioDevice(dev, 0);
    return true;
}

void Audio::Exit()
{
    if (dev) {
        SDL_CloseAudioDevice(dev);
        dev = 0;
    }
}

float Audio::AddData(uint8_t* pData_, int len_bytes)
{
    if (!dev) return 0.0f;
#ifdef __EMSCRIPTEN__
    if (SDL_GetAudioDeviceStatus(dev) != SDL_AUDIO_PLAYING) return 0.0f;
    SDL_QueueAudio(dev, pData_, len_bytes);
    return 0.0f;
#else
    SDL_QueueAudio(dev, pData_, len_bytes);
    auto buffer_frames = std::max(GetOption(latency), MIN_LATENCY_FRAMES);
    Uint32 buffer_size = SAMPLES_PER_FRAME * buffer_frames * BYTES_PER_SAMPLE;
    while (SDL_GetQueuedAudioSize(dev) >= buffer_size) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return static_cast<float>(SDL_GetQueuedAudioSize(dev)) / buffer_size;
#endif
}
