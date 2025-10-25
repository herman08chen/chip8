//
// Created by herman on 10/20/25.
//

#ifndef CHIP8_OPCODES_H
#define CHIP8_OPCODES_H

#include "types.h"
#include <array>
#include <bitset>
#include <cstring>
#include <random>
#include <ranges>
#include <utility>

#include "display.h"
#include "state.h"

namespace CHIP8 {
    struct vtable_t {
        void(*execute)(State&, const instr_t&);
    };
    constexpr auto deconstruct(const instr_t &instr) {
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
            [[maybe_unused]]  auto bits = deconstruct(instr);
            if (instr == 0x00E0) {
                state._sdl.clear();
                state.advance_one_instruction();
            }
            else if (instr == 0x00EE) {
                state._pc = state._stack.top();
                state._stack.pop();
                state.advance_one_instruction();
            }
            else {
                state._done = true;
            }
        },
    };
    constexpr vtable_t _1 = {
        .execute = +[](State& state, const instr_t& instr) -> void {
            const auto bits = deconstruct(instr);
            if (state._pc != bits.NNN)
                state._pc = bits.NNN;
            else
                state._done = true;
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
                if (bits.X == bits.Y) {
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
            state._regs[bits.X] = bits.NN;
            state.advance_one_instruction();
        },
    };
    constexpr vtable_t _7 = {
        .execute = +[](State& state, const instr_t& instr) -> void {
            const auto bits = deconstruct(instr);
            state._regs[bits.X] += bits.NN;
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
                case 4:
                    if (256 - VX < VY) {
                        VX += VY;
                        VF = 1;
                    }
                    else {
                        VX += VY;
                        VF = 0;
                    }
                    break;
                case 5:
                    if (VX >= VY) {
                        VX = VX - VY;
                        VF = 1;
                    }
                    else {
                        VX = VX - VY;
                        VF = 0;
                    }

                    break;
                case 7:
                    if (VY >= VX) {
                        VX = VY - VX;
                        VF = 1;
                    }
                    else {
                        VX = VY - VX;
                        VF = 0;
                    }

                    break;
                case 6:
                    if (VX & 1u) {
                        VX = VY;
                        VX = VX >> 1;
                        VF = 1;
                    }
                    else {
                        VX = VY;
                        VX = VX >> 1;
                        VF = 0;
                    }

                    break;
                case 0xE:
                    if (VX >> 7) {
                        VX = VY;
                        VX = VX << 1;
                        VF = 1;
                    }
                    else {
                        VX = VY;
                        VX = VX << 1;
                        VF = 0;
                    }
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
            state._index = bits.NNN;
            state.advance_one_instruction();
        },
    };
    constexpr vtable_t _B = {
        .execute = +[](State& state, const instr_t& instr) -> void {
            const auto bits = deconstruct(instr);
            state._pc = bits.NNN + state._regs[0];
        }
    };
    constexpr vtable_t _C = {
        .execute = +[](State& state, const instr_t& instr) -> void {
            const auto bits = deconstruct(instr);
            static std::random_device rd;
            static std::mt19937 gen(rd());
            static std::uniform_int_distribution<unsigned int> dist(0, 255);
            state._regs[bits.X] = dist(gen) & bits.NN;
            state.advance_one_instruction();
        }
    };
    constexpr vtable_t _D = {
        .execute = +[](State& state, const instr_t& instr) -> void {
            const auto bits = deconstruct(instr);
            const std::span<std::uint8_t> sprite(state._memory + state._index, bits.N);
            const auto VX = state._regs[bits.X] % 64;
            const auto VY = state._regs[bits.Y] % 32;
            auto& VF = state._regs[0xFu];
            VF = 0;

            for (auto&& [row, y] : std::ranges::views::zip(sprite, std::ranges::iota_view{VY, 32} | std::ranges::views::take(bits.N))) {
                for (auto&& [bit, x] : std::ranges::iota_view(VX, 64) | std::ranges::views::take(8) | std::ranges::views::enumerate) {
                    const std::uint8_t pixel = std::bitset<8>(row)[7 - bit];
                    VF |= state._sdl.exchange_pixel(x, y, pixel);
                }
            }
            state.advance_one_instruction();
        },
    };
    constexpr vtable_t _E = {
        .execute = +[](State& state, const instr_t& instr) -> void {
            auto&& bits = deconstruct(instr);
            state.advance_one_instruction();
            auto& VX = state._regs[bits.X];
            if (bits.NN == 0x9E) {
                if (state._sdl.is_pressed(VX))
                    state.advance_one_instruction();
            }
            else if (bits.NN == 0xA1) {
                if (!state._sdl.is_pressed(VX))
                    state.advance_one_instruction();
            }
            else {
                state._done = true;
            }
        }
    };
    constexpr vtable_t _F = {
        .execute = +[](State& state, const instr_t& instr) -> void {
            auto&& bits = deconstruct(instr);

            auto&& VX = state._regs[bits.X];
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
                    state._index += VX;
                    break;
                case 0x0A: {
                    const auto key = std::ranges::find(state._sdl.get_keys(), 1);
                    if (key != std::end(state._sdl.get_keys())) {
                        VX = *key;
                        state.advance_one_instruction();
                    }
                    break; }
                case 0x29: {
                    state.advance_one_instruction();
                    const auto character = deconstruct(VX).NN;
                    state._index = character * 5;
                    break; }
                case 0x33: {
                    state.advance_one_instruction();
                    state._memory[state._index] = VX / 100;
                    state._memory[state._index + 1] = (VX / 10) % 10;
                    state._memory[state._index + 2] = VX % 10;
                    break; }
                case 0x55:
                    state.advance_one_instruction();
                    for (auto&& i : std::ranges::iota_view<std::uint8_t, std::size_t>{0, bits.X + 1}) {
                        state._memory[state._index + i] = state._regs[i];
                    }
                    break;
                case 0x65:
                    state.advance_one_instruction();
                    for (auto&& i : std::ranges::iota_view<std::uint8_t, std::size_t>{0, bits.X + 1}) {
                        state._regs[i] = state._memory[state._index + i];
                    }
                    break;
                default:
                    state._done = true;
            }
        }
    };


    constexpr auto& make_opcode_arr() noexcept {
        static auto ret = std::array<vtable_t, 16> {_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _A, _B, _C, _D, _E, _F};
        return ret;
    }

    void execute_instruction(State&, const instr_t&) noexcept;
}



#endif //CHIP8_OPCODES_H