//
// Created by herman on 10/24/25.
//

#include "clock.h"

#include <thread>

namespace CHIP8 {
    void Clock::sleep_until_next_period() noexcept {
        std::this_thread::sleep_until(time += period);
    }
}
