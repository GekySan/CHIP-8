#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>

#include "Audio.hpp"
#include "CHIP-8.hpp"
#include "Constants.hpp"
#include "Locale_Initializer.hpp"
#include "Timer.hpp"
#include "Utils.hpp"

#include <commdlg.h>
#include <conio.h>
#include <iostream>
#include <windows.h>

int main() {
    std::string filePathStr = selectROMFile();

    if (filePathStr.empty()) {
        std::cout << "Aucun fichier ROM sélectionné. Fermeture de l'application.\n";
        (void)_getch();
        return -1;
    }

    if (!initializeSDL()) {
        return -1;
    }

    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    if (!createWindowAndRenderer(&window, &renderer)) {
        SDL_Quit();
        return -1;
    }

    SDL_SetWindowTitle(window, "Émulateur CHIP-8");

    // Optionnel
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "direct3d12");
    SDL_SetHint(SDL_HINT_AUDIO_RESAMPLING_MODE, "best");

    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    #ifdef _DEBUG
        std::cout << "Fichier chargé -> " << filePathStr.c_str() << std::endl;
    #endif

    Chip8 chip8(filePathStr.c_str());

    SDL_AudioDeviceID audioDevice = initializeAudio(chip8);

    SDL_Scancode keys[16] = {
        SDL_SCANCODE_X, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3,
        SDL_SCANCODE_Q, SDL_SCANCODE_W, SDL_SCANCODE_E, SDL_SCANCODE_A,
        SDL_SCANCODE_S, SDL_SCANCODE_D, SDL_SCANCODE_Z, SDL_SCANCODE_C,
        SDL_SCANCODE_4, SDL_SCANCODE_R, SDL_SCANCODE_F, SDL_SCANCODE_V
    };


    std::clock_t last_frame_time = std::clock();
    std::clock_t last_instr_time = std::clock();

    bool running = true;
    while (running) {
        std::clock_t current_time = std::clock();

        if (static_cast<double>(current_time - last_instr_time) >= (static_cast<double>(CLOCKS_PER_SEC) / IPS)) {
            try {
                chip8.runInstruction();
            }
            catch (const std::exception& e) {
                std::cerr << "Erreur lors de l'exécution de l'instruction: " << e.what() << "\n";
                running = false;
                break;
            }
            last_instr_time = current_time;
        }

        if (static_cast<double>(current_time - last_frame_time) >= (static_cast<double>(CLOCKS_PER_SEC) / FPS)) {
            handleEvents(running, chip8, keys);
            updateTimer(chip8);
            chip8.renderDisplay(renderer);
            last_frame_time = current_time;
        }
    }

    if (audioDevice != 0) {
        SDL_PauseAudioDevice(audioDevice, 1);
        SDL_CloseAudioDevice(audioDevice);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    SDL_Quit();

    return 0;
}
