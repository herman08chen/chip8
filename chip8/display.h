//
// Created by herman on 10/20/25.
//

#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H


#include <SDL2pp/SDL2pp.hh>
namespace sdl2 = SDL2pp;


namespace CHIP8 {
    class sdl_t {
        static constexpr std::size_t size_mult = 10;
        sdl2::Window window;
        sdl2::Renderer renderer;
        sdl2::Texture texture;
        std::array<std::array<std::uint8_t, 64>, 32> pixels;
        bool needs_draw;
        std::array<std::uint8_t, 16> keys;
        sdl2::Mixer mixer;

    public:
        sdl_t() noexcept;
        void clear() noexcept;
        std::uint8_t exchange_pixel(std::uint16_t x, std::uint16_t y, std::uint8_t pixel) noexcept;;
        void draw() noexcept;
        void set_key(std::uint8_t, bool is_down) noexcept;
        bool is_pressed(const std::uint8_t&) const noexcept;
        const std::array<std::uint8_t, 16>& get_keys() const noexcept;
        void play_sound() noexcept;
        void stop_sound() noexcept;
    };
}

#endif //CHIP8_DISPLAY_H