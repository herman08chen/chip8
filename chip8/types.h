//
// Created by herman on 10/20/25.
//

#ifndef CHIP8_TYPES_H
#define CHIP8_TYPES_H
#include <cstdint>

namespace CHIP8 {
    using instr_t = std::uint16_t;
    using pc_t = instr_t*;
    using ptr_t = std::uint16_t*;
}

#endif //CHIP8_TYPES_H