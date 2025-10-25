//
// Created by herman on 10/20/25.
//

#include "display.h"

#include <ranges>
#include <utility>
#include <SDL_mixer.h>

namespace CHIP8 {
    sdl_t::sdl_t() noexcept :
        _window("CHIP-8",
              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
              20 * 64, 20 * 32,
              SDL_WINDOW_RESIZABLE),
        _renderer(_window, -1, SDL_RENDERER_ACCELERATED),
        _texture(_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 64, 32),
        _pixels{},
        _needs_draw{true},
        _keys{},
        _mixer{MIX_DEFAULT_FREQUENCY, AUDIO_S8, 1, 4096},
        _timer{std::chrono::steady_clock::now(), Clock::hz(60)}
    {}
    std::uint8_t sdl_t::exchange_pixel(std::uint16_t x, std::uint16_t y, std::uint8_t pixel) noexcept {
        auto ret = pixel & _pixels[y][x];
        _pixels[y][x] ^= pixel;
        _needs_draw = true;
        return ret;
    }
    void sdl_t::clear() noexcept {
        _renderer.Clear();
        std::fill_n(_pixels.data()->data(), 64*32, 0);
        _needs_draw = true;
    }
    void sdl_t::draw() noexcept {
        _timer.invoke_if_can_update(
        [this]() -> void {
            if (_needs_draw) {
                std::array<std::array<std::uint32_t, 64>, 32> arr{};
                for (auto&& [x, row] : arr | std::ranges::views::enumerate) {
                    for (auto&& [y, elem] : row | std::ranges::views::enumerate) {
                        elem = _pixels[x][y] ? 0xFFFFFFFF : 0xFF000000;
                    }
                }
                _texture.Update({}, arr.data(),  64 * sizeof(std::uint32_t));
                _renderer.Clear();
                _renderer.Copy(_texture);
                _renderer.Present();
            }
        }
        );

    }
    void sdl_t::set_key(std::uint8_t key, bool is_down) noexcept {
        /*if (is_down) {
            std::cout << "pressing key " << std::hex << static_cast<int>(key) << "\n";
        }
        else {
            std::cout << "releasing key " << std::hex << static_cast<int>(key) << "\n";
        }
        std::cout << std::flush;Z*/
        _keys[key] = is_down;
    }

    bool sdl_t::is_pressed(const std::uint8_t& key) const noexcept {
        return _keys[key];
    }
    const std::array<std::uint8_t, 16>& sdl_t::get_keys() const noexcept {
        return _keys;
    }
    void sdl_t::play_sound() noexcept {
        static const sdl2::Chunk beep("./beep.mp3");
        _mixer.PlayChannel(0, beep);
        //std::cout << "playing sound" << std::endl;

    }
    void sdl_t::stop_sound() noexcept {
        _mixer.HaltChannel(-1);
        //std::cout << "stopping sound" << std::endl;
    }
    bool sdl_t::needs_draw() const noexcept {
        return _needs_draw;
    }

}
