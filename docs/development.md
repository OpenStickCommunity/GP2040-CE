# GP2040-CE Development

GP2040-CE is written in C++ and set up as a standard Pico SDK project.
See [Getting Started with the Raspberry Pi Pico](https://rptl.io/pico-get-started).

## Environment Setup

Most of this will be parroting the above linked PDF from the Raspberry Pi Foundation.

For GP2040-CE development on the Windows platform using the Pico SDK Installer, please skip ahead to the 'Windows Setup (Pico SDK Installer)' section on this page.

<!-- tabs:start -->

### **Windows (Pico SDK)**

### Windows (Pico SDK)


1. Download and install CMake from the [CMake Website](https://github.com/Kitware/CMake/releases/download/v3.27.4/cmake-3.27.4-windows-x86_64.msi).
2. Download and install [pico-setup-windows-x64-standalone](https://github.com/raspberrypi/pico-setup-windows/releases/).
3. Download and install VSCode.
4. Download and install [Windows Terminal](https://github.com/microsoft/terminal/releases/tag/v1.17.11461.0).
5. Open Windows Terminal.
6. Download the GP2040-CE-main repository by running the following commands in the Windows Terminal. This will download the folder to `C:\Users\user\GP2040-CE`.

    ```console
    git clone https://github.com/OpenStickCommunity/GP2040-CE.git
    cd GP2040-CE
    git submodule update --init
    ```

   After installing the Raspberry Pi Pico SDK, you should now have a shortcut to "Pico-Visual Studio Code" (search for it using Windows Search).

7. Open "Pico-Visual Studio Code" via Windows search. This is a shortcut with pre-configured environment variables (this will be explained later).
8. Navigate to the Extensions window by pressing `Ctrl+Shift+X`.
9. Install "CMake Tools" by Microsoft.
10. Open the GP2040-CE folder you downloaded earlier via "File > Open Folder."

### **Windows (Manual)**

### Windows (Manual)


1. Install the latest ARM GNU Toolchain
    - [Arm GNU Toolchain](https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads)
2. Install latest version of CMake
    - [CMake](https://cmake.org/download/)
3. Install Visual Studio Build tools, or the full Visual Studio IDE
    - Make sure to select the Desktop development with C++ workload
    - Select the latest Windows 10 or Windows 11 SDK from the Individual Components
    - [Visual Studio Build tools](https://visualstudio.microsoft.com/downloads/#build-tools-for-visual-studio-2022)
    - [Visual Studio Community Edition](https://visualstudio.microsoft.com/downloads/#visual-studio-community-2022)
4. Install Python 3.10
    - At the end of the installation, there is an option to disable max file path length. You want to select this.
    - [Python](https://www.python.org/downloads/windows/)
5. Install Visual Studio Code - Optional
     - [Visual Studio Code](https://code.visualstudio.com/)
6. Install git
     - Set default editor to anything other than VIM, such as Visual Studio Code
7. Install NodeJS and NPM
    - [Download and install NodeJS](https://nodejs.org/en/download)
8. Clone the Pico SDK to your local computer

    ```console
    git clone https://github.com/raspberrypi/pico-sdk.git
    cd pico-sdk
    git submodule update --init
    cd ..
    ```

9. From a command-prompt, Clone GP2040-CE to your local computer

    ```console
    git clone https://github.com/OpenStickCommunity/GP2040-CE.git
    cd GP2040-CE
    ```

### **Linux**

### Linux


#### Ubuntu

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

#### Raspberry Pi

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

<!-- tabs:end -->

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
|PICO_BOARD|pico| This is the embedded board that the RP2040 chip is part of. By default, it assumes the Pico. This variable would match the `<boardname.h>` file in the board's configs folder.|

<!-- tabs:start -->

### **Windows (Pico SDK)**

### Windows (Pico SDK)

After installing the Raspberry Pi Pico SDK, you should now have a shortcut to "Pico-Visual Studio Code" (search for it using Windows Search). This shortcut should already have everything configured, **allowing you to skip to step 7**. If you're experiencing compilation issues, consider following the manual steps outlined here.

Ensure the `PICO_SDK_PATH` environment variable is set:

1. Search for "Edit environment variables for your account" in Windows.
2. Under "User Variables," click "New...".
3. In the dialog that appears, enter `PICO_SDK_PATH` for the Variable Name.
4. Click "Browse Directory" and navigate to `C:\Program Files\Raspberry Pi\Pico SDK v1.5.1\pico-sdk` for the Variable Value.
5. Create another new variable.
6. Enter `GP2040_BOARDCONFIG` for the Variable Name.
7. Enter `Pico` (or the name of your edited config folder) for the Variable Value.

You can also set the variable within VSCode:

1. Press `Ctrl + ,` to open the settings.
2. Use the search bar to find "CMake."
3. Scroll until you see "CMake: Configure Environment."
4. Click "Add Item."
5. Key: `GP2040_BOARDCONFIG`
6. Value: `Pico` (or your working folder name).

When prompted, choose `GCC 10.3.1 ARM NONE EABI` for "Select a kit for GP2040-CE"

From inside VSCode:

1. Click the CMake icon on the left sidebar of VSCode.
2. You'll see three icons at the top-right corner of the CMake project outline. Click the "Configure All Projects" icon (looks like a page with an arrow).
3. Wait for the configuration process to complete. If progress isn't visible, open a new terminal in VSCode by clicking on the "Terminal" menu, then "New Terminal". A new terminal window will open at the bottom, navigate the "Output" tab.
4. Click "Build All Projects" in the CMake project outline.
5. The files should be in a new folder named "build" inside the GP2040-CE folder

### **Windows (Manual)**

### Windows (Manual)

Start in the GP2040-CE folder. **From a Developer Powershell or Developer Command Command Prompt**:

?> A new Powershell or Command Prompt session will be required after setting an environment variable.

1. Ensure you have the `PICO_SDK_PATH` environment variable set to the path to your pico-sdk folder.
2. (optional) Set the `GP2040_BOARDCONFIG` environment variable to the folder name for your board configuration.
    - Default value is `Pico`

3. Create a build directory, configure the build, and execute the build.

    ```console
    mkdir build
    cd build
    cmake -G "NMake Makefiles" ..
    nmake
    ```

4. Your UF2 file should be in the build directory.

### **Linux**

### Linux

Start in the GP2040-CE folder

1. Ensure you have the `PICO_SDK_PATH` environment variable set to the path to your pico-sdk folder.
2. (optional) Set the `GP2040_BOARDCONFIG` environment variable to the folder name for your board configuration.
    - Default value is `Pico`
3. Create a build directory, configure the build, and execute the build.

    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```

4. Your UF2 file should be in the build directory.

<!-- tabs:end -->

## Configuration

?> We're moving away from compile time configuration, in favor of runtime configuration.

There are two simple options for building GP2040-CE for your board. You can either edit an existing board definition, or create your own. Several example configurations are located in the repository **[configs](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs)** folder. This document will outline setting up a new build configuration.

### Board Configuration Folder

Each subfolder in [`configs`](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs) contains a separate board configuration, which consists of the following:

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

#### I2C Display Splash

The firmware also has a splash display feature. The default splash image has been defined in `headers/addons/i2cdisplay.h`. The data for the splash image are bytes representing the bitmap to be displayed on the OLED screen. The splash image can be set via the web-config. If you would like to change the default version of the splash image (to preserve it beyond data wipe), it can be customized with a C define named `DEFAULT_SPLASH` in the active `BoardConfig.h` file.
The instructions on how to generate those bytes are as follows:

1. Navigate to Bitmap Converter web utility
    - <https://marlinfw.org/tools/u8glib/converter.html>
2. Configure generated code
    - Select Marlin 2.x and Bitmap, untick all the rest.
3. Upload the splash image file
    - Set the image file after clicking on "Choose file". The image file needs to be preprocessed to your liking and cropped to 128x64.  This should populate a code block below.
4. Use the generated code
    - Copy the hex numbers inside the curly braces from the code block.
    - Navigate to the `BoardConfig.h` of your choice and at the end of the file right before `#endif`. Create a C define like so:

        ```cpp
        #define DEFAULT_SPLASH \
        ```

    - Then paste what you copied below what you just typed. Be sure to edit the array of bytes as follows:

        ```cpp
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \ // First line
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \ // Second line
        ... // All lines below previous ones end in \ similar to those above
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 // Last line doesn't have an \ to escape the new line
        ```

    - Here's a sample entry from Pico `BoardConfig.h`:

        ```cpp
        #define DEFAULT_SPLASH \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x80,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0xDF,0xC0, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xC1,0xFF,0xA0, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xCF,0xFE,0x80, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xE0,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xAB,0xC0,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2C,0x00,0x00,0xFB,0x83,0xFF,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x01,0xFF,0x00,0x01,0xF3,0x07,0xFC,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x07,0xFF,0x00,0x03,0xE0,0x3F,0xF4,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x2C,0x38,0x0A,0xE7,0x80,0x03,0xC0,0x5F,0x80,0x00, \
        0x00,0x00,0x00,0x20,0x07,0x81,0xFF,0x39,0xC3,0xC7,0x01,0xD7,0x80,0x3F,0x00,0x00, \
        0x00,0x00,0x00,0xE0,0x1F,0xE7,0xFF,0x79,0xC7,0x87,0x9F,0xA7,0x80,0x6E,0x3C,0x00, \
        0x00,0x00,0xA0,0xC0,0x3F,0xEA,0xE7,0xF3,0xEF,0x07,0x7F,0xC7,0x01,0xCF,0xFF,0x80, \
        0x00,0x03,0x8F,0xFC,0x7F,0xC3,0xC7,0x7F,0xFE,0x0F,0x7E,0x8F,0x87,0xDF,0xFC,0x00, \
        0x00,0x0F,0xFF,0xFF,0x73,0xC7,0x87,0xFF,0xDE,0x0E,0x50,0x07,0x1F,0x1F,0xD0,0x00, \
        0x00,0x1F,0x57,0xFF,0xE7,0x8F,0x07,0xFF,0x1C,0x3E,0x00,0x0F,0xFF,0x7F,0x00,0x00, \
        0x00,0x7E,0x0B,0x8F,0x8F,0x8E,0x0F,0xFF,0x9E,0x7C,0x00,0x07,0xFD,0xFC,0x00,0x00, \
        0x00,0x7C,0x37,0x9F,0x1F,0x1E,0x0F,0x87,0x1E,0xF8,0x00,0x07,0xF0,0x30,0x00,0x00, \
        0x00,0xF1,0xF7,0xFE,0x1E,0x1C,0x3E,0x0E,0x1F,0xF0,0x00,0x01,0x40,0x28,0x00,0x00, \
        0x01,0xEF,0xE7,0xF8,0x3C,0x3E,0x7C,0x0F,0x1F,0xE0,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x03,0xFF,0x8F,0xF0,0x7F,0xFE,0xF8,0x0E,0x0F,0x80,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x03,0xFF,0xCF,0xC0,0xFF,0xFF,0xF0,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x07,0x8B,0xFF,0x01,0xFF,0x8F,0xC0,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x07,0x87,0xFE,0x01,0xFE,0x0F,0x80,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x07,0x1F,0xDE,0x03,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x07,0xBF,0x9E,0x01,0xC0,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x07,0xFF,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x07,0xFB,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x03,0xE7,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x01,0x47,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x0E,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
        ```

6. Test and verify
    - Compile, if you did everything right, it should compile fine
    - Load the UF2 after the compile. Check and verify in the web-config, if you have set a custom image before, you might need to reset data.
