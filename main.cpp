#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <optional>
#include <thread>

#include <SDL2pp/SDL2pp.hh>

#include "chip8/Chip8.h"
#include "chip8/clock.h"

namespace sdl2 = SDL2pp;

// Map an SDL keycode to a CHIP-8 key index (0x0..0xF).  Returns nullopt for
// keys that are not part of the CHIP-8 keypad.
[[nodiscard]] constexpr auto map_key(SDL_Keycode key) noexcept -> std::optional<std::uint8_t> {
    switch (key) {
        case SDLK_1:  return std::uint8_t{0x1};
        case SDLK_2:  return std::uint8_t{0x2};
        case SDLK_3:  return std::uint8_t{0x3};
        case SDLK_4:  return std::uint8_t{0xC};
        case SDLK_q:  return std::uint8_t{0x4};
        case SDLK_w:  return std::uint8_t{0x5};
        case SDLK_e:  return std::uint8_t{0x6};
        case SDLK_r:  return std::uint8_t{0xD};
        case SDLK_a:  return std::uint8_t{0x7};
        case SDLK_s:  return std::uint8_t{0x8};
        case SDLK_d:  return std::uint8_t{0xE};
        case SDLK_z:  return std::uint8_t{0xA};
        case SDLK_x:  return std::uint8_t{0x0};
        case SDLK_c:  return std::uint8_t{0xB};
        case SDLK_v:  return std::uint8_t{0xF};
        default:      return std::nullopt;
    }
}

int main(int argc, char* argv[]) {
    using namespace std::chrono_literals;

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <rom-file>\n";
        return 1;
    }

    sdl2::SDL sdl(SDL_INIT_VIDEO | SDL_INIT_AUDIO);

    auto display = CHIP8::make_sdl_display();
    CHIP8::State state(std::move(display),
                       std::ifstream(argv[1], std::ios::in | std::ios::binary | std::ios::ate));

    CHIP8::Clock clock(std::chrono::steady_clock::now(), 2ms);
    while (true) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                return 0;
            }
            else if (event.type == SDL_KEYDOWN) {
                const SDL_Keycode key = event.key.keysym.sym;
                if (key == SDLK_ESCAPE) {
                    return 0;
                }
                if (const auto chip8_key = map_key(key); chip8_key) {
                    state.set_key(*chip8_key, true);
                }
            }
            else if (event.type == SDL_KEYUP) {
                if (const auto chip8_key = map_key(event.key.keysym.sym); chip8_key) {
                    state.set_key(*chip8_key, false);
                }
            }
        }

        if (!state.done()) {
            state.execute_one_instruction();
            state.draw();
            state.update_timers();
        }
        else if (state._display && state._display->needs_draw()) {
            state._display->draw();
        }

        clock.sleep_until_next_period();
    }

    return 0;
}
