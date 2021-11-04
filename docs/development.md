# GP2040 Development

The project is built using the PlatformIO VS Code plugin along with the [Wiz-IO Raspberry Pi Pico](https://github.com/Wiz-IO/wizio-pico) platform package, using the baremetal (Pico SDK) configuration. There is an external dependency on the [MPG](https://github.com/FeralAI/MPG) C++ gamepad library for handling input state, providing extra features like Left/Right stick emulation and SOCD cleaning, and converting the generic gamepad state to the appropriate USB report.

There are two simple options for building GP2040 for your board. You can either edit an existing board definition, or create your own and configure PlatformIO to build it.

## Configuration

Several example configurations are located in the repository **[configs](https://github.com/FeralAI/GP2040/tree/main/configs)** folder. This document will outline setting up a new build configuration.

### Build Configuration

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

### Board Configuration

Create `configs/NewBoard/BoardConfig.h` and add your pin configuration and options:

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
```

The following board options are available in the `BoardConfig.h` file:

| Name             | Description                  | Required? |
| ---------------- | ---------------------------- | --------- |
| **PIN_DPAD_*X***<br>**PIN_BUTTON_*X*** | The GPIO pin for the button. Replace the *`X`* with GP2040 button or D-pad direction. | Yes |
| **DEFAULT_SOCD_MODE** | Defines the default SOCD mode to use, defaults to `SOCD_MODE_NEUTRAL`.<br>Available options are:<br>`SOCD_MODE_NEUTRAL`<br>`SOCD_MODE_UP_PRIORITY`<br>`SOCD_MODE_SECOND_INPUT_PRIORITY` | No |

### RGB LEDs

GP2040 supports per-button WS2812 and similar RGB LEDs.

The following RGB LED options are available in the `BoardConfig.h` file:

| Name             | Description                  | Required? |
| ---------------- | ---------------------------- | --------- |
| **BOARD_LEDS_PIN** | Data PIN for your LED strand | Yes       |
| **LED_LAYOUT** | Defines the layout of your LED buttons as a `Pixel` matrix.<br>Available options are:<br>`LED_LAYOUT_ARCADE_BUTTONS`<br>`LED_LAYOUT_ARCADE_HITBOX`<br>`LED_LAYOUT_ARCADE_WASD` | Yes |
| **LED_FORMAT** | Defines the color data format for the LED chain.<br>Available options are:<br>`LED_FORMAT_GRB`<br>`LED_FORMAT_RGB`<br>`LED_FORMAT_GRBW`<br>`LED_FORMAT_RGBW` | No, default value `LED_FORMAT_GRB` |
| **LEDS_PER_PIXEL** | The number of LEDs per button. | Yes |
| **LED_BRIGHTNESS_MAX** | Max brightness value, `uint8_t` 0-255. | Yes |
| **LED_BRIGHTNESS_STEPS** | The number of brightness steps when using the up/down hotkey. | Yes |
| **LEDS_DPAD_*X***<br>**LEDS_BUTTON_*X*** | The index of the button on the LED chain. Replace the *`X`* with GP2040 button or D-pad direction. | Yes |

An example RGB LED setup in the `BoardConfig.h` file:

```cpp
// BoardConfig.h

#include "enums.h"
#include "NeoPico.hpp"

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

### Player LEDs

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

## Building

You should now be able to build or upload the project to your RP2040 board from the Build and Upload status bar icons. You can also open the PlatformIO tab and select the actions to execute for a particular environment. Output folders are defined in the `platformio.ini` file and should default to a path under `.pio/build/${env:NAME}`.
