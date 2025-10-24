#include <chrono>
#include <fstream>
#include <iostream>
#include <ranges>
#include <sstream>
#include <thread>
#include <SDL2pp/SDL2pp.hh>
#include "chip8/Chip8.h"

//import std;

namespace sdl2 = SDL2pp;

int main(int argc, char* argv[]) {
    assert(argc == 2);

    sdl2::SDL sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    CHIP8::State state(std::ifstream(argv[1], std::ios::in | std::ios::binary | std::ios::ate));

    while (true) {
        using namespace std::chrono_literals;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            }
            else if (event.type == SDL_KEYDOWN) {
                switch (SDL_Keycode key = event.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        return 0;
                    default:
                        state.set_key(static_cast<SDL_KeyCode>(key), true);
                }
            }
            else if (event.type == SDL_KEYUP) {
                state.set_key(static_cast<SDL_KeyCode>(event.key.keysym.sym), false);
            }
        }
        if (!state.done()) {
            state.execute_one_instruction();
            state.draw();
            state.update_timers();
        }
        std::this_thread::sleep_for(1ms);
    }

    return 0;
}