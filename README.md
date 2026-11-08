# CHIP-8 Emulator

A minimal, header-separated CHIP-8 interpreter written in modern C++ (C++23). It
loads a CHIP-8 ROM, executes its instructions, emulates the timers, renders the
64×32 monochrome display, handles the 16-key hex keypad, and plays sound through
SDL2/SDL2pp.

> **Note:** there is no `README.md` in the repository history. This document
> describes the code as it currently exists in `main.cpp`, `chip8/`, and
> `tests/`.

---

## 1. Program workflow

`main()` is the entry point:

1. Initializes SDL (`SDL_INIT_VIDEO | SDL_INIT_AUDIO`).
2. Creates a `CHIP8::State`, loading the ROM file passed as `argv[1]`. The ROM
   is copied into memory starting at address `0x200`; the built-in font sprites
   occupy `0x000`–`0x04F`.
3. Enters the main loop, gated by a `CHIP8::Clock` with a **2 ms period** (500 Hz):
   - Polls SDL events (`SDL_QUIT`, `SDLK_ESCAPE` to quit; `SDL_KEYDOWN`/`SDL_KEYUP`
     to update the keypad).
   - If the machine is not done: executes one instruction, draws the screen,
     updates the timers.
   - If the machine is done: redraws only if the display needs it.
   - Sleeps until the next clock period.

The state machine halts (`_done = true`) on an unknown/invalid opcode or when the
program counter runs past the end of memory.

---

## 2. Architecture

The code is split into small, single-responsibility components:

```
main.cpp
  └─ SDL event loop + CHIP8::Clock (500 Hz)
       └─ CHIP8::State
            ├─ memory[4096], V[16], I, PC, stack, timers
            ├─ CHIP8::IDisplay (abstract)  ← sdl_t (SDL2pp)
            └─ CHIP8::Clock (60 Hz)        → delay / sound timers
                 └─ execute_instruction()
                      └─ opcode vtable [0x0..0xF] (chip8/opcodes.h)
                           └─ per-opcode lambdas
```

| File | Responsibility |
|------|----------------|
| `main.cpp` | Process entry, SDL init, event polling, keypad mapping, main loop. |
| `chip8/Chip8.h` | Umbrella header including `display.h`, `opcodes.h`, `state.h`. |
| `chip8/state.h` / `chip8/state.cpp` | `State`: CPU core, memory, registers, PC, stack, timers, ROM/font loading, instruction fetch+decode dispatch. |
| `chip8/opcodes.h` / `chip8/opcodes.cpp` | `deconstruct()` splits an instruction into fields; a `constexpr` vtable of 16 execute lambdas (one per high nibble `0x0..0xF`); `execute_instruction()` dispatches through it. |
| `chip8/display.h` / `chip8/display.cpp` | `IDisplay` abstract interface (clear, draw, pixels, keys, sound) + `sdl_t` SDL2pp implementation + `make_sdl_display()` factory. |
| `chip8/clock.h` / `chip8/clock.cpp` | `Clock`: steady-clock time point + period; `sleep_until_next_period()` and `invoke_if_can_update()`. |
| `chip8/types.h` | `instr_t` (`uint16_t`), `pc_t`, `ptr_t`. |
| `tests/test_chip8.cpp` | Headless unit tests using a `MockDisplay` (no SDL). |

`State` depends on `IDisplay`, not on SDL directly, so the CPU core can be
compiled and tested without any graphics/audio library.

---

## 3. Core logic

### Memory map
| Range | Contents |
|-------|----------|
| `0x000`–`0x04F` | Font sprites (16 digits × 5 bytes) |
| `0x050`–`0x1FF` | Reserved / unused |
| `0x200`–`0xFFF` | Program ROM and workspace |

The program counter starts at `0x200`. Each instruction is 2 bytes, fetched
big-endian: `(memory[pc] << 8) | memory[pc + 1]`.

### Registers
- `V[0]`–`V[E]`: general-purpose 8-bit registers.
- `V[F]` (`0xF`): carry/borrow/collision flag.
- `I`: 16-bit index/address register.
- `delay_timer`, `sound_timer`: 8-bit timers decremented at 60 Hz.
- `stack`: stores return addresses (`instr_t`).

### Instruction execution
`State::execute_one_instruction()` fetches the instruction at `_pc` and calls
`execute_instruction(*this, instr)`, which looks up the vtable entry for the
high nibble and invokes the corresponding lambda. Most opcodes advance the PC by
2; skip opcodes advance by 2 or 4 depending on the condition.

Key semantics implemented (per the source):

