#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "CHIP-8.hpp"

#include <SDL2/SDL.h>

void audioCallback(void* userdata, Uint8* stream, int len);
SDL_AudioDeviceID initializeAudio(Chip8& chip8);

#endif
