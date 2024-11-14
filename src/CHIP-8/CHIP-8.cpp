#include "CHIP-8.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>

Chip8::Chip8(const char* prog_file) : keypad(0), delay(0), sound(0), pc(PROG_OFFSET), ind(0), sp(0) {

    std::memset(ram, 0, sizeof(ram));
    std::memset(display, 0, sizeof(display));
    std::memset(stack, 0, sizeof(stack));
    std::memset(reg, 0, sizeof(reg));

    FILE* prog;
    errno_t err = fopen_s(&prog, prog_file, "rb");
    if (err != 0) {
        throw std::runtime_error("Échec de l'ouverture du fichier programme.");
    }
    std::fseek(prog, 0, SEEK_END);
    long size = std::ftell(prog);
    if (size > RAM_SIZE - PROG_OFFSET) {
        std::fclose(prog);
        throw std::runtime_error("La taille du programme dépasse la RAM disponible.");
    }
    std::fseek(prog, 0, SEEK_SET);
    if (std::fread(ram + PROG_OFFSET, 1, size, prog) < 1) {
        std::fclose(prog);
        throw std::runtime_error("Échec de la lecture du fichier programme.");
    }
    std::fclose(prog);

    static const Uint8 font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    std::memcpy(ram + FONT_OFFSET, font, sizeof(font));
}

void Chip8::runInstruction() {
    Uint16 instruction = (ram[pc] << 8) | ram[pc + 1];
    pc += 2;
    Uint8 opcode = instruction >> 12;

    switch (opcode) {
    case 0x0:
        if (instruction == 0x00E0) {
            clearScreen();
        }
        else if (instruction == 0x00EE) {
            returnFromSubroutine();
        }
        break;
    case 0x1:
        jumpToAddress(instruction);
        break;
    case 0x2:
        callSubroutine(instruction);
        break;
    case 0x3:
        skipIfEqualImmediate(instruction);
        break;
    case 0x4:
        skipIfNotEqualImmediate(instruction);
        break;
    case 0x5:
        skipIfEqualRegister(instruction);
        break;
    case 0x6:
        loadImmediate(instruction);
        break;
    case 0x7:
        addImmediate(instruction);
        break;
    case 0x8:
        performArithmetic(instruction);
        break;
    case 0x9:
        skipIfRegistersNotEqual(instruction);
        break;
    case 0xA:
        setIndexRegister(instruction);
        break;
    case 0xB:
        jumpWithOffset(instruction);
        break;
    case 0xC:
        generateRandomValue(instruction);
        break;
    case 0xD:
        drawSprite(instruction);
        break;
    case 0xE:
        handleKeyPress(instruction);
        break;
    case 0xF:
        handleTimersAndMemory(instruction);
        break;
    default:
        break;
    }
}

void Chip8::renderDisplay(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 7, 68, 220, 255);

    for (int y = 0; y < DISPLAY_ROWS; y++) {
        for (int x = 0; x < DISPLAY_COLS; x++) {
            if (display[y] & (1ULL << x)) {
                SDL_Rect rect = { x * PIXEL_SIZE, y * PIXEL_SIZE, PIXEL_SIZE, PIXEL_SIZE };
                SDL_RenderFillRect(renderer, &rect);
            }
        }
    }

    SDL_RenderPresent(renderer);
}

Uint8 Chip8::reverseByte(Uint8 b) {
    b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
    b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
    b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
    return b;
}

void Chip8::clearScreen() {
    std::memset(display, 0, sizeof(display));
}

void Chip8::returnFromSubroutine() {
    sp = (sp - 1) & (STACK_SIZE - 1);
    pc = stack[sp];
}

void Chip8::jumpToAddress(Uint16 instruction) {
    pc = instruction & 0x0FFF;
}

void Chip8::callSubroutine(Uint16 instruction) {
    stack[sp] = pc;
    sp = (sp + 1) & (STACK_SIZE - 1);
    pc = instruction & 0x0FFF;
}

void Chip8::skipIfEqualImmediate(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 value = instruction & 0x00FF;
    if (reg[vx] == value) {
        pc += 2;
    }
}

void Chip8::skipIfNotEqualImmediate(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 value = instruction & 0x00FF;
    if (reg[vx] != value) {
        pc += 2;
    }
}

void Chip8::skipIfEqualRegister(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 vy = (instruction & 0x00F0) >> 4;
    if (reg[vx] == reg[vy]) {
        pc += 2;
    }
}

void Chip8::loadImmediate(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 value = instruction & 0x00FF;
    reg[vx] = value;
}

void Chip8::addImmediate(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 value = instruction & 0x00FF;
    reg[vx] += value;
}

void Chip8::performArithmetic(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 vy = (instruction & 0x00F0) >> 4;
    Uint8 operation = instruction & 0x000F;

    switch (operation) {
    case 0x0:
        assignRegister(vx, vy);
        break;
    case 0x1:
        bitwiseOr(vx, vy);
        break;
    case 0x2:
        bitwiseAnd(vx, vy);
        break;
    case 0x3:
        bitwiseXor(vx, vy);
        break;
    case 0x4:
        addRegisters(vx, vy);
        break;
    case 0x5:
        subtractRegisters(vx, vy);
        break;
    case 0x6:
        shiftRight(vx, vy);
        break;
    case 0x7:
        subtractRegistersReverse(vx, vy);
        break;
    case 0xE:
        shiftLeft(vx, vy);
        break;
    default:
        break;
    }
}

void Chip8::assignRegister(Uint8 vx, Uint8 vy) {
    reg[vx] = reg[vy];
}

void Chip8::bitwiseOr(Uint8 vx, Uint8 vy) {
    reg[vx] |= reg[vy];
    reg[0xF] = 0;
}

