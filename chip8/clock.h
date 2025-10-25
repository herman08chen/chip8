//
// Created by herman on 10/24/25.
//

#ifndef CHIP8_CLOCK_H
#define CHIP8_CLOCK_H

#include <chrono>
#include <functional>
using namespace std::chrono_literals;
namespace CHIP8 {
    struct Clock {
        consteval static auto hz(std::size_t HZ) noexcept {
            return std::chrono::nanoseconds(1'000'000'000/HZ);
        }

        std::chrono::time_point<std::chrono::steady_clock> time;
        decltype(time)::duration period;

        void sleep_until_next_period() noexcept;
        template<class F>
        void invoke_if_can_update(F&& f) noexcept {
            if (std::chrono::steady_clock::now() >= time + period) {
                time += period;
                std::invoke(f);
            }
        }
    };
}


#endif //CHIP8_CLOCK_H