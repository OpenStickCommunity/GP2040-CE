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

## Performance

One of the highest priorities of GP2040 is low input latency. Why bother building a custom controller if it's just a laggy, input-missing mess?

Input latency is tested using the methodology outlined at [WydD's inputlag.science website](https://inputlag.science/controller/methodology), using the default 1000 Hz (1 ms) polling rate in the firmware.

| Mode | Poll Rate | Min | Max | Avg | Stdev | % on time | %1f skip | %2f skip |
| - | - | - | - | - | - | - | - | - |
| All | 1 ms | 0.56 ms | 1.58 ms | 0.89 ms | 0.25 ms | 95.71% | 4.29% | 0% |

## Mappings

### Button Mappings

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

### Pin Mappings

The stock pin definitions for a pin-compatible Pico board are:

![Raspberry Pi Pico Default Pin Mapping](assets/pico-pin-mapping.png)

## Development

The project is built using the PlatformIO VS Code plugin along with the [Wiz-IO Raspberry Pi Pico](https://github.com/Wiz-IO/wizio-pico) platform package, using the baremetal (Pico SDK) configuration. There is an external dependency on the [MPG](https://github.com/FeralAI/MPG) C++ gamepad library for handling input state, providing extra features like Left/Right stick emulation and SOCD cleaning, and converting the generic gamepad state to the appropriate USB report.

There are two simple options for building GP2040 for your board. You can either edit an existing board definition, or create your own and configure PlatformIO to build it.

### Existing Configuration

Once you have the project loaded into PlatformIO, edit the `config/Pico/BoardConfig.h` file to map your GPIO pins. Then from the VS Code status bar, use the PlatformIO environment selector to choose `env:raspberry-pi-pico`.

### New Configuration

* Create a file named `BoardConfig.h` in `configs/[BoardNameHere]` with your pin configuration and options, for example: `configs/MyAwesomeGamepad/BoardConfig.h`. A `BoardConfig.h` file can be as basic as some pin definitions:

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
  ```

* Add a new environment to the `platformio.ini`
  * Copy from existing environment and update/rename options accordingly.
  * Update the `build_flags` parameter for the config folder, should look like `-I configs/Pico/` or similar.
  * If you're not using a Pico or bare RP2040, check the `include/pico/config_autogen.h` file to see if there is a define for your board. If so, add or update the `-D BOARD_...` option in `build_flags`. The `env:pimoroni-pico-lipo` config is an example of usage.

You can now use the VS Code status bar to select your new PlatformIO environment target.

### LED Configuration

If your board has WS2812 (or similar) LEDs, they can be configured in the `BoardConfig.h` file:

| Name             | Description                  | Required? |
| ---------------- | ---------------------------- | --------- |
| BOARD_LEDS_PIN   | Data PIN for your LED strand | Yes       |
| LED_BRIGHTNESS_MAX | Max brightness value, `uint8_t` 0-255 | Yes |
| LED_BRIGHTNESS_STEPS | The number of brightness steps when using the up/down hotkey | Yes |
| LEDS_RAINBOW_CYCLE_TIME | For "RAINBOW," this sets how long (in ms) it takes to cycle from one color step to the next | No |
| LEDS_CHASE_CYCLE_TIME | For "CHASE," this sets how long (in ms) it takes to move from one pixel to the next | No |
| LEDS_STATIC_COLOR_COLOR | For "STATIC", this sets the static color. This is an `RGB` struct which can be found in `AnimationStation/src/Animation.hpp`. Can be custom or one of these predefined values: `ColorBlack`, `ColorWhite`, `ColorRed`, `ColorOrange`, `ColorYellow`, `ColorLimeGreen`, `ColorGreen`, `ColorSeafoam`, `ColorAqua`, `ColorSkyBlue`, `ColorBlue`, `ColorPurple`, `ColorPink`, `ColorMagenta` | No |

#### Pixels

The `AnimationStation` local library uses the `Pixel` struct to map buttons to LEDs:

```c++
struct Pixel {
  uint8_t index;                  // The pixel index
  uint32_t mask;                  // Used to detect per-pixel lighting
  std::vector<uint8_t> positions; // The actual LED indexes on the chain
}
```

This allows an arbitrary number of LEDs to be grouped and treated as a single LED for theming and animation purposes.

#### LED Example

A full pixel setup in your `BoardConfig.h` file:

```c++
#include <Pixel.h>

#define BOARD_LEDS_PIN 22
#define LEDS_RAINBOW_CYCLE_TIME 100
#define LEDS_CHASE_CYCLE_TIME 50
#define LEDS_STATIC_COLOR_COLOR ColorRed // Could also use: RGB(255, 0, 0)

#define LEDS_DPAD_UP     3
#define LEDS_DPAD_DOWN   1
#define LEDS_DPAD_LEFT   0
#define LEDS_DPAD_RIGHT  2
#define LEDS_BUTTON_01   8
#define LEDS_BUTTON_02   9
#define LEDS_BUTTON_03   4
#define LEDS_BUTTON_04   5
#define LEDS_BUTTON_05   7
#define LEDS_BUTTON_06   6
#define LEDS_BUTTON_07   11
#define LEDS_BUTTON_08   10

const static std::vector<Pixel> pixels =
{
  { .index = LEDS_DPAD_LEFT,  .mask = GAMEPAD_MASK_LEFT,  .positions = { 0 } },
  { .index = LEDS_DPAD_DOWN,  .mask = GAMEPAD_MASK_DOWN,  .positions = { 1 } },
  { .index = LEDS_DPAD_RIGHT, .mask = GAMEPAD_MASK_RIGHT, .positions = { 2 } },
  { .index = LEDS_DPAD_UP,    .mask = GAMEPAD_MASK_UP,    .positions = { 3, 4, 5 } },
  { .index = LEDS_BUTTON_03,  .mask = GAMEPAD_MASK_B3,    .positions = { 6 } },
  { .index = LEDS_BUTTON_04,  .mask = GAMEPAD_MASK_B4,    .positions = { 7 } },
  { .index = LEDS_BUTTON_06,  .mask = GAMEPAD_MASK_R1,    .positions = { 8 } },
  { .index = LEDS_BUTTON_05,  .mask = GAMEPAD_MASK_L1,    .positions = { 9 } },
  { .index = LEDS_BUTTON_01,  .mask = GAMEPAD_MASK_B1,    .positions = { 10, 11 } },
  { .index = LEDS_BUTTON_02,  .mask = GAMEPAD_MASK_B2,    .positions = { 12, 13 } },
  { .index = LEDS_BUTTON_08,  .mask = GAMEPAD_MASK_R2,    .positions = { 14 } },
  { .index = LEDS_BUTTON_07,  .mask = GAMEPAD_MASK_L2,    .positions = { 15 } },
};
```

### Generating Binaries

You should now be able to build or upload the project to your RP2040 board from the Build and Upload status bar icons. You can also open the PlatformIO tab and select the actions to execute for a particular environment. Output folders are defined in the `platformio.ini` file and should default to a path under `.pio/build/${env:NAME}`.

## Usage

Any button references in this documentation will use the `XInput` labels for clarity.

> NOTE: LED modes are only available for custom builds and the OSFRD configuration right now.

### Home Button

If you do not have a dedicated Home button, you can activate it via the **`BACK + START + UP`** button combination.

### Input Modes

To change the input mode, **hold one of the following buttons as the controller is plugged in:**

* **`RS`** for DirectInput/PS3
* **`BACK`** for Nintendo Switch
* **`START`** for XInput

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

## Support

If you would like to discuss features, issues or anything else related to GP2040 please join the [OpenStick GP2040 Discord channel](https://discord.gg/BQAqDj2N).

## Acknowledgements

* Ha Thach's excellent [TinyUSB library](https://github.com/hathach/tinyusb) examples
* fluffymadness's [tinyusb-xinput](https://github.com/fluffymadness/tinyusb-xinput) sample
* Kevin Boone's [blog post on using RP2040 flash memory as emulated EEPROM](https://kevinboone.me/picoflash.html)
