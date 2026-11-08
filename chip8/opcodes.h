//
// Created by herman on 10/20/25.
//

#ifndef CHIP8_OPCODES_H
#define CHIP8_OPCODES_H

#include "state.h"
#include "types.h"

#include <array>
#include <random>
#include <ranges>

namespace CHIP8 {

struct vtable_t {
    void (*execute)(State&, const instr_t&);
};

constexpr auto deconstruct(const instr_t& instr) {
    struct ret_t {
        unsigned int _;
        unsigned int X;
        unsigned int Y;
        unsigned int N;
        unsigned int NN;
        unsigned int NNN;
    };
    return ret_t {
        ._ = (instr & 0xF000u) >> 12,
        .X = (instr & 0x0F00u) >> 8,
        .Y = (instr & 0x00F0u) >> 4,
        .N = (instr & 0x000Fu),
        .NN = (instr & 0x00FFu),
        .NNN = (instr & 0x0FFFu)
    };
}

constexpr vtable_t _0 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        [[maybe_unused]] auto bits = deconstruct(instr);
        if (instr == 0x00E0) {
            state._display->clear();
            state.advance_one_instruction();
        }
        else if (instr == 0x00EE) {
            if (state._stack.empty()) {
                state._done = true;
            }
            else {
                state._pc = state._stack.top();
                state._stack.pop();
                state.advance_one_instruction();
            }
        }
        else {
            state._done = true;
        }
    },
};

constexpr vtable_t _1 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        state._pc = bits.NNN;
    },
};

constexpr vtable_t _2 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        state._stack.push(state._pc);
        state._pc = bits.NNN;
    },
};

constexpr vtable_t _3 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        state.advance_one_instruction();
        if (state._regs[bits.X] == bits.NN) {
            state.advance_one_instruction();
        }
    }
};

constexpr vtable_t _4 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        state.advance_one_instruction();
        if (state._regs[bits.X] != bits.NN) {
            state.advance_one_instruction();
        }
    }
};

constexpr vtable_t _5 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        if (bits.N == 0) {
            state.advance_one_instruction();
            if (state._regs[bits.X] == state._regs[bits.Y]) {
                state.advance_one_instruction();
            }
        }
        else {
            state._done = true;
        }
    }
};

constexpr vtable_t _6 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        state._regs[bits.X] = static_cast<std::uint8_t>(bits.NN);
        state.advance_one_instruction();
    },
};

constexpr vtable_t _7 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        state._regs[bits.X] = static_cast<std::uint8_t>(state._regs[bits.X] + bits.NN);
        state.advance_one_instruction();
    },
};

constexpr vtable_t _8 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        auto& VX = state._regs[bits.X];
        auto& VY = state._regs[bits.Y];
        auto& VF = state._regs[0xFu];
        switch (bits.N) {
            case 0:
                VX = VY;
                break;
            case 1:
                VX |= VY;
                break;
            case 2:
                VX &= VY;
                break;
            case 3:
                VX ^= VY;
                break;
            case 4: {
                const auto sum = static_cast<unsigned>(VX) + static_cast<unsigned>(VY);
                VF = static_cast<std::uint8_t>(sum > 0xFFu ? 1u : 0u);
                VX = static_cast<std::uint8_t>(sum);
                break;
            }
            case 5: {
                VF = static_cast<std::uint8_t>(VX >= VY ? 1u : 0u);
                VX = static_cast<std::uint8_t>(VX - VY);
                break;
            }
            case 7: {
                VF = static_cast<std::uint8_t>(VY >= VX ? 1u : 0u);
                VX = static_cast<std::uint8_t>(VY - VX);
                break;
            }
            case 6:
                // SHR Vx {, Vy}: Vx = Vy >> 1, VF = least-significant bit of Vy.
                VF = static_cast<std::uint8_t>(VY & 1u);
                VX = static_cast<std::uint8_t>(VY >> 1);
                break;
            case 0xE:
                // SHL Vx {, Vy}: Vx = Vy << 1, VF = most-significant bit of Vy.
                VF = static_cast<std::uint8_t>((VY >> 7) & 1u);
                VX = static_cast<std::uint8_t>(VY << 1);
                break;
            default:
                state._done = true;
        }
        state.advance_one_instruction();
    }
};

constexpr vtable_t _9 = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        if (bits.N == 0) {
            state.advance_one_instruction();
            if (state._regs[bits.X] != state._regs[bits.Y]) {
                state.advance_one_instruction();
            }
        }
        else {
            state._done = true;
        }
    }
};

