# GP2040 - Multi-platform Gamepad Firmware for RP2040 microcontrollers

The goal of GP2040 is to provide multi-platform compatibility for RP2040-based game controllers. The current feature set is:

* Support for the following input modes:
  * Nintendo Switch
  * XInput (PC, Android, Raspberry Pi)
  * DirectInput (PC, Mac, PS3)
* Left and Right stick emulation via D-pad inputs
* 3 SOCD cleaning modes - Neutral, Up Priority (a.k.a. Hitbox), Second Input Priority
* Low input latency, with default 1000 Hz (1 ms) polling rate in all modes
* Save options to internal memory
* Per-button LED support

Prebuilt `uf2` files are available in the [Releases](https://github.com/FeralAI/GP2040/releases) section for the following boards:

* [Raspberry Pi Pico](https://github.com/FeralAI/GP2040/tree/main/configs/Pico) and other pin-compatible boards such as the Pimoroni Pico Lipo
* [Pico Fighting Board](https://github.com/FeralAI/GP2040-Config-PicoFightingBoard/tree/ca6659ca678aad4f5fbb2756fdbf6d00782c6cd2)
* [OSFRD](https://github.com/FeralAI/GP2040/tree/main/configs/OSFRD)

Several other working example configurations are located in the [configs](https://github.com/FeralAI/GP2040/tree/main/configs) folder.

## Installation

The instructions will slightly vary based on your device. These instructions are for a Raspberry Pi Pico.

1. Download the latest `GP2040.uf2` file from the [Releases](https://github.com/FeralAI/GP2040/releases) section for your board (e.g. `GP2040-PiPico.uf2` for the Raspberry Pi Pico).
1. Unplug your Pico.
1. Hold the BOOTSEL button on the Pico and plug into your computer. A new removable drive named `RPI-RP2` should appear in your file explorer.
1. Drag and drop the `GP2040.uf2` file into the removable drive. This will flash the board.
1. The board is now running the GP2040 firmware and will appear as a controller on your computer.

## Performance

One of the highest priorities of GP2040 is low input latency. Why bother building a custom controller if it's just a laggy, input-missing mess?

Input latency is tested using the methodology outlined at [WydD's inputlag.science website](https://inputlag.science/controller/methodology), using the default 1000 Hz (1 ms) polling rate in the firmware.

| Mode | Poll Rate | Min | Max | Avg | Stdev | % on time | %1f skip | %2f skip |
| - | - | - | - | - | - | - | - | - |
| All | 1 ms | 0.56 ms | 1.59 ms | 0.87 ms | 0.24 ms | 95.88% | 4.12% | 0% |

## Buttons

GP2040 uses a generic button labeling for gamepad state, which is then converted to the appropriate input type before sending. Here are the mappings of generic buttons to each supported platform/layout:

| GP2040  | XInput | Switch  | PS3          | DirectInput  | Arcade |
| ------- | ------ | ------- | ------------ | ------------ | ------ |
| B1      | A      | B       | Cross        | 2            | K1     |
| B2      | B      | A       | Circle       | 3            | K2     |
| B3      | X      | Y       | Square       | 1            | P1     |
| B4      | Y      | X       | Triangle     | 4            | P2     |
| L1      | LB     | L       | L1           | 5            | P4     |
| R1      | RB     | R       | R1           | 6            | P3     |
| L2      | LT     | ZL      | L2           | 7            | K4     |
| R2      | RT     | ZR      | R2           | 8            | K3     |
| S1      | Back   | Minus   | Select       | 9            | Coin   |
| S2      | Start  | Plus    | Start        | 10           | Start  |
| L3      | LS     | LS      | L3           | 11           | LS     |
| R3      | RS     | RS      | R3           | 12           | RS     |
| A1      | Guide  | Home    | -            | 13           | -      |
| A2      | -      | Capture | -            | 14           | -      |

## Usage

Any button references in this documentation will use the `XInput` labels for clarity.

> NOTE: LED modes are only available for custom builds and the OSFRD configuration right now.

### Home Button

If you do not have a dedicated Home button, you can activate it via the **`BACK + START + UP`** button combination.

### Input Modes

To change the input mode, **hold one of the following buttons as the controller is plugged in (Arcade mapping in parentheses):**

* **`X (P1)`** for DirectInput/PS3
* **`A (K1)`** for Nintendo Switch
* **`B (K2)`** for XInput

Input mode is saved across power cycles.

### D-Pad Modes

You can switch between the 3 modes for the D-Pad **while the controller is in use by pressing one of the following combinations:**

* **`BACK + START + DOWN`** - D-Pad
* **`BACK + START + LEFT`** - Emulate Left Analog stick
* **`BACK + START + RIGHT`** - Emulate Right Analog stick

D-Pad mode is saved across power cycles.

### SOCD Modes

Simultaneous Opposite Cardinal Direction (SOCD) cleaning will ensure the controller doesn't send invalid directional inputs to the computer/console, like Left + Right at the same time. There are 3 modes to choose from **while the controller is in use by pressing one of the following combinations:**

* **`LS + RS + UP`** - **Up Priority mode**: Up + Down = Up, Left + Right = Neutral (Hitbox behavior)
* **`LS + RS + DOWN`** - **Neutral mode**: Up + Down = Neutral, Left + Right = Neutral
* **`LS + RS + LEFT`** - **Last Input Priority (Last Win)**: Hold Up then hold Down = Down, then release and re-press Up = Up. Applies to both axes.

SOCD mode is saved across power cycles.

### LED Brightness

You can increase brightness with `BACK + START + Y` and decrease brightness with `BACK + START + B`.

### LED Modes

Swap between LED modes using the `BACK + START + A` or `BACK + START + X`. The following modes are available (pics coming eventually):

* Off
* Static Color
* Rainbow Cycle
* Rainbow Chase
* Static Rainbow
* Super Famicom
* Xbox
* Neo Geo Classic
* Neo Geo Curved
* Neo Geo Modern
* Six Button Fighter
* Six Button Fighter+
* Guilty Gear Type-A
* Guilty Gear Type-D

## Development

The project is built using the PlatformIO VS Code plugin along with the [Wiz-IO Raspberry Pi Pico](https://github.com/Wiz-IO/wizio-pico) platform package, using the baremetal (Pico SDK) configuration. There is an external dependency on the [MPG](https://github.com/FeralAI/MPG) C++ gamepad library for handling input state, providing extra features like Left/Right stick emulation and SOCD cleaning, and converting the generic gamepad state to the appropriate USB report.

There are two simple options for building GP2040 for your board. You can either edit an existing board definition, or create your own and configure PlatformIO to build it.

### Board Configuration

Configurations for each type of board/build are contained in separate folders inside `configs`.

#### Existing Configuration

Once you have the project loaded into PlatformIO, edit the `config/Pico/BoardConfig.h` file to map your GPIO pins. Then from the VS Code status bar, use the PlatformIO environment selector to choose `env:raspberry-pi-pico`.

#### New Configuration

1. Create a new folder in `configs` for your board, e.g. `configs/NewBoard`.
1. Create `configs/NewBoard/BoardConfig.h` and add your pin configuration and options. A `BoardConfig.h` file can be as basic as some pin definitions:

    ```c++
    // BoardConfig.h

    #define PIN_DPAD_UP     2
    #define PIN_DPAD_DOWN   3
    #define PIN_DPAD_LEFT   4
    #define PIN_DPAD_RIGHT  5
    #define PIN_BUTTON_B1   6
    #define PIN_BUTTON_B2   7
    #define PIN_BUTTON_B3   8
    #define PIN_BUTTON_B4   9
    #define PIN_BUTTON_L1   10
    #define PIN_BUTTON_R1   11
    #define PIN_BUTTON_L2   26
    #define PIN_BUTTON_R2   27
    #define PIN_BUTTON_S1   16
    #define PIN_BUTTON_S2   17
    #define PIN_BUTTON_L3   18
    #define PIN_BUTTON_R3   19
    #define PIN_BUTTON_A1   20
    #define PIN_BUTTON_A2   21

    #define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
    ```

1. Create `configs/NewBoard/env.ini` using the following template:

    ```c++
    [env:new-board]
    upload_port = .pio/build/new-board/
    build_flags =
        ${env.build_flags}
        -I configs/NewBoard/
    ```

    a. If you're not using a Pico or bare RP2040, check the `include/pico/config_autogen.h` file to see if there is a define for your board. If so, add or update the `-D BOARD_...` option in `build_flags`, for example if using the SparkFun Pro Micro RP2040:

    ```c++
    [env:sparkfun-pro-micro]
    upload_port = .pio/build/sparkfun-pro-micro/
    build_flags =
        ${env.build_flags}
        -D BOARD_SPARKFUN_MICRO_RP2040
        -I configs/SparkFunProMicro/
    ```

Select your environment from the VS Code status bar menu. You may need to restart VS Code in order for PlatformIO to pick up on the `env.ini` changes.

### LED Support

If your board has WS2812 (or similar) LEDs, they can be configured in the `BoardConfig.h` file. The `AnimationStation` local library uses the `Pixel` struct to map buttons to LEDs:

```c++
struct Pixel {
  uint8_t index;                  // The pixel index
  uint32_t mask;                  // Used to detect per-pixel lighting
  std::vector<uint8_t> positions; // The actual LED indexes on the chain
}
```

This allows an arbitrary number of LEDs to be grouped and treated as a single LED for theming and animation purposes.

#### LED Options

The following options are available for use in your `BoardConfig.h` file:

| Name             | Description                  | Required? |
| ---------------- | ---------------------------- | --------- |
| **BOARD_LEDS_PIN** | Data PIN for your LED strand | Yes       |
| **LED_LAYOUT** | Defines the layout of your LED buttons as a `Pixel` matrix. Available options are: `LED_LAYOUT_ARCADE_BUTTONS`, `LED_LAYOUT_ARCADE_HITBOX`, `LED_LAYOUT_ARCADE_WASD` | Yes |
| **LED_FORMAT** | `LED_FORMAT_GRB` | No, default value `LED_FORMAT_GRB` |
| **LEDS_PER_PIXEL** | 1 | Yes |
| **LED_BRIGHTNESS_MAX** | Max brightness value, `uint8_t` 0-255 | Yes |
| **LED_BRIGHTNESS_STEPS** | The number of brightness steps when using the up/down hotkey | Yes |
| **LEDS_RAINBOW_CYCLE_TIME** | For `RAINBOW`, this sets how long (in ms) it takes to cycle from one color step to the next | No |
| **LEDS_CHASE_CYCLE_TIME** | For `CHASE`, this sets how long (in ms) it takes to move from one pixel to the next | No |
| **LEDS_STATIC_COLOR_COLOR** | For `STATIC`, this sets the static color. This is an `RGB` struct which can be found in `AnimationStation/src/Animation.hpp`. Can be custom or one of these predefined values: `ColorBlack`, `ColorWhite`, `ColorRed`, `ColorOrange`, `ColorYellow`, `ColorLimeGreen`, `ColorGreen`, `ColorSeafoam`, `ColorAqua`, `ColorSkyBlue`, `ColorBlue`, `ColorPurple`, `ColorPink`, `ColorMagenta` | No |

#### LED Example

A full pixel setup in your `BoardConfig.h` file:

```c++
#include "enums.h"
#include "NeoPico.hpp"
#include "Animation.hpp"

#define BOARD_LEDS_PIN 22

#define LED_BRIGHTNESS_MAXIMUM 100
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LED_LAYOUT LED_LAYOUT_ARCADE_HITBOX
#define LEDS_PER_PIXEL 2

#define LEDS_RAINBOW_CYCLE_TIME 100
#define LEDS_CHASE_CYCLE_TIME 50
#define LEDS_STATIC_COLOR_COLOR ColorRed // Could also use: RGB(255, 0, 0)

#define LEDS_DPAD_LEFT   0
#define LEDS_DPAD_DOWN   1
#define LEDS_DPAD_RIGHT  2
#define LEDS_DPAD_UP     3
#define LEDS_BUTTON_B3   4
#define LEDS_BUTTON_B4   5
#define LEDS_BUTTON_R1   6
#define LEDS_BUTTON_L1   7
#define LEDS_BUTTON_B1   8
#define LEDS_BUTTON_B2   9
#define LEDS_BUTTON_R2   10
#define LEDS_BUTTON_L2   11
```

### Generating Binaries

You should now be able to build or upload the project to your RP2040 board from the Build and Upload status bar icons. You can also open the PlatformIO tab and select the actions to execute for a particular environment. Output folders are defined in the `platformio.ini` file and should default to a path under `.pio/build/${env:NAME}`.

## Support

If you would like to discuss features, issues or anything else related to GP2040 please join the [OpenStick GP2040 Discord channel](https://discord.gg/ABQafnty).

## Acknowledgements

* Ha Thach's excellent [TinyUSB library](https://github.com/hathach/tinyusb) examples
* fluffymadness's [tinyusb-xinput](https://github.com/fluffymadness/tinyusb-xinput) sample
* Kevin Boone's [blog post on using RP2040 flash memory as emulated EEPROM](https://kevinboone.me/picoflash.html)
