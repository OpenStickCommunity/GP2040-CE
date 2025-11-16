# Sky 2040 v2 GP2040-CE Turbo Enhancement Project

## Quick Summary
**Goal**: Add hardware-based turbo controls to Sky 2040 v2 joystick
- ✅ Analog turbo speed dial (potentiometer, 2-30 shots/sec)
- ✅ 8 per-button turbo switches (B1-B4, L1-L2, R1-R2)
- **Strategy**: Use I2C GPIO expander (MCP23017 breakout board) for switches
- **GPIO Required**: 1 ADC (speed dial) + I2C bus (shared with display)
- **Hardware**: MCP23017 breakout board + potentiometer + 8 switches
- **Target Board**: Sky 2040 Version 2

## Project Location
- **Fork**: `/Users/fwong/Documents/github/wongfei2009/GP2040-CE`
- **Upstream**: OpenStickCommunity/GP2040-CE

---

## Sky 2040 v2 Configuration

### Base Configuration
- **Board Type**: Raspberry Pi Pico form factor
- **Board Version**: Sky 2040 Version 2
- **Note**: Actual GPIO mappings will be documented once physical board is available for testing

### Planned Features
- I2C Display (shared bus with MCP23017)
- RGB LEDs
- Turbo speed dial (ADC input)
- 8 per-button turbo switches via MCP23017 I2C expander

---

## Hardware Implementation

### Why I2C Expander?
**Problem**: Sky 2040 v2 has sufficient GPIOs on the RP2040, but they're already assigned to existing features (buttons, display, RGB LEDs, etc.)
- We need 8 switches + 1 speed dial = 9 **additional** pins
- No unassigned GPIOs available for these new turbo features

**Solution**: MCP23017 I2C GPIO Expander
- Provides 16 additional GPIO pins via I2C
- Shares I2C bus with display (no extra pins needed)
- Configurable I2C address (default 0x20)
- Built-in pull-up resistors
- Widely available and inexpensive (~$1-2)

### Component List

#### 1. MCP23017 I2C GPIO Expander Breakout Board
**Specs**:
- 16 GPIO pins (we need 8)
- I2C interface (3.3V compatible)
- Configurable address: 0x20-0x27 (via solder jumpers or switches)
- Internal pull-ups available
- Pre-wired power, I2C, and address configuration