constexpr vtable_t _A = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        state._index = static_cast<std::uint16_t>(bits.NNN & 0xFFFu);
        state.advance_one_instruction();
    },
};

constexpr vtable_t _B = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        state._pc = static_cast<std::uint16_t>(bits.NNN + state._regs[0]);
    }
};

constexpr vtable_t _C = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<unsigned int> dist(0, 255);
        state._regs[bits.X] = static_cast<std::uint8_t>(dist(gen) & bits.NN);
        state.advance_one_instruction();
    }
};

constexpr vtable_t _D = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        const std::uint8_t VX = state._regs[bits.X];
        const std::uint8_t VY = state._regs[bits.Y];
        auto& VF = state._regs[0xFu];
        VF = 0;

        // Coordinates wrap around the screen (X mod 64, Y mod 32) and sprite
        // data is never read past the end of the 4 KiB memory.
        for (std::uint8_t row = 0; row < bits.N; ++row) {
            const int addr = static_cast<int>(state._index) + static_cast<int>(row);
            if (addr >= 4096) {
                continue;
            }
            const std::uint8_t sprite_row = state._memory[addr];
            const std::uint16_t y = static_cast<std::uint16_t>((VY + row) % 32u);
            for (std::uint8_t bit = 0; bit < 8; ++bit) {
                const std::uint16_t x = static_cast<std::uint16_t>((VX + bit) % 64u);
                const std::uint8_t pixel = static_cast<std::uint8_t>((sprite_row >> (7 - bit)) & 1u);
                VF |= state._display->exchange_pixel(x, y, pixel);
            }
        }
        state.advance_one_instruction();
    },
};

constexpr vtable_t _E = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        state.advance_one_instruction();
        const auto VX = state._regs[bits.X];
        if (bits.NN == 0x9E) {
            if (state._display->is_pressed(VX)) {
                state.advance_one_instruction();
            }
        }
        else if (bits.NN == 0xA1) {
            if (!state._display->is_pressed(VX)) {
                state.advance_one_instruction();
            }
        }
        else {
            state._done = true;
        }
    }
};

constexpr vtable_t _F = {
    .execute = +[](State& state, const instr_t& instr) -> void {
        const auto bits = deconstruct(instr);
        auto& VX = state._regs[bits.X];
        switch (bits.NN) {
            case 0x07:
                state.advance_one_instruction();
                VX = state._delay_timer;
                break;
            case 0x15:
                state.advance_one_instruction();
                state._delay_timer = VX;
                break;
            case 0x18:
                state.advance_one_instruction();
                state._sound_timer = VX;
                break;
            case 0x1E:
                state.advance_one_instruction();
                state._index = static_cast<std::uint16_t>(
                    (static_cast<unsigned>(state._index) + static_cast<unsigned>(VX)) & 0xFFFu);
                break;
            case 0x0A: {
                // LD Vx, K: wait for a key press, then store the *index* of the key.
                const auto& keys = state._display->get_keys();
                const auto it = std::ranges::find(keys, std::uint8_t{1});
                if (it != std::end(keys)) {
                    VX = static_cast<std::uint8_t>(std::distance(std::begin(keys), it));
                    state.advance_one_instruction();
                }
                break; }
            case 0x29: {
                state.advance_one_instruction();
                const auto character = deconstruct(VX).NN;
                state._index = static_cast<std::uint16_t>(character * 5u);
                break; }
            case 0x33: {
                state.advance_one_instruction();
                state._memory[state._index] = VX / 100;
                state._memory[state._index + 1] = (VX / 10) % 10;
                state._memory[state._index + 2] = VX % 10;
                break; }
            case 0x55:
                state.advance_one_instruction();
                for (std::size_t i = 0; i <= bits.X && (state._index + i) < 4096; ++i) {
                    state._memory[state._index + i] = state._regs[i];
                }
                break;
            case 0x65:
                state.advance_one_instruction();
                for (std::size_t i = 0; i <= bits.X && (state._index + i) < 4096; ++i) {
                    state._regs[i] = state._memory[state._index + i];
                }
                break;
            default:
                state._done = true;
        }
    }
};

inline auto& make_opcode_arr() noexcept {
    static auto ret = std::array<vtable_t, 16> {_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _A, _B, _C, _D, _E, _F};
    return ret;
}

void execute_instruction(State&, const instr_t&) noexcept;

}  // namespace CHIP8

#endif  // CHIP8_OPCODES_H
