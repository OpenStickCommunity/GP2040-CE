# GP2040-CE Turbo Enhancement Project

## Quick Summary
**Goal**: Add hardware-based turbo controls to GP2040-CE using an I2C GPIO expander.
- ✅ Analog turbo speed dial (potentiometer, 2-30 shots/sec)
- ✅ 8 per-button turbo switches (B1-B4, L1-L2, R1-R2)
- **Strategy**: Use I2C GPIO expander (MCP23017 breakout board) for switches. This makes the feature available to any board with an I2C bus.
- **GPIO Required**: 1 ADC (speed dial) + I2C bus (shared with display)
- **Hardware**: MCP23017 breakout board + potentiometer + 8 switches
- **Target Board**: Any Pico-based board (like Sky 2040 v2) using the `Pico` BoardConfig.

## Project Location
- **Fork**: `/Users/fwong/Documents/github/wongfei2009/GP2040-CE`
- **Upstream**: OpenStickCommunity/GP2040-CE

---

## Hardware Implementation

### Why I2C Expander?
**Problem**: Most boards have limited free GPIOs after connecting buttons, displays, and LEDs.
- We need 8 switches + 1 speed dial = 9 **additional** pins.
- An I2C expander avoids the need for a board with many spare GPIOs.

**Solution**: MCP23017 I2C GPIO Expander
- Provides 16 additional GPIO pins via I2C.
- Shares the I2C bus with a display (SDA/SCL pins).
- Configurable I2C address (default 0x20).
- Built-in pull-up resistors.
- Widely available and inexpensive (~$1-2).

### Component List

#### 1. MCP23017 I2C GPIO Expander Breakout Board
**Specs**:
- 16 GPIO pins (we need 8 for switches).
- I2C interface (3.3V compatible).
- Configurable address: 0x20-0x27.
- **I2C Address**: Use **0x20** (default) to avoid conflict with common displays (0x3C).

#### 2. Turbo Speed Dial
- 10kΩ linear potentiometer.

#### 3. Turbo Switches
- 8× SPST toggle switches OR 1× DIP-8 switch array.

#### 4. Wiring Components
- Breadboard and jumper wires.

---

## Software Implementation

