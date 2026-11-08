//
// Created by herman on 10/20/25.
//

#ifndef CHIP8_DISPLAY_H
#define CHIP8_DISPLAY_H

#include <array>
#include <cstdint>
#include <memory>

#include "clock.h"

namespace CHIP8 {

// Abstract display/input/audio interface.  Keeping this free of SDL lets the
// CPU core be compiled and unit-tested without any SDL dependency.
class IDisplay {
public:
    virtual ~IDisplay() = default;

    virtual void clear() noexcept = 0;
    virtual std::uint8_t exchange_pixel(std::uint16_t x, std::uint16_t y,
                                        std::uint8_t pixel) noexcept = 0;
    virtual void draw() noexcept = 0;
    virtual void set_key(std::uint8_t key, bool is_down) noexcept = 0;
    [[nodiscard]] virtual bool is_pressed(std::uint8_t key) const noexcept = 0;
    [[nodiscard]] virtual const std::array<std::uint8_t, 16>& get_keys() const noexcept = 0;
    virtual void play_sound() noexcept = 0;
    virtual void stop_sound() noexcept = 0;
    [[nodiscard]] virtual bool needs_draw() const noexcept = 0;
};

// SDL implementation of IDisplay, defined in display.cpp.
class sdl_t;

std::unique_ptr<IDisplay> make_sdl_display() noexcept;

}  // namespace CHIP8

#endif  // CHIP8_DISPLAY_H
