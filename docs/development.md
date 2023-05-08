# GP2040-CE Development

GP2040-CE is written in C++ and set up as a standard Pico SDK project.
See [Getting Started with the Raspberry Pi Pico](https://rptl.io/pico-get-started).

## Environment Setup

Most of this will be parroting the above linked PDF from the Raspberry Pi Foundation.

### Windows Setup

1. Install the latest ARM GNU Toolcahin
    - [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
1. Install latest version of CMake
    - [CMake](https://cmake.org/download/)
1. Install Visual Studio Build tools, or the full Visual Studio IDE
    - Make sure to select the Desktop development with C++ workload
    - Select the latest Windows 10 or Windows 11 SDK from the Individual Components
    - [Visual Studio Build tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
    - [Visual Studio Community Edition](https://visualstudio.microsoft.com/downloads/#visual-studio-community-2022)
1. Install Python 3.10
    - At the end of the installation, there is an option to disable max file path length. You want to select this.
    - [Python](https://www.python.org/downloads/windows/)
1. Install Visual Studio Code - Optional
     - [Visual Studio Code](https://code.visualstudio.com/)
1. Install git
     - Set default editor to anything other than VIM, such as Visual Studio Code
1. Install NodeJS and NPM
    - [Download and install NodeJS](https://nodejs.org/en/download)
1. Clone the Pico SDK to your local computer

    ```bash
    git clone https://github.com/raspberrypi/pico-sdk.git
    cd pico-sdk
    git submodule update --init
    cd ..
    ```

1. Clone GP2040-CE to your local computer

    ```bash
    git clone https://github.com/OpenStickCommunity/GP2040-CE.git
    cd GP2040-CE
    git submodule update --init
    ```

### Linux Setup (Ubuntu)

This setup assumes an understanding of Linux terminal usage.

1. Get the SDK

    ```bash
    cd ~/
    mkdir pico
    cd pico
    git clone https://github.com/raspberrypi/pico-sdk.git --branch master
    cd pico-sdk
    git submodule update --init
    cd ..
    ```

1. Install the toolchain

    ```bash
    sudo apt update
    sudo apt install cmake gcc-arm-none-eabi libnewlib-arm-none-eabi build-essential
    ```

    - May additionally need to install `libstdc++-arm-none-eabi-newlib`

1. Install NodeJS and NPM

    ```bash
    curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash - &&\
    sudo apt-get install -y nodejs
    ```

1. Get GP2040-CE

    ```bash
    git clone https://github.com/OpenStickCommunity/GP2040-CE.git
    cd GP2040-CE
    git submodule update --init
    ```

### Linux Setup (Raspberry Pi)

This setup script requires approximately 2.5GB of disk space on your SD card.

1. Download the setup script

    ```bash
    wget https://raw.githubusercontent.com/raspberrypi/pico-setup/master/pico_setup.sh
    ```

1. Make script executable and Run it.

    ```bash
    chmod +x pico_setup.sh
    ```

1. Install NodeJS and NPM

    ```bash
    curl -fsSL https://deb.nodesource.com/setup_18.x | sudo -E bash - &&\
    sudo apt-get install -y nodejs
    ```

1. Reboot your Pi

    ```bash
    sudo reboot
    ```

1. Get GP2040-CE

    ```bash
    git clone https://github.com/OpenStickCommunity/GP2040-CE.git
    cd GP2040-CE
    git submodule update --init
    ```

## Building

### Environment Variables

A number of new environment variables have been setup to control parts of the build flow.

| Name | Default | Description |
| ----------- | --------- | ----------- |
|GP2040_BOARDCONFIG |Pico |The boards.h config file to use for the build.|
|SKIP_WEBBUILD|FALSE|Determines whether the web configurator is built during the cmake configuration step.|
|SKIP_SUBMODULES|FALSE|Determines whether the submodule init command is run automatically during the cmake configuration step.|

#### SDK Variables

There are a few SDK variables we take advantage of for our builds.

| Name | Default | Description |
| ----------- | --------- | ----------- |
|PICO_BOARD|pico| This is the embeded board that the RP2040 chip is part of. By default, it assumes the Pico. This variable would match the `<boardname.h>` file in the board's configs folder.|

### Windows

Start in the GP2040-CE folder. **From a Developer Powershell or Developer Command Command Prompt**:

?> A new Powershell or Command Prompt session will be required after setting an environment variable.

1. Ensure you have the `PICO_SDK_PATH` environment variable set to the path to your pico-sdk folder.
2. (optional) Set the `GP2040_BOARDCONFIG` environment variable to the folder name for your board configuration.
    - Default value is `Pico`

3. Create a build directory, configure the build, and execute the build.

    ```bash
    mkdir build
    cd build
    cmake -G "NMake Makefiles" ..
    nmake
    ```

4. Your UF2 file should be in the build directory.

### Linux

Start in the GP2040-CE folder

1. Ensure you have the `PICO_SDK_PATH` environment variable set to the path to your pico-sdk folder.
1. (optional) Set the `GP2040_BOARDCONFIG` environment variable to the folder name for your board configuration.
    - Default value is `Pico`
1. Create a build directory, configure the build, and execute the build.

    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

1. Your UF2 file should be in the build directory.

## Configuration

?> We're moving away from compile time configuration, in favor of runtime configuration.

There are two simple options for building GP2040-CE for your board. You can either edit an existing board definition, or create your own. Several example configurations are located in the repository **[configs](https://github.com/OpenStickFoundation/GP2040-CE/tree/main/configs)** folder. This document will outline setting up a new build configuration.

### Board Configuration Folder

Each subfolder in [`configs`](https://github.com/OpenStickFoundation/GP2040-CE/tree/main/configs) contains a separate board configuration, which consists of the following:

| Name | Required? | Description |
| ----------- | --------- | ----------- |
| `BoardConfig.h` | Yes | The configuration file used when building GP2040-CE for a specific controller/board. Contains initial pin mappings, LED configuration, etc. |
| `README.md` | No | Provides information related to this board configuration. Not required for the build process, but suggested for pull requests of new board configurations. |
| `assets/` | No | Folder for containing assets included in the `README.md`. Not required for the build process.
|'<boardname.h>'|No| Board definition file, named after the board itself, used by the Pico SDK for configuring board specific SDK features. [Pico Example](https://github.com/raspberrypi/pico-sdk/blob/master/src/boards/include/boards/pico.h)

### Board Configuration (`BoardConfig.h`)

The following board options are available in the `BoardConfig.h` file:

| Name             | Description                  | Required? |
| ---------------- | ---------------------------- | --------- |
| **PIN_DPAD_*X***<br>**PIN_BUTTON_*X*** | The GPIO pin for the button. Replace the *`X`* with GP2040-CE button or D-pad direction. | Yes |
| **DEFAULT_SOCD_MODE** | The default SOCD mode to use, defaults to `SOCD_MODE_NEUTRAL`.<br>Available options are:<br>`SOCD_MODE_NEUTRAL`<br>`SOCD_MODE_UP_PRIORITY`<br>`SOCD_MODE_SECOND_INPUT_PRIORITY` | No |
| **BUTTON_LAYOUT** | The layout of controls/buttons for use with per-button LEDs and external displays.<br>Available options are:<br>`BUTTON_LAYOUT_STICKLESS`<br>`BUTTON_LAYOUT_WASD` | Yes |

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
#define PIN_BUTTON_TURBO -1
#define PIN_SLIDER_LS    -1
#define PIN_SLIDER_RS    -1

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define BUTTON_LAYOUT BUTTON_LAYOUT_ARCADE
```

#### RGB LEDs

GP2040-CE supports per-button WS2812 and similar RGB LEDs.

The following RGB LED options are available in the `BoardConfig.h` file:

| Name             | Description                  | Required? |
| ---------------- | ---------------------------- | --------- |
| **BUTTON_LAYOUT** | The layout of controls/buttons for use with per-button LEDs and external displays.<br>Available options are:<br>`BUTTON_LAYOUT_STICKLESS`<br>`BUTTON_LAYOUT_WASD` | Yes |
| **BOARD_LEDS_PIN** | Data PIN for your LED strand | Yes       |
| **LED_FORMAT** | The color data format for the LED chain.<br>Available options are:<br>`LED_FORMAT_GRB`<br>`LED_FORMAT_RGB`<br>`LED_FORMAT_GRBW`<br>`LED_FORMAT_RGBW` | No, default value `LED_FORMAT_GRB` |
| **LEDS_PER_PIXEL** | The number of LEDs per button. | Yes |
| **LED_BRIGHTNESS_MAXIMUM** | Max brightness value, `uint8_t` 0-255. | Yes |
| **LED_BRIGHTNESS_STEPS** | The number of brightness steps when using the up/down hotkey. | Yes |
| **LEDS_DPAD_*X***<br>**LEDS_BUTTON_*X*** | The index of the button on the LED chain. Replace the *`X`* with GP2040-CE button or D-pad direction. | Yes |
| **LEDS_BASE_ANIMATION_INDEX** | The default LED animation index. | No, defaults to `1` |
| **LEDS_STATIC_COLOR_INDEX** | The default color index for the static color theme  | No, defaults to `2` |
| **LEDS_BUTTON_COLOR_INDEX** | The default color index for the pressed button color | No, defaults to `1` |
| **LEDS_THEME_INDEX** | The default theme index for static themes | No, defaults to `0` |
| **LEDS_RAINBOW_CYCLE_TIME** | The color cycle time for rainbow cycle theme | No, defaults to `40` |
| **LEDS_CHASE_CYCLE_TIME** | The animation speed for the rainbow chase theme | No, defaults to `85` |

An example RGB LED setup in the `BoardConfig.h` file:

```cpp
// BoardConfig.h

#include "gp2040.h"
#include "NeoPico.hpp"

#define BUTTON_LAYOUT BUTTON_LAYOUT_STICKLESS

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

GP2040-CE supports PWM and RGB player LEDs (PLEDs) and can be configured in the `BoardConfig.h` file.

?> RGB PLEDs require [RGB LEDs](#rgb-leds) to be configured.

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

GP2040-CE supports 128x64 monochrome displays that run on the SSD1306, SH1106 or SH1107 drivers. The following options are available for displays:

| Name | Description | Required? |
| - | - | - |
| **BUTTON_LAYOUT** | The layout of controls/buttons for use with per-button LEDs and external displays.<br>Available options are:<br>`BUTTON_LAYOUT_STICKLESS`<br>`BUTTON_LAYOUT_WASD` | Yes |
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
