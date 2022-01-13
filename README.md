# GP2040 Firmware

GP2040 is a gamepad firmware for the Raspberry Pi Pico and other boards based on the RP2040 microcontroller, and provides high performance with a rich feature set across multiple platforms. GP2040 is compatible with PC, MiSTer, Android, Raspberry Pi, Nintendo Switch, PS3 and PS4 (legacy controller support).

Full documentation can be found at <https://gp2040.info>.

## Features

* Selectable input modes (XInput, DirectInput and Nintendo Switch)
* Overclocked polling rate to 1000 Hz (1 ms) in all modes, with less than 1 ms of input latency
* Multiple SOCD cleaning modes - Neutral, Up Priority (a.k.a. Hitbox), Second Input Priority
* Left and Right stick emulation via D-pad inputs
* Per-button RGB LED support
* PWM and RGB player indicator LED support (XInput only)
* Saves options to internal memory
* Support for 128x64 monochrome I2C displays using SSD1306, SH1106 or SH1107 display drivers.
* [Built-in configuration app](https://gp2040.info/#/web-configurator) hosted via embedded webserver...no downloading a separate app!

Take a look at the [GP2040 Usage](https://gp2040.info/#/usage) page for more details.

## Performance

Input latency is tested using the methodology outlined at [WydD's inputlag.science website](https://inputlag.science/controller/methodology), using the default 1000 Hz (1 ms) polling rate in the firmware.

| Version | Mode | Poll Rate | Min | Max | Avg | Stdev | % on time | %1f skip | %2f skip |
| - | - | - | - | - | - | - | - | - | - |
| v0.3.1 | All | 1 ms | 0.56 ms | 1.32 ms | 0.85 ms | 0.24 ms | 95.95% | 4.05% | 0% |

Full results can be found in the [GP2040 Firmware Latency Test Results](https://docs.google.com/spreadsheets/d/1eeX0SCOYnUDZMYzt_69wDpjnB_XUtvsfvHJYxxgTj28/edit#gid=1559471406) Google Sheet.

## Installation

Prebuilt `uf2` files are available in the [Releases](https://github.com/FeralAI/GP2040/releases) section for the following boards and controllers:

* [Raspberry Pi Pico](https://github.com/FeralAI/GP2040/tree/main/configs/Pico) and other pin-compatible boards such as the Pimoroni Pico Lipo ([wiring diagram](https://raw.githubusercontent.com/FeralAI/GP2040/main/configs/Pico/assets/PinMapping.png))
* [Pico Fighting Board](https://github.com/FeralAI/GP2040-Config-PicoFightingBoard/)
* [Crush Counter](https://github.com/FeralAI/GP2040/tree/main/configs/CrushCounter) (formerly the [OSFRD](https://github.com/FeralAI/GP2040/tree/main/configs/OSFRD))
* [DURAL](https://github.com/FeralAI/GP2040/tree/main/configs/DURAL)
* [Flatbox Rev 4](https://github.com/jfedor2/flatbox/tree/master/hardware-rev4)

Several other working example configurations are located in the [configs](https://github.com/FeralAI/GP2040/tree/main/configs) folder.

The instructions will slightly vary based on your device. These instructions are for a Raspberry Pi Pico.

> If the device has been previously used for something other than GP2040, please flash this file first to clear the on-board storage: [flash_nuke.uf2](docs/downloads/flash_nuke.uf2). After flashing the nuke file, wait a minute for the clear program to run and the RPI-RP2 drive to reappear.

1. Download the latest `GP2040.uf2` file from the [Releases](https://github.com/FeralAI/GP2040/releases) section for your board (e.g. `GP2040-PiPico.uf2` for the Raspberry Pi Pico).
1. Unplug your Pico.
1. Hold the BOOTSEL button on the Pico and plug into your computer. A new removable drive named `RPI-RP2` should appear in your file explorer.
1. Drag and drop the `GP2040.uf2` file into the removable drive. This will flash the board.
1. The board is now running the GP2040 firmware and will appear as a controller on your computer.

## Support

If you would like to discuss features, issues or anything else related to GP2040 please [create an issue](https://github.com/FeralAI/GP2040/issues/new) or join the [OpenStick GP2040 Discord channel](https://discord.gg/KyQCHcjwJ2).

### Frequently Asked Questions

#### Which input mode should I use?

Generally speaking, XInput will be the mode of choice for everything except Nintendo Switch and PlayStation 3. XInput mode is the most fully-featured, has the best compatibility with PC games and is compatible with console adapters like the Brook Wingman product line. All things being equal, performance is the same in all modes.

#### What is the extent of PS4 support in GP2040?

GP2040 will work on PS4 games that implement support for legacy PS3 controllers. Many of the popular PS4 fighting games have this support.

#### Does/can/will GP2040 natively support the PS4, PS5, Xbox One or Xbox Series consoles?

These consoles implement security to prevent unauthorized accessories from being used. The process of cracking or bypassing that security may not be legal everywhere. These consoles could be supported in the future if a user-friendly and completely legal implementation method is found.

#### Can I use multiple controllers with GP2040 on the same system?

Yes! Each board with GP2040 is treated as a separate controller. The one thing to keep in mind would be to only run the web configurator for one controller at a time.

#### Does GP2040 really have less than 1 ms of latency?

Yes...if your platform supports 1000 Hz USB polling. GP2040 is configured for 1000 Hz / 1 ms polling by default in all modes, however some systems override or ignore the polling rate the controller requests. PC and MiSTer are confirmed to work with 1000 Hz polling. Even if your system doesn't support a USB polling rate that high, you can feel comfortable knowing GP2040 is still reading and processing your inputs as fast as the target system will allow.

#### Do the additional features like RGB LEDs, Player LEDs and OLED displays affect performance?

No! The RP2040 chip contains two processing cores. GP2040 dedicates one core to reading inputs and sending them via USB, while the second core is used to handle any auxiliary modules like LEDs and display support. No matter how crazy the feature set of GP2040 becomes, it's unlikely your controller's input latency will be affected.

#### Why do the buttons have weird labels like B3, A1, S2, etc.?

GP2040 uses a generic system for handling button inputs that most closely maps to a traditional PlayStation controller layout with a few extra buttons. This means 4 face buttons (B1-B4), 4 shoulder buttons (L1, L2, R1, R2), Select and Start (S1, S2), 2 stick buttons (L3, R3) and 2 auxiliary buttons for things like Home and Capture (A1, A2) on the Switch. The GP2040 documentation and web configurator have a dropdown to change the labels to more familiar controller layouts. You can also refer to the button mapping table on the [GP2040 Usage](https://gp2040.info/#/usage?id=buttons) page.

#### Why use PlatformIO instead of \<insert favorite project setup\>?

Setting up a development environment to build Pico SDK projects is a manual process which requires several components to be installed and configured. Using PlatformIO allows easy installation and updating of build and project dependencies, and makes for a less confusing experience for new developers and people that just want to make a few tweaks for a custom build.

#### What kind of voodoo is that built-in web configurator?

There's no magic here, just some useful libraries working together:

* Single page application using React and Bootstrap is embedded in the GP2040 firmware
* TinyUSB library provides virtual network connection over USB via RNDIS
* lwIP library provides an HTTP server for the embedded React app and the web configuration API
* ArduinoJson library is used for serialization and deserialization of web API requests

## Contributions

Want to help improve GP2040? There are a bunch of ways to contribute!

### Pull Requests

Pull requests are welcome and encouraged for enhancements, bug fixes and documentation updates.

Please respect the coding style of the file(s) you are working in, and enforce the use of the `.editorconfig` file when present.

### Donations

If you'd like to make a donation to my open source work, you can

<a href="https://github.com/sponsors/FeralAI"><img src="https://github.com/FeralAI/GP2040/raw/main/.github/assets/github-sponsor-dimmed.png" alt="Sponsor Feral AI on Github" style="height: 40px !important;" ></a>

or

<a href="https://www.buymeacoffee.com/feralai" target="_blank"><img src="https://www.buymeacoffee.com/assets/img/custom_images/orange_img.png" alt="Buy Me A Coffee" style="height: 40px !important;" ></a>

## Acknowledgements

* Ha Thach's excellent [TinyUSB library](https://github.com/hathach/tinyusb) examples
* fluffymadness's [tinyusb-xinput](https://github.com/fluffymadness/tinyusb-xinput) sample
* Kevin Boone's [blog post on using RP2040 flash memory as emulated EEPROM](https://kevinboone.me/picoflash.html)
* [bitbank2](https://github.com/bitbank2) for the [OneBitDisplay](https://github.com/bitbank2/OneBitDisplay) and [BitBang_I2C](https://github.com/bitbank2/BitBang_I2C) libraries, which were ported for use with the Pico SDK
