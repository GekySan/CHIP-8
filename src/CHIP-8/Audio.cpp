#include "Audio.hpp"
#include "Constants.hpp"

#include <cstring>
#include <iostream>

void audioCallback(void* userdata, Uint8* stream, int len) {
    Chip8* chip8 = static_cast<Chip8*>(userdata);
    if (chip8->sound) {
        for (int i = 0; i < len; i++) {
            int x = i % 100;
            if (x > 50) x = 100 - x;
            stream[i] = static_cast<Uint8>(x * 0.5);
        }
    }
    else {
        std::memset(stream, 0, len);
    }
}

SDL_AudioDeviceID initializeAudio(Chip8& chip8) {
    SDL_AudioSpec desiredSpec;
    SDL_zero(desiredSpec);
    desiredSpec.freq = 48000;
    desiredSpec.format = AUDIO_S8;
    desiredSpec.channels = 1;
    desiredSpec.samples = desiredSpec.freq / FPS;
    desiredSpec.callback = audioCallback;
    desiredSpec.userdata = &chip8;

    SDL_AudioSpec obtainedSpec;
    SDL_AudioDeviceID audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desiredSpec, &obtainedSpec, 0);
    if (audioDevice == 0) {
        std::cerr << "Échec de l'ouverture du périphérique audio: " << SDL_GetError() << std::endl;
    }
    else {
        SDL_PauseAudioDevice(audioDevice, 0);
    }
    return audioDevice;
}
