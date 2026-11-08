//
// Created by herman on 10/20/25.
//

#ifndef CHIP8_STATE_H
#define CHIP8_STATE_H

#include <cstdint>
#include <fstream>
#include <memory>
#include <span>
#include <stack>

#include "clock.h"
#include "display.h"
#include "types.h"

namespace CHIP8 {

class State {
public:
    std::uint8_t _memory[4096]{};
    std::unique_ptr<IDisplay> _display;
    std::uint16_t _pc{};
    std::uint16_t _index{};
    std::stack<instr_t> _stack{};
    std::uint8_t _delay_timer{};
    std::uint8_t _sound_timer{};
    Clock _timer;
    std::uint8_t _regs[16]{};
    bool _done{};

    explicit State(std::unique_ptr<IDisplay> display, std::ifstream&& rom) noexcept;
    explicit State(std::unique_ptr<IDisplay> display, std::span<std::uint8_t> rom) noexcept;
    State() noexcept = delete;
    State(const State&) = delete;
    State(State&&) = delete;
    State& operator=(const State&) = delete;
    State& operator=(State&&) = delete;
    ~State() noexcept;

    void execute_one_instruction() noexcept;
    void draw() noexcept;
    void advance_one_instruction() noexcept;
    [[nodiscard]] bool done() const noexcept;
    // `key` is a CHIP-8 key index (0x0..0xF), not an SDL keycode.
    void set_key(std::uint8_t key, bool is_down) noexcept;
    void update_timers() noexcept;

private:
    void load_rom(std::span<const std::uint8_t> rom) noexcept;
};

}  // namespace CHIP8

#endif  // CHIP8_STATE_H
