# GP2040 Development

GP2040 is written in C++ and set up as a [PlatformIO](https://platformio.org/) project, using the [Wiz-IO Raspberry Pi Pico](https://github.com/Wiz-IO/wizio-pico) platform package in the `baremetal` (Pico SDK) configuration.

## Environment Setup

The recommended setup is to develop using the [PlatformIO IDE](https://platformio.org/platformio-ide), which is an extension to the excellent [Visual Studio Code (VS Code)](https://code.visualstudio.com/) editor. If a dedicated IDE for embedded development isn't your thing, you can easily build the project using the [PlatformIO CLI](https://platformio.org/install/cli) instead. This section will cover using the PlatformIO IDE.

1. Use Git to clone the [GP2040 repository](https://github.com/FeralAI/GP2040.git), or [download the latest version](https://github.com/FeralAI/GP2040/archive/refs/heads/main.zip) and extract it.
1. Follow the [installation instructions for the PlatformIO IDE](https://platformio.org/install/ide?install=vscode).
1. Open VS Code and you should be greeted with the PlatformIO Home screen.
1. Select the PlatformIO tab in the activity bar (bug icon), then go to `PIO Home > Platforms`.
1. On the Platforms tab click the `Advanced Installation` button, then type `https://github.com/Wiz-IO/wizio-pico` and click `Install`.
1. Open the `GP2040` (`GP2040-main` if from zip) folder in VS Code and it should automatically get picked up as a Platform IO project.
1. Click on the VS Code Explorer tab (or Ctrl+Shift+E) and expand the folders and files in your project.

PlatformIO will download any dependencies not already included with the project.

## Configuration

There are two simple options for building GP2040 for your board. You can either edit an existing board definition, or create your own and configure PlatformIO to build it. Several example configurations are located in the repository **[configs](https://github.com/FeralAI/GP2040/tree/main/configs)** folder. This document will outline setting up a new build configuration.

### Board Configuration Folder

Each subfolder in [`configs`](https://github.com/FeralAI/GP2040/tree/main/configs) contains a separate PlatformIO build configuration, which consists of the following:

| Name | Required? | Description |
| ----------- | --------- | ----------- |
| `BoardConfig.h` | Yes | The configuration file used when building GP2040 for a specific controller/board. Contains initial pin mappings, LED configuration, etc. |
| `env.ini` | Yes | A partial PlatformIO project configuration file which defines the build parameters for this board. All `env.ini` files in subfolders of `configs` will be parsed and selectable when loading the project in the PlatformIO IDE (may require a restart to pick up the new build config).
| `README.md` | No | Provides information related to this board configuration. Not required for the build process, but suggested for pull requests of new board configurations. |
| `assets/` | No | Folder for containing assets included in the `README.md`. Not required for the build process.

### Build Configuration (`env.ini`)

1. Create a new folder in `configs` for your board, e.g. `configs/NewBoard`.
1. Create `configs/NewBoard/env.ini` using the following template:

    ```ini
    [env:new-board]
    upload_port = .pio/build/new-board/
    build_flags =
        ${env.build_flags}
        -I configs/NewBoard/
    ```

    a. If you're not using a Pico or bare RP2040, check the `include/pico/config_autogen.h` file to see if there is a define for your board. If so, add or update the `-D BOARD_...` option in `build_flags`, for example if using the SparkFun Pro Micro RP2040:

    ```ini
    [env:sparkfun-pro-micro]
    upload_port = .pio/build/sparkfun-pro-micro/
    build_flags =
        ${env.build_flags}
        -D BOARD_SPARKFUN_MICRO_RP2040
        -I configs/SparkFunProMicro/
    ```

This will create a new PlatformIO build environment named `new-board`. Select the new environment from the VS Code status bar menu. You may need to restart VS Code in order for PlatformIO to pick up on the `env.ini` changes.

### Board Configuration (`BoardConfig.h`)

The following board options are available in the `BoardConfig.h` file:

| Name             | Description                  | Required? |
| ---------------- | ---------------------------- | --------- |
| **PIN_DPAD_*X***<br>**PIN_BUTTON_*X*** | The GPIO pin for the button. Replace the *`X`* with GP2040 button or D-pad direction. | Yes |
| **DEFAULT_SOCD_MODE** | The default SOCD mode to use, defaults to `SOCD_MODE_NEUTRAL`.<br>Available options are:<br>`SOCD_MODE_NEUTRAL`<br>`SOCD_MODE_UP_PRIORITY`<br>`SOCD_MODE_SECOND_INPUT_PRIORITY` | No |
| **BUTTON_LAYOUT** | The layout of controls/buttons for use with per-button LEDs and external displays.<br>Available options are:<br>`BUTTON_LAYOUT_HITBOX`<br>`BUTTON_LAYOUT_HITBOX`<br>`BUTTON_LAYOUT_WASD` | Yes |

Create `configs/NewBoard/BoardConfig.h` and add your pin configuration and options. An example `BoardConfig.h` file:

```cpp
// BoardConfig.h

#include <GamepadEnums.h>

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
#define BUTTON_LAYOUT BUTTON_LAYOUT_ARCADE
```

#### RGB LEDs

GP2040 supports per-button WS2812 and similar RGB LEDs.

The following RGB LED options are available in the `BoardConfig.h` file:

| Name             | Description                  | Required? |
| ---------------- | ---------------------------- | --------- |
| **BUTTON_LAYOUT** | The layout of controls/buttons for use with per-button LEDs and external displays.<br>Available options are:<br>`BUTTON_LAYOUT_HITBOX`<br>`BUTTON_LAYOUT_HITBOX`<br>`BUTTON_LAYOUT_WASD` | Yes |
| **BOARD_LEDS_PIN** | Data PIN for your LED strand | Yes       |
| **LED_FORMAT** | The color data format for the LED chain.<br>Available options are:<br>`LED_FORMAT_GRB`<br>`LED_FORMAT_RGB`<br>`LED_FORMAT_GRBW`<br>`LED_FORMAT_RGBW` | No, default value `LED_FORMAT_GRB` |
| **LEDS_PER_PIXEL** | The number of LEDs per button. | Yes |
| **LED_BRIGHTNESS_MAX** | Max brightness value, `uint8_t` 0-255. | Yes |
| **LED_BRIGHTNESS_STEPS** | The number of brightness steps when using the up/down hotkey. | Yes |
| **LEDS_DPAD_*X***<br>**LEDS_BUTTON_*X*** | The index of the button on the LED chain. Replace the *`X`* with GP2040 button or D-pad direction. | Yes |

An example RGB LED setup in the `BoardConfig.h` file:

```cpp
// BoardConfig.h

#include "gp2040.h"
#include "NeoPico.hpp"

#define BUTTON_LAYOUT BUTTON_LAYOUT_HITBOX

#define BOARD_LEDS_PIN 22

#define LED_BRIGHTNESS_MAXIMUM 100
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LEDS_PER_PIXEL 2

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

#### Player LEDs

GP2040 supports PWM and RGB player LEDs (PLEDs) and can be configured in the `BoardConfig.h` file.

> NOTE: RGB PLEDs require [RGB LEDs](#rgb-leds) to be configured.

The following PLED options are available in the `BoardConfig.h` file:

| Name             | Description                  | Required? |
| ---------------- | ---------------------------- | --------- |
| **PLED_TYPE** | Configures the type of PLEDs.<br>Available options are: `PLED_TYPE_PWM`, `PLED_TYPE_RGB` | Yes |
| **PLED1_PIN** | (PWM) The GPIO pin for PLED #1.<br>(RGB) The index of PLED #1 on the LED chain. | Yes |
| **PLED2_PIN** | (PWM) The GPIO pin for PLED #2.<br>(RGB) The index of PLED #2 on the LED chain. | Yes |
| **PLED3_PIN** | (PWM) The GPIO pin for PLED #3.<br>(RGB) The index of PLED #3 on the LED chain. | Yes |
| **PLED4_PIN** | (PWM) The GPIO pin for PLED #4.<br>(RGB) The index of PLED #4 on the LED chain. | Yes |

An example PLED setup in the `BoardConfig.h` file:

```cpp
// BoardConfig.h

#include "PlayerLEDs.h"

#define PLED_TYPE PLED_TYPE_RGB
#define PLED1_PIN 12
#define PLED2_PIN 13
#define PLED3_PIN 14
#define PLED4_PIN 15
```

#### I2C Displays

GP2040 supports 128x64 monochrome displays that run on the SSD1306, SH1106 or SH1107 drivers. The following options are available for displays:

| Name | Description | Required? |
| - | - | - |
| **BUTTON_LAYOUT** | The layout of controls/buttons for use with per-button LEDs and external displays.<br>Available options are:<br>`BUTTON_LAYOUT_HITBOX`<br>`BUTTON_LAYOUT_HITBOX`<br>`BUTTON_LAYOUT_WASD` | Yes |
| **HAS_I2C_DISPLAY** | Flag to indicate the controller contains an I2C display module. | No |
| **DISPLAY_I2C_ADDR** | The I2C address of the display. | No, defaults to `0x3C` |
| **I2C_SDA_PIN** | The GPIO pin for the I2C SDA line. | If `HAS_I2C_DISPLAY` is enabled |
| **I2C_SCL_PIN** | The GPIO pin for the I2C SCL line. | If `HAS_I2C_DISPLAY` is enabled |
| **I2C_BLOCK** | The I2C block on the Pico. Refer to the [Pico Pinout Diagram](https://datasheets.raspberrypi.com/pico/Pico-R3-A4-Pinout.pdf) to identify which block is in use based on the SDA and SCL pins being used.<br>Available options are:<br>`i2c0`<br>`i2c1` | No, defaults to `i2c0` |
| **I2C_SPEED** | The speed of the I2C bus. `100000` is standard mode, while `400000` is used for fast mode communication. Higher values may be used but will require testing the device for support. | No, defaults to `400000` |
| **DISPLAY_FLIP** | Flag to flip the rendered display output. Set to `1` to enable. | No, defaults to `0` |
| **DISPLAY_INVERT** | Flag to invert the rendered display output. Set to `1` to enable. | No, defaults to `0` |

An example I2C display setup in the `BoardConfig.h` file:

```cpp
#define BUTTON_LAYOUT BUTTON_LAYOUT_WASD
#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1
#define I2C_BLOCK i2c0
#define I2C_SPEED 800000
```

## Building

You should now be able to build or upload the project to your RP2040 board from the Build and Upload status bar icons. You can also open the PlatformIO tab and select the actions to execute for a particular environment. Output folders are defined in the `platformio.ini` file and should default to a path under `.pio/build/${env:NAME}`.