void Chip8::bitwiseAnd(Uint8 vx, Uint8 vy) {
    reg[vx] &= reg[vy];
    reg[0xF] = 0;
}

void Chip8::bitwiseXor(Uint8 vx, Uint8 vy) {
    reg[vx] ^= reg[vy];
    reg[0xF] = 0;
}

void Chip8::addRegisters(Uint8 vx, Uint8 vy) {
    Uint8 bef = reg[vx];
    reg[vx] += reg[vy];
    reg[0xF] = (reg[vx] < bef) ? 1 : 0;
}

void Chip8::subtractRegisters(Uint8 vx, Uint8 vy) {
    Uint8 bef = reg[vx];
    reg[vx] -= reg[vy];
    reg[0xF] = (reg[vx] < bef) ? 1 : 0;
}

void Chip8::shiftRight(Uint8 vx, Uint8 vy) {
    reg[vx] = reg[vy];
    Uint8 carry = reg[vx] & 1;
    reg[vx] >>= 1;
    reg[0xF] = carry;
}

void Chip8::subtractRegistersReverse(Uint8 vx, Uint8 vy) {
    reg[vx] = reg[vy] - reg[vx];
    reg[0xF] = (reg[vx] < reg[vy]) ? 1 : 0;
}

void Chip8::shiftLeft(Uint8 vx, Uint8 vy) {
    reg[vx] = reg[vy];
    Uint8 carry = (reg[vx] & 0x80) ? 1 : 0;
    reg[vx] <<= 1;
    reg[0xF] = carry;
}

void Chip8::skipIfRegistersNotEqual(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 vy = (instruction & 0x00F0) >> 4;
    if (reg[vx] != reg[vy]) {
        pc += 2;
    }
}

void Chip8::setIndexRegister(Uint16 instruction) {
    ind = instruction & 0x0FFF;
}

void Chip8::jumpWithOffset(Uint16 instruction) {
    Uint16 addr = instruction & 0x0FFF;
    pc = (addr + reg[0]) & (RAM_SIZE - 1);
}

void Chip8::generateRandomValue(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 value = instruction & 0x00FF;
    reg[vx] = (std::rand() % 256) & value;
}

void Chip8::drawSprite(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 vy = (instruction & 0x00F0) >> 4;
    Uint8 height = instruction & 0x000F;

    reg[0xF] = 0;
    Uint8 x = reg[vx] % DISPLAY_COLS;
    Uint8 y = reg[vy] % DISPLAY_ROWS;

    for (int i = 0; i < height; i++) {
        int row = y + i;

        if ((ind + i) >= RAM_SIZE || row >= DISPLAY_ROWS) {
            break;
        }

        Uint8 spriteLine = ram[ind + i];
        Uint64 sprite = static_cast<Uint64>(reverseByte(spriteLine)) << x;

        if (display[row] & sprite) {
            reg[0xF] = 1;
        }
        display[row] ^= sprite;
    }
}

void Chip8::handleKeyPress(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 keyInstruction = instruction & 0x00FF;

    if (keyInstruction == 0x9E) {
        if (keypad & (1 << reg[vx])) {
            pc += 2;
        }
    }
    else if (keyInstruction == 0xA1) {
        if (!(keypad & (1 << reg[vx]))) {
            pc += 2;
        }
    }
}

void Chip8::handleTimersAndMemory(Uint16 instruction) {
    Uint8 vx = (instruction & 0x0F00) >> 8;
    Uint8 subcode = instruction & 0x00FF;

    switch (subcode) {
    case 0x07:
        getDelayTimer(vx);
        break;
    case 0x0A:
        awaitKeyPress(vx);
        break;
    case 0x15:
        setDelayTimer(vx);
        break;
    case 0x18:
        setSoundTimer(vx);
        break;
    case 0x1E:
        addToIndexRegister(vx);
        break;
    case 0x29:
        setIndexToFontCharacter(vx);
        break;
    case 0x33:
        storeBCDRepresentation(vx);
        break;
    case 0x55:
        storeRegistersInMemory(vx);
        break;
    case 0x65:
        loadRegistersFromMemory(vx);
        break;
    default:
        break;
    }
}

void Chip8::getDelayTimer(Uint8 vx) {
    reg[vx] = delay;
}

void Chip8::awaitKeyPress(Uint8 vx) {
    if (keypad) {
        for (int i = 0; i < 16; i++) {
            if (keypad & (1 << i)) {
                reg[vx] = i;
                break;
            }
        }
    }
    else {
        pc -= 2;
    }
}

void Chip8::setDelayTimer(Uint8 vx) {
    delay = reg[vx];
}

void Chip8::setSoundTimer(Uint8 vx) {
    sound = reg[vx];
}

void Chip8::addToIndexRegister(Uint8 vx) {
    ind = (ind + reg[vx]) & 0x0FFF;
    reg[0xF] = (ind > 0x0FFF) ? 1 : 0;
}

void Chip8::setIndexToFontCharacter(Uint8 vx) {
    ind = FONT_OFFSET + (reg[vx] & 0x0F) * 5;
}

void Chip8::storeBCDRepresentation(Uint8 vx) {
    Uint8 value = reg[vx];
    ram[ind] = value / 100;
    ram[ind + 1] = (value / 10) % 10;
    ram[ind + 2] = value % 10;
}

void Chip8::storeRegistersInMemory(Uint8 vx) {
    for (int i = 0; i <= vx; i++) {
        ram[ind + i] = reg[i];
    }
    ind += (vx + 1);
}

void Chip8::loadRegistersFromMemory(Uint8 vx) {
    for (int i = 0; i <= vx; i++) {
        reg[i] = ram[ind + i];
    }
    ind += (vx + 1);
}