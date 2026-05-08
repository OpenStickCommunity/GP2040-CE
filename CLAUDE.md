# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

GP2040-CE is a multi-platform gamepad firmware for the Raspberry Pi Pico (RP2040). This is a DIY fork (currently v0.8.4-DIY). The firmware supports 14+ input modes (XInput, Switch, PS3/4/5, Xbox One, D-Input, Keyboard) with sub-millisecond latency.

## Build Commands

### Firmware (C++)

Prerequisites: Pico SDK 2.2.0, arm-none-eabi-gcc, CMake 3.10+, Python 3, Node.js 20+.

```bash
# Initialize submodules (required once after clone)
git submodule update --init --recursive

# Configure build (default board: Pico)
cmake -B build -DPICO_SDK_PATH=/path/to/pico-sdk

# Configure for a specific board
cmake -B build -DPICO_SDK_PATH=/path/to/pico-sdk -DGP2040_BOARDCONFIG=FlatboxRev5

# Build
cmake --build build

# Output: build/GP2040-CE_<VERSION>_<BOARD>_DIY.uf2
```

Key CMake variables:
- `GP2040_BOARDCONFIG` — board config name (matches a folder in `configs/`), default: `Pico`
- `SKIP_WEBBUILD=TRUE` — skip npm build (use if web files are pre-built)
- `SKIP_SUBMODULES=TRUE` — skip submodule update
- `GP2040_VERSION_OVERRIDE` — override the version string (default: `"v0.8.4-DIY"`)

Windows helper script: `scripts/bump-and-build-pico.ps1` (PowerShell, handles version bumping and build).

### Web Configurator (React + TypeScript)

```bash
cd www

npm run dev          # Start dev server with mocked board data (localhost:8080)
npm run dev-board    # Start dev server connected to a real device at 192.168.7.1
npm run lint         # Run ESLint
npm run format       # Run Prettier
npm run build        # Production build (outputs to www/data/)
npm run analyze      # Bundle size breakdown
```

## Architecture

### Dual-Core Design

The firmware uses both RP2040 cores:
- **Core 0**: GPIO reading, debouncing, USB device stack, input mode drivers, hotkey processing
- **Core 1**: Add-ons (display, LEDs, rotary encoders, etc.), peripheral management

Entry points: `src/gp2040.cpp` (Core 0 `GP2040` class) and `src/gp2040aux.cpp` (Core 1 `GP2040Aux` class).

### Input Pipeline

`Gamepad` class (`src/gamepad.cpp`) reads raw GPIO state → applies SOCD cleaning → produces a `GamepadState` snapshot → active `DriverManager` driver converts state to USB HID report for the target platform.

### Add-on System

`AddonManager` (`src/addons/`) manages modular features. Each add-on implements process hooks: `CORE0_INPUT`, `CORE0_USBREPORT`, `CORE1_ALWAYS`, `CORE1_LOOP`. Add-ons are enabled/disabled via web configurator and persisted in flash config.

### Configuration Storage

`StorageManager` persists Protobuf-encoded config to RP2040 flash (EEPROM emulation via FlashPROM). Schemas are defined in `proto/config.proto` and `proto/enums.proto`. nanopb generates C structs from these during the build.

### Web Configurator Embedding

The web UI is built by Vite into `www/data/`, then `www/makefsdata.js` converts it to `lib/httpd/fsdata.c` which is compiled directly into the firmware binary. The device exposes it over lwIP at `192.168.7.1`.

## Adding a New API Endpoint

1. In `src/webserver.cpp`:
   - Add a `#define API_GET_NEW_ENDPOINT "/api/getNewEndpoint"` at the top
   - Create a backing method `string getNewEndpoint()`
   - Add a handler in `fs_open_custom`
2. Add a mock in `www/server/app.js`
3. Add the client function to `www/src/Services/WebApi.js`
4. Add to the Postman collection at `www/server/docs/GP2040-CE.postman_collection.json`

All endpoints must be under the `/api` path. Naming convention: `API_[GET|SET]_{ENDPOINT_PATH}` for defines, `/api/{[get|set]EndpointPath}` for paths.

## Adding a New Board Configuration

1. Create `configs/NewBoardName/BoardConfig.h` — GPIO pin assignments, LED config, display config, button layouts
2. Create `configs/NewBoardName/CMakeLists.txt` (minimal, see existing configs)
3. Add to the build matrix in `.github/workflows/cmake.yml`
4. Build with `-DGP2040_BOARDCONFIG=NewBoardName`

## Code Style

- **C++17** firmware; follow `.editorconfig`: tabs, 2-space indent size, LF line endings
- **TypeScript/TSX** web UI: single quotes, LF, 2-space indent
- Compiler warnings are treated seriously; avoid `-Wstack-usage=500` violations
- Web UI images: JPG/PNG only — SVG requires lwIP modifications that aren't implemented
- After adding web dependencies, check bundle size with `npm run analyze`
