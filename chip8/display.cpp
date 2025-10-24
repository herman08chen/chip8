//
// Created by herman on 10/20/25.
//

#include "display.h"

#include <ranges>
#include <utility>
#include <SDL_mixer.h>

namespace CHIP8 {
    sdl_t::sdl_t() noexcept :
            window("CHIP-8",
                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                    size_mult * 64, size_mult * 32,
                    SDL_WINDOW_RESIZABLE),
            renderer(window, -1, SDL_RENDERER_ACCELERATED),
            texture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, size_mult * 64, size_mult * 32),
            pixels{},
            needs_draw{true},
            mixer{MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 4096}
    {}
    std::uint8_t sdl_t::exchange_pixel(std::uint16_t x, std::uint16_t y, std::uint8_t pixel) noexcept {
        auto ret = pixel & pixels[y][x];
        pixels[y][x] ^= pixel;
        needs_draw = true;
        return ret;
    }
    void sdl_t::clear() noexcept {
        renderer.Clear();
        std::fill_n(pixels.data()->data(), 64*32, 0);
        needs_draw = true;
    }
    void sdl_t::draw() noexcept {
        if (needs_draw) {
            std::array<std::array<std::uint32_t, 64*size_mult>, 32*size_mult> arr{};
            for (auto&& [x, row] : arr | std::ranges::views::enumerate) {
                for (auto&& [y, elem] : row | std::ranges::views::enumerate) {
                    elem = pixels[x / size_mult][y / size_mult] ? 0xFFFFFFFF : 0xFF000000;
                }
            }
            texture.Update({}, reinterpret_cast<void*>(arr.data()), size_mult * 64 * sizeof(std::uint32_t));
            renderer.Clear();
            renderer.Copy(texture, {});
            renderer.Present();
        }
    }
    void sdl_t::set_key(std::uint8_t key, bool is_down) noexcept {
        /*if (is_down) {
            std::cout << "pressing key " << std::hex << static_cast<int>(key) << "\n";
        }
        else {
            std::cout << "releasing key " << std::hex << static_cast<int>(key) << "\n";
        }
        std::cout << std::flush;Z*/
        keys[key] = is_down;
    }

    bool sdl_t::is_pressed(const std::uint8_t& key) const noexcept {
        return keys[key];
    }
    const std::array<std::uint8_t, 16>& sdl_t::get_keys() const noexcept {
        return keys;
    }
    void sdl_t::play_sound() noexcept {
        static sdl2::Music beep("./beep.mp3");
        mixer.PlayMusic(beep, INT_MAX);
        //std::cout << "playing sound" << std::endl;

    }
    void sdl_t::stop_sound() noexcept {
        mixer.HaltMusic();
        //std::cout << "stopping sound" << std::endl;
    }
}