- **`0x00E0`** — clear screen. **`0x00EE`** — return from subroutine (pop PC).
- **`0x1NNN`** — jump to `NNN`. **`0x2NNN`** — call `NNN` (push PC, jump).
- **`0x3XNN` / `0x4XNN`** — skip next instruction if `Vx ==` / `!=` byte.
- **`0x5XY0` / `0x9XY0`** — skip if `Vx ==` / `!=` `Vy`.
- **`0x6XNN`** — `Vx = byte`. **`0x7XNN`** — `Vx += byte` (mod 256).
- **`0x8XY0`** — `Vx = Vy`. **`0x8XY1`** — `Vx \|= Vy`. **`0x8XY2`** — `Vx &= Vy`. **`0x8XY3`** — `Vx ^= Vy`.
- **`0x8XY4`** — `Vx += Vy`; `VF = 1` on carry. **`0x8XY5`** — `Vx -= Vy`; `VF = 1` if no borrow. **`0x8XY7`** — `Vx = Vy - Vx`; `VF = 1` if no borrow.
- **`0x8XY6`** — `Vx = Vy >> 1`; `VF =` LSB of `Vy`. **`0x8XYE`** — `Vx = Vy << 1`; `VF =` MSB of `Vy`.
- **`0xANNN`** — `I = NNN`. **`0xBNNN`** — `PC = NNN + V0`.
- **`0xCXNN`** — `Vx = random() & NN`.
- **`0xDXYN`** — draw `N`-byte sprite at `(Vx % 64, Vy % 32)` with XOR; `VF = 1` if any pixel is erased. Coordinates wrap; sprite reads are bounds-checked against 4 KiB.
- **`0xEX9E`** — skip if key `Vx` pressed. **`0xEXA1`** — skip if key `Vx` not pressed.
- **`0xFX07`** — `Vx = delay_timer`. **`0xFX0A`** — wait for a key press, then `Vx =` key index. **`0xFX15`** — `delay_timer = Vx`. **`0xFX18`** — `sound_timer = Vx`.
- **`0xFX1E`** — `I = (I + Vx) & 0xFFF`. **`0xFX29`** — `I = Vx * 5` (font sprite). **`0xFX33`** — store BCD of `Vx` at `I..I+2`.
- **`0xFX55`** — store `V0..Vx` at `I..` (does **not** modify `I`). **`0xFX65`** — load `V0..Vx` from `I..` (does **not** modify `I`). Writes/reads are clamped to 4 KiB.

Unknown opcodes set `_done = true`.

### Timers
`State::update_timers()` is called once per main-loop iteration but the actual
decrement happens inside `Clock::invoke_if_can_update()` at a **60 Hz** period:
- `delay_timer` decrements when > 0.
- `sound_timer` decrements when > 0; `play_sound()` is called while it is
  nonzero, `stop_sound()` when it reaches zero.

### Display
`sdl_t` creates an SDL2pp window of `1280×640` (20× scale of 64×32), an
accelerated renderer, and an `ARGB8888` static texture. The internal pixel
buffer is `std::array<std::array<uint8_t, 64>, 32>`. `DRW` XORs sprite pixels
into the buffer and sets the "needs draw" flag. `draw()` is throttled by its own
60 Hz clock and uploads the buffer to the texture, then clears/copies/presents.

### Input
The SDL keycode → CHIP-8 key index mapping (in `main.cpp` and
`State::set_key()`):

| SDL key | CHIP-8 key | SDL key | CHIP-8 key |
|---------|------------|---------|------------|
| `1` | `0x1` | `4` | `0xC` |
| `2` | `0x2` | `Q` | `0x4` |
| `3` | `0x3` | `W` | `0x5` |
| `4` | `0xC` | `E` | `0x6` |
| `R` | `0xD` | `A` | `0x7` |
| `S` | `0x8` | `D` | `0xE` |
| `Z` | `0xA` | `X` | `0x0` |
| `C` | `0xB` | `V` | `0xF` |

`SDLK_ESCAPE` and the window close button quit the emulator.

### Sound
`sdl_t::play_sound()` loads `./beep.mp3` lazily (once, inside a `try/catch`)
and plays it on channel 0 via `sdl2::Mixer`; if the file is missing or loading
throws, sound is silently disabled and the emulator keeps running.
`stop_sound()` halts all channels.

---

## 4. Libraries and dependencies

| Dependency | Purpose | Official repository |
|------------|---------|---------------------|
| [SDL2](https://github.com/libsdl-org/SDL) | Cross-platform windowing, input, and audio low-level library. | https://github.com/libsdl-org/SDL |
| [SDL2pp](https://github.com/SDL2pp/SDL2pp) | C++ wrapper around SDL2 used for `Window`, `Renderer`, `Texture`, `Mixer`, and `Chunk`. | https://github.com/SDL2pp/SDL2pp |
| [SDL2_mixer](https://github.com/libsdl-org/SDL_mixer) | Audio mixing; used via `sdl2::Mixer` to play `./beep.mp3`. | https://github.com/libsdl-org/SDL_mixer |

The graphical target (`chip8`) requires SDL2, SDL2pp, and SDL2_mixer development
files. The headless test target (`chip8_test`) requires none of them.

---

## 5. Setup, build, and run

### Install dependencies (Debian/Ubuntu)

```bash
sudo apt-get update
sudo apt-get install -y libsdl2-dev libsdl2-mixer-dev libsdl2pp-dev cmake g++
```

`libsdl2pp-dev` provides the SDL2pp C++ wrapper. If it is unavailable in your
package repository, build SDL2pp from source:
https://github.com/SDL2pp/SDL2pp.

You also need a `beep.mp3` file in the working directory if you want sound;
otherwise sound is silently disabled.

### Configure and build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

CMake builds two targets:
- `chip8_test` — headless unit tests (always built; no SDL required).
- `chip8` — graphical emulator (built only when SDL2pp is found).

### Run the tests

```bash
./build/chip8_test
```

### Run the emulator

```bash
./build/chip8 /path/to/game.ch8
```

Press `Esc` or close the window to quit.

---

## 6. Testing

`tests/test_chip8.cpp` exercises the core with a mock `IDisplay`: opcode field
decomposition, LD/ADD, BCD, store/load registers, skip conditions, arithmetic
flags, SHR/SHL, JP-to-self (must loop, not halt), CALL/RET, sprite drawing and
collision, coordinate wrapping, SKP/SKNP, LD-key index semantics and waiting,
font-sprite addressing, CLS, JP V0,addr, and memory bounds safety. All tests
pass with no SDL dependency.
