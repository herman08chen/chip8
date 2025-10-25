//
// Created by herman on 10/20/25.
//

#include "state.h"

#include <cstring>
#include <fstream>
#include <iomanip>
#include <ranges>

namespace CHIP8 {
    constexpr std::array<int, 80> fonts {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80  // F
    };
    std::vector<std::uint8_t> read_file(std::ifstream&& file) {
        std::streamsize size = file.tellg();
        std::cout << "File size: " << size << " bytes\n";
        file.seekg(0, std::ios::beg);

        std::vector<std::uint8_t> rom(size);

        file.read(reinterpret_cast<char*>(rom.data()), size);

        std::span<std::uint16_t> view(reinterpret_cast<std::uint16_t*>(rom.data()), rom.size() / 2);
        std::cout << std::hex << std::setw(4) << std::setfill('0');
        for (auto&& i : view | std::ranges::views::chunk(8)) {
            for (auto&& j : i ) {
                std::cout << std::setw(4) << std::setfill('0') << std::byteswap(j) << ' ';
            }
            std::cout << std::endl;
        }

        return rom;
    }
    State::State(std::ifstream&& file) noexcept :
        _memory{},
        _sdl{},
        _pc{0x200},
        _index{0x200},
        _stack{},
        _delay_timer{},
        _sound_timer{},
        _timer{std::chrono::steady_clock::now(), 33ms},
        _regs{},
        _done{false}
    {
        std::vector<std::uint8_t> vec = read_file(std::move(file));
        std::span rom(vec);
        for (int i = 0; i < 80; i++) {
            _memory[i] = fonts[i];
        }
        std::memcpy(_memory + 0x200, rom.data(), rom.size());
        std::cout << "initialized state" << std::endl;
    }
    State::State(std::span<std::uint8_t> rom) noexcept :
        _memory{},
        _sdl{},
        _pc{0x200},
        _index{0x200},
        _stack{},
        _delay_timer{},
        _sound_timer{},
        _timer{std::chrono::steady_clock::now(), Clock::hz(60)},
        _regs{},
        _done{false}
    {
        std::memcpy(_memory + 0x200, rom.data(), rom.size());
        std::cout << "initialized state" << std::endl;
    }

    State::~State() noexcept = default;
    void execute_instruction(State&, const instr_t&) noexcept;

    instr_t get_instruction(const std::uint8_t* _memory, const std::uint16_t _pc) noexcept {
        return (static_cast<instr_t>(_memory[_pc]) << 8) + _memory[_pc + 1];
    }

    void State::execute_one_instruction() noexcept {
        const auto instr = get_instruction(_memory, _pc);
        execute_instruction(*this, instr);

        //std::cout << std::hex << std::setw(4) << std::setfill('0') << instr << std::endl;
    }
    void State::draw() noexcept {
        _sdl.draw();
    }
    void State::advance_one_instruction() noexcept {
        if (_pc + 2 < 4096)
            _pc += 2;
    }
    bool State::done() const noexcept {
        return _done;
    }
    void State::set_key(const SDL_KeyCode& key, bool is_down) noexcept {
        switch (key) {
            case SDLK_1:
                _sdl.set_key(1, is_down);
                break;
            case SDLK_2:
                _sdl.set_key(2, is_down);
                break;
            case SDLK_3:
                _sdl.set_key(3, is_down);
                break;
            case SDLK_4:
                _sdl.set_key(0xC, is_down);
                break;
            case SDLK_q:
                _sdl.set_key(4, is_down);
                break;
            case SDLK_w:
                _sdl.set_key(5, is_down);
                break;
            case SDLK_e:
                _sdl.set_key(6, is_down);
                break;
            case SDLK_r:
                _sdl.set_key(0xD, is_down);
                break;
            case SDLK_a:
                _sdl.set_key(7, is_down);
                break;
            case SDLK_s:
                _sdl.set_key(8, is_down);
                break;
            case SDLK_d:
                _sdl.set_key(0xE, is_down);
                break;
            case SDLK_z:
                _sdl.set_key(0xA, is_down);
                break;
            case SDLK_x:
                _sdl.set_key(0, is_down);
                break;
            case SDLK_c:
                _sdl.set_key(0xB, is_down);
                break;
            case SDLK_v:
                _sdl.set_key(0xF, is_down);
                break;
            default: ;
        }
    }
    void State::update_timers() noexcept {
        _timer.invoke_if_can_update(
            [this]() -> void {
                if (_delay_timer > 0)   _delay_timer--;
                if (_sound_timer > 0) {
                    _sdl.play_sound();
                    _sound_timer--;
                }
                else {
                    _sdl.stop_sound();
                }
            }
        );
    }

}
