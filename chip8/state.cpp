//
// Created by herman on 10/20/25.
//

#include "state.h"

#include <algorithm>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>

namespace CHIP8 {
    void execute_instruction(State&, const instr_t&) noexcept;

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
        if (!file) {
            return {};
        }
        const std::streamsize size = file.tellg();
        if (size <= 0) {
            std::cout << "File size: " << size << " bytes\n";
            return {};
        }
        std::cout << "File size: " << size << " bytes\n";
        file.seekg(0, std::ios::beg);

        std::vector<std::uint8_t> rom(static_cast<std::size_t>(size));
        file.read(reinterpret_cast<char*>(rom.data()), size);

        // Debug hex dump of the ROM (16-bit words, byte-swapped for display).
        std::span<std::uint16_t> view(reinterpret_cast<std::uint16_t*>(rom.data()), rom.size() / 2);
        std::cout << std::hex << std::setw(4) << std::setfill('0');
        std::size_t count = 0;
        for (auto j : view) {
            std::cout << std::setw(4) << std::setfill('0')
                      << static_cast<std::uint16_t>((j >> 8) | (j << 8)) << ' ';
            if (++count % 8 == 0) {
                std::cout << std::endl;
            }
        }

        return rom;
    }

    void State::load_rom(std::span<const std::uint8_t> rom) noexcept {
        for (int i = 0; i < 80; ++i) {
            _memory[i] = static_cast<std::uint8_t>(fonts[i]);
        }

        constexpr std::size_t max_rom_size = 4096 - 0x200;
        const std::size_t copy_size = std::min(rom.size(), max_rom_size);
        std::memcpy(_memory + 0x200, rom.data(), copy_size);
    }

    State::State(std::unique_ptr<IDisplay> display, std::ifstream&& file) noexcept :
        _memory{},
        _display{std::move(display)},
        _pc{0x200},
        _index{0x200},
        _stack{},
        _delay_timer{},
        _sound_timer{},
        _timer{std::chrono::steady_clock::now(), Clock::hz(60)},
        _regs{},
        _done{false}
    {
        if (!_display) {
            _done = true;
        }
        std::vector<std::uint8_t> vec = read_file(std::move(file));
        load_rom({vec.data(), vec.size()});
        std::cout << "initialized state" << std::endl;
    }

    State::State(std::unique_ptr<IDisplay> display, std::span<std::uint8_t> rom) noexcept :
        _memory{},
        _display{std::move(display)},
        _pc{0x200},
        _index{0x200},
        _stack{},
        _delay_timer{},
        _sound_timer{},
        _timer{std::chrono::steady_clock::now(), Clock::hz(60)},
        _regs{},
        _done{false}
    {
        if (!_display) {
            _done = true;
        }
        load_rom(rom);
        std::cout << "initialized state" << std::endl;
    }

    State::~State() noexcept = default;

    instr_t get_instruction(const std::uint8_t* memory, const std::uint16_t pc) noexcept {
        return static_cast<instr_t>((static_cast<unsigned>(memory[pc]) << 8) |
                                    memory[pc + 1]);
    }

    void State::execute_one_instruction() noexcept {
        if (_pc > 4094) {
            // A full instruction cannot be fetched past the end of memory.
            _done = true;
            return;
        }
        const auto instr = get_instruction(_memory, _pc);
        execute_instruction(*this, instr);
    }

    void State::draw() noexcept {
        _display->draw();
    }

    void State::advance_one_instruction() noexcept {
        if (_pc + 2 <= 4096) {
            _pc = static_cast<std::uint16_t>(_pc + 2);
        }
    }

    bool State::done() const noexcept {
        return _done;
    }

    void State::set_key(std::uint8_t key, bool is_down) noexcept {
        if (_display && key < 16) {
            _display->set_key(key, is_down);
        }
    }

    void State::update_timers() noexcept {
        _timer.invoke_if_can_update(
            [this]() -> void {
                if (_delay_timer > 0) {
                    _delay_timer--;
                }
                if (_sound_timer > 0) {
                    if (_display) {
                        _display->play_sound();
                    }
                    _sound_timer--;
                }
                else {
                    if (_display) {
                        _display->stop_sound();
                    }
                }
            }
        );
    }

}  // namespace CHIP8
