//
// Created by herman on 10/20/25.
//

#ifndef CHIP8_STATE_H
#define CHIP8_STATE_H

#include <span>

#include "display.h"
#include "types.h"
#include <stack>
#include <chrono>

#include "clock.h"


namespace CHIP8 {
    class State {
    public:
        std::uint8_t _memory[4096];
        sdl_t _sdl;
        std::uint16_t _pc;
        std::uint16_t _index;
        std::stack<instr_t> _stack;
        std::uint8_t _delay_timer;
        std::uint8_t _sound_timer;
        Clock _timer;
        std::uint8_t _regs[16];
        bool _done;

        explicit State(std::ifstream&& rom) noexcept;
        explicit State(std::span<std::uint8_t>) noexcept;
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
        void set_key(const SDL_KeyCode&, bool) noexcept;
        void update_timers() noexcept;

    };
}

#endif //CHIP8_STATE_H