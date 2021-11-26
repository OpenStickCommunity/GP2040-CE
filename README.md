# GP2040 Firmware

GP2040 is a gamepad firmware for the RP2040 microcontroller that provides high performance and a rich feature set across multiple platforms. GP2040 is compatible with PC, PS3, PS4 (legacy support) and Nintendo Switch.

Full documentation can be found at <https://feralai.github.io/GP2040>.

## Features

* Selectable input modes (XInput, DirectInput and Nintendo Switch)
* Overclocked polling rate to 1000 Hz (1 ms) in all modes, with less than 1 ms of input latency
* Multiple SOCD cleaning modes - Neutral, Up Priority (a.k.a. Hitbox), Second Input Priority
* Left and Right stick emulation via D-pad inputs
* Per-button RGB LED support
* PWM and RGB player indicator LED support (XInput only)
* Saves options to internal memory
* Support for 128x64 monochrome I2C displays using SSD1306, SH1106 or SH1107 display drivers.
* Built-in configuration app hosted via embedded webserver...no downloading a separate app!

Take a look at the [GP2040 Usage](https://feralai.github.io/GP2040/#/usage) page for more details.

## Installation

Prebuilt `uf2` files are available in the [Releases](https://github.com/FeralAI/GP2040/releases) section for the following boards and controllers:

* [Raspberry Pi Pico](https://github.com/FeralAI/GP2040/tree/main/configs/Pico) and other pin-compatible boards such as the Pimoroni Pico Lipo
* [Pico Fighting Board](https://github.com/FeralAI/GP2040-Config-PicoFightingBoard/)
* [Crush Counter](https://github.com/FeralAI/GP2040/tree/main/configs/CrushCounter) (formerly the [OSFRD](https://github.com/FeralAI/GP2040/tree/main/configs/OSFRD))
* [DURAL](https://github.com/FeralAI/GP2040/tree/main/configs/DURAL)

Several other working example configurations are located in the [configs](https://github.com/FeralAI/GP2040/tree/main/configs) folder.

The instructions will slightly vary based on your device. These instructions are for a Raspberry Pi Pico.

1. Download the latest `GP2040.uf2` file from the [Releases](https://github.com/FeralAI/GP2040/releases) section for your board (e.g. `GP2040-PiPico.uf2` for the Raspberry Pi Pico).
1. Unplug your Pico.
1. Hold the BOOTSEL button on the Pico and plug into your computer. A new removable drive named `RPI-RP2` should appear in your file explorer.
1. Drag and drop the `GP2040.uf2` file into the removable drive. This will flash the board.
1. The board is now running the GP2040 firmware and will appear as a controller on your computer.

## Performance

Input latency is tested using the methodology outlined at [WydD's inputlag.science website](https://inputlag.science/controller/methodology), using the default 1000 Hz (1 ms) polling rate in the firmware.

| Version | Mode | Poll Rate | Min | Max | Avg | Stdev | % on time | %1f skip | %2f skip |
| - | - | - | - | - | - | - | - | - | - |
| v0.3.1 | All | 1 ms | 0.56 ms | 1.32 ms | 0.85 ms | 0.24 ms | 95.95% | 4.05% | 0% |

Full results can be found in the [GP2040 Firmware Latency Test Results](https://docs.google.com/spreadsheets/d/1eeX0SCOYnUDZMYzt_69wDpjnB_XUtvsfvHJYxxgTj28/edit#gid=1559471406) Google Sheet.

## Support

If you would like to discuss features, issues or anything else related to GP2040 please [create an issue](https://github.com/FeralAI/GP2040/issues/new) or join the [OpenStick GP2040 Discord channel](https://discord.gg/KyQCHcjwJ2).

## Acknowledgements

* Ha Thach's excellent [TinyUSB library](https://github.com/hathach/tinyusb) examples
* fluffymadness's [tinyusb-xinput](https://github.com/fluffymadness/tinyusb-xinput) sample
* Kevin Boone's [blog post on using RP2040 flash memory as emulated EEPROM](https://kevinboone.me/picoflash.html)
* [bitbank2](https://github.com/bitbank2) for the [OneBitDisplay](https://github.com/bitbank2/OneBitDisplay) and [BitBang_I2C](https://github.com/bitbank2/BitBang_I2C) libraries, which were ported for use with the Pico SDK
