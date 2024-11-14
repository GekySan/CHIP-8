#ifndef UTILS_HPP
#define UTILS_HPP

#include "CHIP-8.hpp"

#include <SDL2/SDL.h>
#include <string>

std::string WideStringToString(const wchar_t* wstr);
std::string selectROMFile();

bool initializeSDL();
bool createWindowAndRenderer(SDL_Window** window, SDL_Renderer** renderer);
void handleEvents(bool& running, Chip8& chip8, const SDL_Scancode keys[16]);

#endif
