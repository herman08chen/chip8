cmake_minimum_required(VERSION 3.16)
project(chip8 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Headless unit tests for the CHIP-8 core.  These compile state.cpp and
# opcodes.cpp only (no SDL), so they run without a graphics/audio subsystem.
add_executable(chip8_test
    tests/test_chip8.cpp
    chip8/state.cpp
    chip8/opcodes.cpp
)

target_include_directories(chip8_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(chip8_test PRIVATE -Wall -Wextra -O2)

# The graphical frontend is optional because SDL2pp/SDL2 development files are
# not installed in every environment.
find_package(SDL2pp QUIET)
if (SDL2pp_FOUND)
    add_executable(chip8
        main.cpp
        chip8/Chip8.h
        chip8/opcodes.cpp
        chip8/opcodes.h
        chip8/display.cpp
        chip8/display.h
        chip8/types.h
        chip8/state.cpp
        chip8/state.h
        chip8/clock.cpp
        chip8/clock.h
    )
    target_link_libraries(chip8 PRIVATE SDL2pp::SDL2pp)
    target_compile_options(chip8 PRIVATE -Wall -Wextra -O3)
endif()
