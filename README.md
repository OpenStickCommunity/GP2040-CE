# GP2040 - Multi-platform Gamepad Firmware for RP2040 microcontrollers

The goal of GP2040 is to provide multi-platform compatibility for RP2040-based game controllers. GP2040 currently support XInput on PC, Android, Raspberry Pi and others along with native Nintendo Switch compatibility. It also features such a 1ms response time, Left/Right stick emulation via D-pad and SOCD (Simultaneous Opposite Cardinal Direction) cleaning for the DIY fightstick crowd.

## Performance

Input latency is tested using the methodology outlined at [WydD's outstanding inputlag.science website](https://inputlag.science/controller/methodology), using the default 1000Hz (1ms) polling rate in the firmware.

| Mode | Min | Max | Avg | Stdev | % on time | %1f skip | %2f skip |
| - | - | - | - | - | - | - | - |
| XInput (1ms polling) | 0.56 ms | 1.34 ms | 0.86 ms | 0.24 ms | 95.94% | 4.06% | 0% |

## Building/Customization

The project is built using the PlatformIO VS Code plugin along with the [Wiz-IO Raspberry Pi Pico](https://github.com/Wiz-IO/wizio-pico) platform package using the baremetal configuration. There are no external dependencies outside of the Pico SDK included in the Wiz-IO platform.

### Edit Existing Definition

Once you have the project loaded into PlatformIO, edit the `include/definitions/RP2040Board.h` file to map your GPIO pins. Then from the PlatformIO environment selector, choose `env:raspberry-pi-pico`. From there you should be able to build or upload the project to you RP2040 board.

### Create New Definition

You can also add a new board definition to `include/definitions`. If you do, perform the following:

* Add `#define` for it in `include/definitions/BoardConfig.h`.
* Add option to `src/RP2040Gamepad.cpp` in the `BOARD_DEFINITION` selection logic.
* Add a new environment to the `platformio.ini`
  * Copy from existing env
  * Replace `BOARD_DEFINITION=#` with the number in the `BoardConfig.h` file.

You will now have a new build environment target for PlatformIO.

## Usage

### Layout

The project uses a generic button labeling for gamepad state, which is then converted to the appropriate input type before sending. Here are the mappings of generic buttons to each supported platform/layout:

| Generic | XInput | Switch  | Arcade |
| ------- | ------ | ------- | ------ |
| 01      | A      | B       | K1     |
| 02      | B      | A       | K2     |
| 03      | X      | Y       | P1     |
| 04      | Y      | X       | P2     |
| 05      | LB     | L       | P4     |
| 06      | RB     | R       | P3     |
| 07      | LT     | ZL      | K4     |
| 08      | RT     | ZR      | K3     |
| 09      | Back   | -       | Coin   |
| 10      | Start  | +       | Start  |
| 11      | LS     | LS      | LS     |
| 12      | RS     | RS      | RS     |
| 13      | Guide  | Home    | -      |
| 14      | RS     | Capture | -      |

Any references to these buttons will use the `XInput` labels in this documentation.

### Home Button

If you do not have a dedicated Home button, but you can activate it via the **`BACK + START + UP`** button combination.

### Input Modes

To change the input mode, **hold one of the following buttons as the controller is plugged in:**

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

## Acknowledgements

* Ha Thach for the TinyUSB library and excellent examples.
* fluffymadness for the [tinyusb-xinput](https://github.com/fluffymadness/tinyusb-xinput) example.
* Kevin Boone for his [blog post on using RP2040 flash memory as emulated EEPROM](https://kevinboone.me/picoflash.html).
