#include "Timer.hpp"

#include <cstring>
#include <iostream>
#include <windows.h>

void updateTimer(Chip8& chip8) {
    if (chip8.delay > 0) {
        chip8.delay--;
    }
    if (chip8.sound > 0) {
        chip8.sound--;
    }
}