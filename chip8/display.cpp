//
// Created by herman on 10/20/25.
//

#include "display.h"

#include <algorithm>
#include <chrono>
#include <memory>
#include <optional>
#include <utility>

#include <SDL2pp/SDL2pp.hh>
#include <SDL_mixer.h>

namespace sdl2 = SDL2pp;

namespace CHIP8 {

namespace {

// The beep is loaded lazily so that a missing sound file (or an SDL_mixer
// build without MP3 support) never crashes the emulator.
std::optional<sdl2::Chunk>& beep_chunk() noexcept {
    static std::optional<sdl2::Chunk> chunk;
    if (!chunk.has_value()) {
        try {
            chunk.emplace("./beep.mp3");
        }
        catch (...) {
            // Sound unavailable; the emulator continues without audio.
            chunk = std::nullopt;
        }
    }
    return chunk;
}

}  // namespace

class sdl_t : public IDisplay {
    sdl2::Window _window;
    sdl2::Renderer _renderer;
    sdl2::Texture _texture;
    std::array<std::array<std::uint8_t, 64>, 32> _pixels;
    bool _needs_draw;
    std::array<std::uint8_t, 16> _keys;
    sdl2::Mixer _mixer;
    Clock _timer;

public:
    sdl_t();
    void clear() noexcept override;
    std::uint8_t exchange_pixel(std::uint16_t x, std::uint16_t y,
                                std::uint8_t pixel) noexcept override;
    void draw() noexcept override;
    void set_key(std::uint8_t key, bool is_down) noexcept override;
    [[nodiscard]] bool is_pressed(std::uint8_t key) const noexcept override;
    [[nodiscard]] const std::array<std::uint8_t, 16>& get_keys() const noexcept override;
    void play_sound() noexcept override;
    void stop_sound() noexcept override;
    [[nodiscard]] bool needs_draw() const noexcept override;
};

sdl_t::sdl_t() :
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
    // Defensive bounds check: the DRW opcode already wraps coordinates, but
    // guard anyway in case exchange_pixel is called from elsewhere.
    if (y >= 32 || x >= 64) {
        return 0;
    }
    const auto ret = pixel & _pixels[y][x];
    _pixels[y][x] ^= pixel;
    _needs_draw = true;
    return ret;
}

void sdl_t::clear() noexcept {
    _renderer.Clear();
    std::fill_n(_pixels.data()->data(), 64 * 32, 0);
    _needs_draw = true;
}

void sdl_t::draw() noexcept {
    _timer.invoke_if_can_update(
        [this]() -> void {
            if (_needs_draw) {
                std::array<std::array<std::uint32_t, 64>, 32> arr{};
                // arr[row][col] holds the pixel at CHIP-8 coordinate (col, row).
                for (std::size_t row = 0; row < arr.size(); ++row) {
                    for (std::size_t col = 0; col < arr[row].size(); ++col) {
                        arr[row][col] = _pixels[row][col] ? 0xFFFFFFFFu : 0xFF000000u;
                    }
                }
                _texture.Update({}, arr.data(), 64 * sizeof(std::uint32_t));
                _renderer.Clear();
                _renderer.Copy(_texture);
                _renderer.Present();
            }
        }
    );
}

void sdl_t::set_key(std::uint8_t key, bool is_down) noexcept {
    if (key < 16) {
        _keys[key] = is_down;
    }
}

bool sdl_t::is_pressed(std::uint8_t key) const noexcept {
    return key < 16 && _keys[key];
}

const std::array<std::uint8_t, 16>& sdl_t::get_keys() const noexcept {
    return _keys;
}

void sdl_t::play_sound() noexcept {
    if (auto& chunk = beep_chunk(); chunk.has_value()) {
        _mixer.PlayChannel(0, *chunk);
    }
}

void sdl_t::stop_sound() noexcept {
    _mixer.HaltChannel(-1);
}

bool sdl_t::needs_draw() const noexcept {
    return _needs_draw;
}

std::unique_ptr<IDisplay> make_sdl_display() noexcept {
    try {
        return std::make_unique<sdl_t>();
    }
    catch (...) {
        // The display cannot be created (e.g. no video device); report failure
        // instead of terminating.
        return nullptr;
    }
}

}  // namespace CHIP8