**Recommended Products**:
- Adafruit MCP23017 I2C GPIO Expander Breakout (#732)
- SparkFun Qwiic GPIO MCP23017
- Generic MCP23017 breakout boards on Amazon/AliExpress

**I2C Address**: Use **0x20** (default) to avoid conflict with display (0x3C)

#### 2. Turbo Speed Dial
- 10kΩ linear potentiometer (rotary or slide)

#### 3. Turbo Switches
- 8× SPST toggle switches OR 1× DIP-8 switch array

#### 4. Wiring Components
- Breadboard (for prototyping)
- Jumper wires (male-to-female recommended for breakout boards)

---

## Software Implementation

### Phase 1: Create Sky2040 v2 BoardConfig
**File**: `configs/Sky2040v2/BoardConfig.h`

**Note**: GPIO pin mappings will be determined once physical Sky 2040 v2 board is available for testing. The following are placeholders based on anticipated differences from standard Pico.

**Expected Changes from Pico**:
```cpp
// Board identification
#define BOARD_CONFIG_LABEL "Sky2040v2"

// GPIO mappings - TO BE VERIFIED with physical Sky 2040 v2 board
// (Sky 2040 v2 may have different pin layout than standard Pico)

// Turbo speed dial (ADC input - pin TBD with physical board)
#define PIN_SHMUP_DIAL 26  // To be verified

// I2C Turbo switches configuration
#define TURBO_I2C_SWITCHES_ENABLED 1
#define TURBO_I2C_SDA_PIN 0     // To be verified - shared with display
#define TURBO_I2C_SCL_PIN 1     // To be verified - shared with display
#define TURBO_I2C_BLOCK i2c0    // Same I2C block as display
#define TURBO_I2C_SPEED 400000  // 400kHz (same as display)
#define TURBO_I2C_ADDR 0x20     // MCP23017 address

// RGB LEDs (pin TBD with physical board)
#define BOARD_LEDS_PIN 28  // To be verified
```

**Action Required**: Once Sky 2040 v2 board arrives, verify all GPIO assignments and update accordingly.

### Phase 2: Add MCP23017 Driver
**Files**: `src/addons/turbo.cpp`, `headers/addons/turbo.h`, `lib/mcp23017/mcp23017.h`

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
    // Note: I2C already initialized by display addon
    mcp_ = new MCP23017(TURBO_I2C_BLOCK, TURBO_I2C_ADDR);
    
    if (!mcp_->init()) {
        // Handle initialization error
        // Could set flag to disable I2C switches
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
    
    // Continue with existing turbo flicker logic
    // Speed dial already supported via pinShmupDial (GPIO 26)
    // ... existing turbo processing code ...
}
```

---

## Implementation Roadmap

### Phase 1: BoardConfig Setup ⏱️ 1 hour
**Tasks**:
1. Create `configs/Sky2040v2/` directory
2. Copy `configs/Pico/BoardConfig.h` → `configs/Sky2040v2/BoardConfig.h`
3. Apply initial changes for Sky 2040 v2:
   - Set `BOARD_CONFIG_LABEL "Sky2040v2"`
   - Add I2C turbo switch definitions (placeholder pins)
   - Add turbo speed dial definition (placeholder pin)
   - Add RGB LED definition (placeholder pin)
   - **IMPORTANT**: Mark all GPIO assignments as "TO BE VERIFIED" until physical board arrives
4. Create `configs/Sky2040v2/README.md` noting that pin mappings are pending physical board verification
5. Test build:
   ```bash
   export GP2040_BOARDCONFIG=Sky2040v2
   cd build && cmake .. && make -j$(sysctl -n hw.ncpu)
   ```

**Post-Hardware Arrival**:
- Map all buttons and verify GPIO assignments with physical Sky 2040 v2 board
- Update BoardConfig.h with verified pin mappings
- Test each button, display, RGB LEDs, and peripherals
- Document actual pinout in README.md

**Success Criteria**:
- ✅ Builds without errors (with placeholder pins)
- ✅ Ready to update with real pin mappings when board arrives

### Phase 2: MCP23017 Library ⏱️ 2-3 hours
**Tasks**:
1. Create `lib/mcp23017/` directory
2. Implement `mcp23017.h` (header file)
3. Implement `mcp23017.cpp` (driver code)
4. Add to CMakeLists.txt
5. Test I2C communication:
   - Verify address detection (0x20)
   - Test register read/write
   - Verify no conflict with display (0x3C)
6. Create test program to read switches

**Success Criteria**:
- ✅ MCP23017 detected on I2C bus
- ✅ Can read Port A register
- ✅ Display continues working
- ✅ No I2C bus conflicts

### Phase 3: Hardware Assembly ⏱️ 2-3 hours
**Tasks**:
1. Acquire components:
   - **MCP23017 breakout board** (Adafruit, SparkFun, or generic)
   - 8× SPST toggle switches OR DIP-8 switch array
   - 10kΩ linear potentiometer
   - Breadboard + jumper wires
   - Sky 2040 v2 board
2. **First Step with Sky 2040 v2**: Map and document all GPIO pins
   - Identify I2C pins (SDA/SCL) for display and MCP23017
   - Identify available ADC pin for potentiometer
   - Identify RGB LED data pin (if present)
   - Identify all button pins and verify their functions
   - Update `configs/Sky2040v2/BoardConfig.h` with verified mappings
3. Connect MCP23017 breakout to Sky 2040 v2:
   - VCC/VDD → 3.3V
   - GND → GND
   - SDA → Verified I2C SDA pin
   - SCL → Verified I2C SCL pin
   - Set I2C address to 0x20 (usually default)
4. Connect 8 switches to MCP23017 GPA0-GPA7 pins (one side to pin, other to GND)
5. Connect potentiometer wiper to verified ADC pin (other pins to GND and 3.3V)
6. Test connections with multimeter

**Success Criteria**:
- ✅ Sky 2040 v2 GPIO pinout fully documented
- ✅ MCP23017 detected on I2C bus at 0x20
- ✅ I2C communication works
- ✅ All switches toggle cleanly
- ✅ Potentiometer reads correctly
- ✅ Display still works on Sky 2040 v2 (no I2C conflict)

### Phase 4: Turbo Addon Integration ⏱️ 4-6 hours
**Tasks**:
1. Modify `headers/addons/turbo.h`:
   - Include MCP23017 header
   - Add MCP23017* member to TurboInput class
   - Add I2C configuration constants
2. Modify `src/addons/turbo.cpp`:
   - Initialize MCP23017 in `setup()`
   - Read switches in `process()`
   - Map switch states to button masks
   - Override turboButtonsMask with hardware state
3. Handle I2C bus sharing:
   - Verify display addon initializes I2C first
   - Ensure MCP23017 doesn't re-initialize bus
   - Add mutex if concurrent access needed (unlikely)
4. Rebuild and flash firmware
5. Test each switch independently
6. Test multiple switches simultaneously
7. Test speed dial adjustment
8. Verify no display interference

**Success Criteria**:
- ✅ Each switch controls correct button turbo
- ✅ All 8 switches work independently
- ✅ Speed dial adjusts turbo rate smoothly
- ✅ Display continues working perfectly
- ✅ No I2C bus conflicts or glitches
- ✅ RGB LEDs still functional

### Phase 5: Testing & Validation ⏱️ 2-3 hours
**Hardware Tests**:
1. Individual button turbo (8 tests)
2. Multiple button turbo simultaneously (all 8)
3. Speed dial adjustment while turbo active
4. Long-duration stress test (30+ minutes)
5. Power cycle persistence
6. I2C bus stress test (rapid switch toggling)

**Cross-Platform Tests**:
1. PC (XInput mode)
2. Nintendo Switch
3. PS4/PS5 (if dongle available)
4. Steam Deck (if available)

**Edge Cases**:
1. All 8 switches ON simultaneously
2. Rapid switch toggling (stress I2C)
3. Speed dial at min/max extremes
4. Turbo button press during hardware turbo
5. Display updates during switch reads

**Success Criteria**:
- ✅ All tests pass without errors
- ✅ No I2C bus hangs or timeouts
- ✅ Display remains responsive
- ✅ Consistent behavior across platforms
- ✅ No firmware crashes

### Phase 6: Optimization & Polish ⏱️ 1-2 hours
**Tasks**:
1. Measure I2C read latency
2. Optimize switch polling frequency
3. Add error recovery for I2C failures
4. Implement switch debouncing if needed
5. Add OLED display indicators for active turbo
6. Test and tune turbo speed range

**Success Criteria**:
- ✅ I2C latency <1ms
- ✅ Total polling overhead <1% CPU
- ✅ Robust error handling
- ✅ Clean switch response

### Phase 7: Documentation ⏱️ 1-2 hours
**Tasks**:
1. Update `configs/Sky2040v2/README.md`:
   - Complete pin mapping table
   - I2C expander wiring diagram
   - Turbo switch wiring
   - Speed dial wiring
   - I2C address configuration
2. Document MCP23017 library API
3. Create build guide for users
4. Add troubleshooting section
5. Take photos of completed build
6. Update CLAUDE.md with final status

**Total Time**: 12-18 hours for complete implementation (reduced due to breakout board simplicity)

---

## Build & Flash Commands

### Build Firmware
```bash
# Set board configuration
export GP2040_BOARDCONFIG=Sky2040v2

# Navigate to project
cd /Users/fwong/Documents/github/wongfei2009/GP2040-CE

# Create and enter build directory
mkdir -p build && cd build

# Configure build
cmake ..

# Build (use all CPU cores)
make -j$(sysctl -n hw.ncpu)

# Output file: GP2040-CE_X.X.X_Sky2040v2.uf2
```

### Flash to Device
```bash
# Method 1: Hold BOOTSEL while plugging in USB
# Method 2: Short RUN pin to GND twice quickly

# Copy firmware (device appears as RPI-RP2)
cp GP2040-CE_*_Sky2040v2.uf2 /Volumes/RPI-RP2/

# Wait for automatic reboot
```

---

## Success Criteria Checklist

### Must Have ✅
- [ ] Sky2040v2 BoardConfig builds successfully
- [ ] All 18 original buttons work correctly
- [ ] MCP23017 communicates via I2C
- [ ] Speed dial adjusts turbo speed (2-30 shots/sec)
- [ ] All 8 turbo switches control their respective buttons
- [ ] Hardware switches override software turbo
- [ ] Display continues working (no I2C conflict)
- [ ] RGB LEDs still functional
- [ ] No input latency added (<1ms overhead)
- [ ] Firmware stable in all input modes

### Nice to Have 🎯
- [ ] OLED shows active turbo indicators
- [ ] Web configurator shows switch states
- [ ] I2C error recovery implemented
- [ ] Settings persist across power cycles
- [ ] Upstream PR contribution

---

## Next Immediate Steps

### Ready to Start
1. ✅ **Decision made**: MCP23017 breakout board approach
2. ✅ **Documentation complete**: Full implementation plan for Sky 2040 v2
3. ✅ **Board identified**: Sky 2040 Version 2
4. ⏭️ **Next action**: Order MCP23017 breakout board + components

### Phase 1 Action Items (Can Start Now - Before Hardware Arrives)
```bash
# 1. Create directory for Sky 2040 v2 config
mkdir -p configs/Sky2040v2

# 2. Copy base config
cp configs/Pico/BoardConfig.h configs/Sky2040v2/BoardConfig.h

# 3. Edit the file (apply placeholder Sky 2040 v2 config)
#    Mark GPIO pins as "TO BE VERIFIED" with physical board

# 4. Create MCP23017 library
mkdir -p lib/mcp23017
# (implement mcp23017.h and mcp23017.cpp)

# 5. Test build for Sky 2040 v2 (with placeholder config)
export GP2040_BOARDCONFIG=Sky2040v2
cd build && cmake .. && make -j$(sysctl -n hw.ncpu)
```

**After Sky 2040 v2 Board Arrives**:
1. Flash placeholder firmware to test basic functionality
2. Map all GPIO pins with multimeter and testing
3. Update BoardConfig.h with verified pin assignments
4. Rebuild and verify all functions work correctly
5. Proceed to Phase 3 (Hardware Assembly)

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
