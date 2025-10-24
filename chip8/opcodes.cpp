//
// Created by herman on 10/20/25.
//

#include "opcodes.h"

namespace CHIP8 {
    void execute_instruction(State& state, const instr_t& instr) noexcept {
        const auto bits = deconstruct(instr);
        make_opcode_arr()[bits._].execute(state, instr);
    }
}