### Phase 1: Enable Turbo Hardware in BoardConfig
**File**: `configs/Pico/BoardConfig.h` (or your target board's config)

Add the following definitions to your `BoardConfig.h` to enable the hardware turbo features.

```cpp
// Turbo speed dial (ADC input)
// Set to -1 to disable. Valid pins are 26, 27, 28.
#define PIN_SHMUP_DIAL 26

// I2C Turbo switches configuration
#define TURBO_I2C_SWITCHES_ENABLED 1
#define TURBO_I2C_SDA_PIN 0
#define TURBO_I2C_SCL_PIN 1
#define TURBO_I2C_BLOCK i2c0
#define TURBO_I2C_SPEED 400000  // 400kHz
#define TURBO_I2C_ADDR 0x20     // MCP23017 address
```

**Action Required**: Verify that the `TURBO_I2C_SDA_PIN` and `TURBO_I2C_SCL_PIN` match the I2C pins used by your display or other I2C devices.

### Phase 2: Add MCP23017 Driver
**Files**: `src/addons/turbo.cpp`, `headers/addons/turbo.h`, `lib/mcp23017/mcp23017.h`, `lib/mcp23017/mcp23017.cpp`

This phase involves creating a generic driver for the MCP23017 and integrating it into the turbo addon.

#### Create MCP23017 Library
**File**: `lib/mcp23017/mcp23017.h`
```cpp
#ifndef MCP23017_H
#define MCP23017_H

#include "hardware/i2c.h"
#include <stdint.h>

// MCP23017 Register Addresses
#define MCP23017_IODIRA   0x00  // I/O direction A
#define MCP23017_IODIRB   0x01  // I/O direction B
#define MCP23017_GPPUA    0x0C  // Pull-up A
#define MCP23017_GPPUB    0x0D  // Pull-up B
#define MCP23017_GPIOA    0x12  // Port A
#define MCP23017_GPIOB    0x13  // Port B

class MCP23017 {
public:
    MCP23017(i2c_inst_t* i2c, uint8_t addr);
    
    bool init();                    // Initialize chip
    void setPinMode(uint8_t pin, bool input);
    void setPullup(uint8_t pin, bool enable);
    bool readPin(uint8_t pin);
    uint16_t readAll();             // Read all 16 pins
    
private:
    i2c_inst_t* i2c_;
    uint8_t addr_;
    
    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
};

#endif // MCP23017_H
```

**File**: `lib/mcp23017/mcp23017.cpp`
```cpp
#include "mcp23017.h"

MCP23017::MCP23017(i2c_inst_t* i2c, uint8_t addr) 
    : i2c_(i2c), addr_(addr) {}

bool MCP23017::init() {
    // Configure all pins on Port A as inputs with pull-ups
    writeRegister(MCP23017_IODIRA, 0xFF);  // All inputs
    writeRegister(MCP23017_GPPUA, 0xFF);   // All pull-ups enabled
    
    // Port B unused (all inputs, no pull-ups)
    writeRegister(MCP23017_IODIRB, 0xFF);
    writeRegister(MCP23017_GPPUB, 0x00);
    
    return true;
}

void MCP23017::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(i2c_, addr_, buf, 2, false);
}

uint8_t MCP23017::readRegister(uint8_t reg) {
    uint8_t value;
    i2c_write_blocking(i2c_, addr_, &reg, 1, true);
    i2c_read_blocking(i2c_, addr_, &value, 1, false);
    return value;
}

uint16_t MCP23017::readAll() {
    uint8_t portA = readRegister(MCP23017_GPIOA);
    uint8_t portB = readRegister(MCP23017_GPIOB);
    return (portB << 8) | portA;
}

bool MCP23017::readPin(uint8_t pin) {
    if (pin < 8) {
        uint8_t portA = readRegister(MCP23017_GPIOA);
        return (portA & (1 << pin)) != 0;
    } else {
        uint8_t portB = readRegister(MCP23017_GPIOB);
        return (portB & (1 << (pin - 8))) != 0;
    }
}
```

#### Modify Turbo Add-on for I2C Switches
**File**: `headers/addons/turbo.h`
```cpp
// Add to existing TurboInput class

#ifdef TURBO_I2C_SWITCHES_ENABLED
#include "mcp23017.h"
#endif

class TurboInput : public GPAddon {
public:
    // ... existing members ...
    
#ifdef TURBO_I2C_SWITCHES_ENABLED
    MCP23017* mcp_;  // I2C expander instance
#endif
};
```

**File**: `src/addons/turbo.cpp`
```cpp
#include "addons/turbo.h"

void TurboInput::setup() {
    // ... existing setup code ...
    
    #ifdef TURBO_I2C_SWITCHES_ENABLED
    // Initialize MCP23017 on shared I2C bus
    // Note: I2C should already be initialized by display addon or another service
    mcp_ = new MCP23017(TURBO_I2C_BLOCK, TURBO_I2C_ADDR);
    
    if (!mcp_->init()) {
        // Handle initialization error
        delete mcp_;
        mcp_ = nullptr;
    }
    #endif
}

void TurboInput::process() {
    #ifdef TURBO_I2C_SWITCHES_ENABLED
    if (mcp_) {
        // Read all 8 switches from MCP23017 Port A (active-low)
        uint8_t switchStates = mcp_->readRegister(MCP23017_GPIOA);
        
        // Build turbo mask from switch states
        uint16_t hardwareTurboMask = 0;
        
        // Map MCP23017 pins to button masks (active-low)
        if (!(switchStates & 0x01)) hardwareTurboMask |= GAMEPAD_MASK_B1;  // GPA0
        if (!(switchStates & 0x02)) hardwareTurboMask |= GAMEPAD_MASK_B2;  // GPA1
        if (!(switchStates & 0x04)) hardwareTurboMask |= GAMEPAD_MASK_B3;  // GPA2
        if (!(switchStates & 0x08)) hardwareTurboMask |= GAMEPAD_MASK_B4;  // GPA3
        if (!(switchStates & 0x10)) hardwareTurboMask |= GAMEPAD_MASK_L1;  // GPA4
        if (!(switchStates & 0x20)) hardwareTurboMask |= GAMEPAD_MASK_R1;  // GPA5
        if (!(switchStates & 0x40)) hardwareTurboMask |= GAMEPAD_MASK_L2;  // GPA6
        if (!(switchStates & 0x80)) hardwareTurboMask |= GAMEPAD_MASK_R2;  // GPA7
        
        // Hardware switches override software turbo settings
        turboButtonsMask = hardwareTurboMask;
    }
    #endif
    
    // ... existing turbo processing code ...
}
```

---

## Implementation Roadmap

### Phase 1: MCP23017 Library ✅ COMPLETE
**Tasks**:
1. ✅ Create `lib/mcp23017/` directory.
2. ✅ Implement `mcp23017.h` (header file).
3. ✅ Implement `mcp23017.cpp` (driver code).
4. ✅ Add library to `CMakeLists.txt`.
5. ✅ Test I2C communication and switch reading.

**Success Criteria**:
- ✅ MCP23017 is detected on the I2C bus.
- ✅ Can read switch states from Port A.
- ✅ No conflicts with other I2C devices (like a display).

### Phase 2: Hardware Assembly ⏱️ 2-3 hours
**Tasks**:
1. Acquire components (MCP23017, switches, potentiometer).
2. Connect MCP23017 to the Pico's I2C bus (VCC, GND, SDA, SCL).
3. Connect 8 switches to MCP23017 GPA0-GPA7 pins.
4. Connect potentiometer to an ADC pin (e.g., GPIO 26).
5. Update `configs/Pico/BoardConfig.h` with correct pin assignments.

**Success Criteria**:
- ⏳ MCP23017 is detected at address 0x20.
- ⏳ Potentiometer provides a variable reading on the ADC pin.
- ⏳ All switches toggle correctly.

### Phase 3: Turbo Addon Integration ✅ COMPLETE
**Tasks**:
1. ✅ Modify `headers/addons/turbo.h` and `src/addons/turbo.cpp` for I2C switches.
2. ✅ Ensure I2C bus is initialized before the turbo addon tries to use it.
3. ✅ Rebuild and flash firmware with `GP2040_BOARDCONFIG=Pico`.
4. ✅ Add real-time web configurator display for I2C switches.
5. ⏳ Test each switch and the speed dial.

**Success Criteria**:
- ⏳ Each switch correctly enables/disables turbo for the assigned button.
- ⏳ Speed dial adjusts turbo rate smoothly.
- ⏳ No interference with other I2C devices.
- ✅ Web configurator displays real-time switch states.

### Phase 4: Testing & Validation ⏱️ 2-3 hours
**Tasks**:
1. Test individual and multiple turbo switches.
2. Test speed dial adjustment.
3. Stress test I2C bus by toggling switches rapidly.
4. Test across different platforms (PC, Switch, etc.).

**Success Criteria**:
- ⏳ All tests pass without errors.
- ⏳ No I2C bus hangs or firmware crashes.
- ⏳ Display (if present) remains responsive.

---

## Web Configurator Features ✅ IMPLEMENTED

### Real-Time Turbo Diagnostics
The web configurator now displays real-time status for:

1. **Turbo Speed Dial** (existing feature):
   - Live percentage reading (0-100%)
   - Raw ADC value (0-4095)
   - Updates every 500ms

2. **I2C Turbo Switches** (NEW):
   - Live ON/OFF status for all 8 switches:
     - B1, B2, B3, B4 (Face buttons)
     - L1, R1, L2, R2 (Shoulder buttons)
   - Visual badges: Green for ON, Gray for OFF
   - Updates every 500ms
   - Only shown when `TURBO_I2C_SWITCHES_ENABLED` is defined

### Implementation Details

**Backend Changes** (`src/webconfig.cpp`):
- Enhanced `getTurboDiagnostics()` API endpoint
- Reads MCP23017 switch states directly via I2C
- Returns JSON with individual switch states
- Conditionally compiled with `#ifdef TURBO_I2C_SWITCHES_ENABLED`

**Frontend Changes** (`www/src/Addons/Turbo.tsx`):
- Extended state to track all 8 switch positions
- Auto-polling mechanism (500ms interval)
- Bootstrap badge UI for visual feedback
- Responsive grid layout (4 switches per row)

---

## Build & Flash Commands

### Build Firmware
```bash
# Set board configuration
export GP2040_BOARDCONFIG=Pico

# Navigate to project
cd /Users/fwong/Documents/github/wongfei2009/GP2040-CE

# Create and enter build directory
mkdir -p build && cd build

# Configure build
cmake ..

# Build (use all CPU cores)
make -j$(sysctl -n hw.ncpu)

# Output file: GP2040-CE_X.X.X_Pico.uf2
```

---

## Next Immediate Steps

### Ready to Start
1. ✅ **Decision made**: Use MCP23017 breakout board to add hardware turbo.
2. ✅ **Plan updated**: Integrate as a generic feature for any board config.
3. ⏭️ **Next action**: Implement the MCP23017 driver.

### Phase 1 Action Items (Software)
```bash
# 1. Create MCP23017 library directory
mkdir -p lib/mcp23017

# 2. Create and implement mcp23017.h and mcp23017.cpp
#    (Use the code provided in the "Software Implementation" section)

# 3. Add the new library to the build system (edit CMakeLists.txt)

# 4. Modify the Turbo addon (turbo.h, turbo.cpp) to use the new driver

# 5. Edit configs/Pico/BoardConfig.h to enable the feature
#    (Add the #defines for TURBO_I2C_SWITCHES_ENABLED, etc.)

# 6. Test build
export GP2040_BOARDCONFIG=Pico
# (run cmake and make from the build directory)
```

---

## macOS Build Instructions

Building the firmware from source on macOS requires a specific setup to resolve dependency conflicts. The following steps have been verified to produce a successful build.

### Prerequisites

1.  **Xcode Command Line Tools**: Install by running `xcode-select --install` in your terminal.
2.  **Homebrew**: The macOS package manager. If not installed, get it from [brew.sh](https://brew.sh/).
3.  **ARM GCC Toolchain**: Install via Homebrew Cask:
    ```bash
    brew install --cask gcc-arm-embedded
    ```

### Build Steps

The build process involves manually downloading the correct Pico SDK version before running CMake and Make.

1.  **Clone the Correct Pico SDK Version**

    The project requires Pico SDK version **2.1.1**. Clone it into the project's root directory:
    ```bash
    git clone --branch 2.1.1 https://github.com/raspberrypi/pico-sdk.git pico-sdk-2.1.1
    ```

2.  **Initialize SDK Submodules**

    Navigate into the newly created SDK directory and initialize its submodules (which include the compatible Mbed TLS library):
    ```bash
    cd pico-sdk-2.1.1
    git submodule update --init --recursive
    cd ..
    ```

3.  **Configure and Build the Firmware**

    This single command chain will clean any previous build, set the required environment variables, configure the project with CMake, and compile it with Make.

    Run this from the root of the `GP2040-CE` project directory:
    ```bash
    rm -rf build && \
    mkdir build && \
    cd build && \
    export SDKROOT=$(xcrun --sdk macosx --show-sdk-path) && \
    PICO_SDK_PATH=$(pwd)/../pico-sdk-2.1.1 GP2040_BOARDCONFIG=Pico cmake -DPICO_SDK_FETCH_FROM_GIT=OFF .. && \
    make -j$(sysctl -n hw.ncpu)
    ```

    - `rm -rf build`: Cleans up any previous build attempts.
    - `export SDKROOT=...`: Explicitly sets the path to the macOS SDK, which fixes issues with compiling host-side Python tools.
    - `PICO_SDK_PATH=...`: Points the build system to our manually downloaded SDK.
    - `GP2040_BOARDCONFIG=Pico`: Sets the target board. Change `Pico` to your desired board if different.
    - `PICO_SDK_FETCH_FROM_GIT=OFF`: Prevents the build system from automatically downloading another SDK.
    - `make -j...`: Compiles the firmware using all available CPU cores.

    Upon completion, the firmware file (e.g., `GP2040-CE_0.0.0_Pico.uf2`) will be located in the `build` directory.
