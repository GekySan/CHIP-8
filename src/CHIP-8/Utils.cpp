#include "Constants.hpp"
#include "Utils.hpp"

#include <cstring>
#include <iostream>
#include <windows.h>

std::string WideStringToString(const wchar_t* wstr) {
    if (wstr == nullptr) return "";

    int size_needed = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size_needed <= 0) {
        return "";
    }

    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, &strTo[0], size_needed, nullptr, nullptr);

    if (!strTo.empty() && strTo.back() == '\0') {
        strTo.pop_back();
    }

    return strTo;
}

std::string selectROMFile() {
    wchar_t filePath[MAX_PATH] = L"";

    LPCWSTR filter = L"CHIP-8 ROMs (*.ch8)\0*.ch8\0All Files (*.*)\0*.*\0";

    OPENFILENAMEW ofn;
    ZeroMemory(&ofn, sizeof(ofn));

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = filter;
    ofn.lpstrFile = filePath;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"ch8";

    if (!GetOpenFileNameW(&ofn)) {
        DWORD error = CommDlgExtendedError();
        if (error != 0) {
            std::cerr << "Erreur lors de la sélection du fichier ROM." << std::endl;
        }
        else {
            std::cout << "Aucun fichier ROM sélectionné." << std::endl;
        }
        return "";
    }

    std::string filePathStr = WideStringToString(filePath);
    return filePathStr;
}

bool initializeSDL() {
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        std::cerr << "Erreur SDL: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

bool createWindowAndRenderer(SDL_Window** window, SDL_Renderer** renderer) {
    if (SDL_CreateWindowAndRenderer(DISPLAY_COLS * PIXEL_SIZE, DISPLAY_ROWS * PIXEL_SIZE, 0, window, renderer) != 0) {
        std::cerr << "Erreur lors de la création de la fenêtre et du renderer: " << SDL_GetError() << std::endl;
        return false;
    }
    return true;
}

void handleEvents(bool& running, Chip8& chip8, const SDL_Scancode keys[16]) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            running = false;
        }
        if (e.type == SDL_KEYDOWN) {
            for (int i = 0; i < 16; i++) {
                if (e.key.keysym.scancode == keys[i]) {
                    chip8.keypad |= (1 << i);
                    break;
                }
            }
        }
        if (e.type == SDL_KEYUP) {
            for (int i = 0; i < 16; i++) {
                if (e.key.keysym.scancode == keys[i]) {
                    chip8.keypad &= ~(1 << i);
                    break;
                }
            }
        }
    }
}

