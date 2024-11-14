#ifndef CHIP8_HPP
#define CHIP8_HPP

#include "Constants.hpp"

#include <cstdint>
#include <fstream>
#include <SDL2/SDL.h>
#include <stdexcept>

class Chip8 {
public:
    Chip8(const char* prog_file);

    void runInstruction();

    void renderDisplay(SDL_Renderer* renderer);

    Uint16 keypad;

    Uint8 delay;
    Uint8 sound;

private:
    Uint8 ram[RAM_SIZE];
    Uint64 display[DISPLAY_ROWS];
    Uint16 pc;
    Uint16 ind;
    Uint16 stack[STACK_SIZE];
    Uint8 sp;
    Uint8 reg[16];

    Uint8 reverseByte(Uint8 b);

    void clearScreen();
    void returnFromSubroutine();

    void jumpToAddress(Uint16 instruction);

    void callSubroutine(Uint16 instruction);

    void skipIfEqualImmediate(Uint16 instruction);

    void skipIfNotEqualImmediate(Uint16 instruction);

    void skipIfEqualRegister(Uint16 instruction);

    void loadImmediate(Uint16 instruction);

    void addImmediate(Uint16 instruction);

    void performArithmetic(Uint16 instruction);
    void assignRegister(Uint8 vx, Uint8 vy);
    void bitwiseOr(Uint8 vx, Uint8 vy);
    void bitwiseAnd(Uint8 vx, Uint8 vy);
    void bitwiseXor(Uint8 vx, Uint8 vy);
    void addRegisters(Uint8 vx, Uint8 vy);
    void subtractRegisters(Uint8 vx, Uint8 vy);
    void shiftRight(Uint8 vx, Uint8 vy);
    void subtractRegistersReverse(Uint8 vx, Uint8 vy);
    void shiftLeft(Uint8 vx, Uint8 vy);

    void skipIfRegistersNotEqual(Uint16 instruction);

    void setIndexRegister(Uint16 instruction);

    void jumpWithOffset(Uint16 instruction);

    void generateRandomValue(Uint16 instruction);

    void drawSprite(Uint16 instruction);

    void handleKeyPress(Uint16 instruction);

    void handleTimersAndMemory(Uint16 instruction);
    void getDelayTimer(Uint8 vx);
    void awaitKeyPress(Uint8 vx);
    void setDelayTimer(Uint8 vx);
    void setSoundTimer(Uint8 vx);
    void addToIndexRegister(Uint8 vx);
    void setIndexToFontCharacter(Uint8 vx);
    void storeBCDRepresentation(Uint8 vx);
    void storeRegistersInMemory(Uint8 vx);
    void loadRegistersFromMemory(Uint8 vx);

};

#endif