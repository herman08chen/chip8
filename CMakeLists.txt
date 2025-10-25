cmake_minimum_required(VERSION 4.0)
project(chip8)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "a9e1cf81-9932-4810-974b-6eccaf14e457")

set(CMAKE_CXX_STANDARD_REQUIRED OFF)
set(CMAKE_CXX_MODULE_STD 1)

add_executable(chip8 main.cpp
        chip8/Chip8.h
        chip8/opcodes.cpp
        chip8/opcodes.h
        chip8/display.cpp
        chip8/display.h
        chip8/types.h
        chip8/state.cpp
        chip8/state.h
        chip8/clock.cpp
        chip8/clock.h)

set(SDL2PP_WITH_IMAGE ON) # if you need SDL_image support
set(SDL2PP_WITH_MIXER ON) # if you need SDL_mixer support
set(SDL2PP_WITH_TTF ON) # if you need SDL_ttf support

find_package(SDL2pp REQUIRED)
target_link_libraries(chip8 SDL2pp::SDL2pp)
target_compile_options(chip8 PUBLIC -fmodules-ts -O3 -Wall -Wextra)