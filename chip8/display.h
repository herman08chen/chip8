//
// Created by herman on 10/20/25.
//

#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H


#include <SDL2pp/SDL2pp.hh>

#include "clock.h"
namespace sdl2 = SDL2pp;


namespace CHIP8 {
    class sdl_t {
        sdl2::Window _window;
        sdl2::Renderer _renderer;
        sdl2::Texture _texture;
        std::array<std::array<std::uint8_t, 64>, 32> _pixels;
        bool _needs_draw;
        std::array<std::uint8_t, 16> _keys;
        sdl2::Mixer _mixer;
        Clock _timer;

    public:
        sdl_t() noexcept;
        void clear() noexcept;
        std::uint8_t exchange_pixel(std::uint16_t x, std::uint16_t y, std::uint8_t pixel) noexcept;;
        void draw() noexcept;
        void set_key(std::uint8_t, bool is_down) noexcept;
        [[nodiscard]] bool is_pressed(const std::uint8_t&) const noexcept;
        [[nodiscard]] const std::array<std::uint8_t, 16>& get_keys() const noexcept;
        void play_sound() noexcept;
        void stop_sound() noexcept;
        [[nodiscard]] bool needs_draw() const noexcept;
    };
}

#endif //CHIP8_DISPLAY_H