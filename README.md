# GP2040-CE | Community Edition Firmware

GP2040-CE is a gamepad firmware for the Raspberry Pi Pico and other boards based on the RP2040 microcontroller, and provides high performance with a rich feature set across multiple platforms. GP2040-CE is compatible with PC, MiSTer, Android, Raspberry Pi, Nintendo Switch, PS3 and the PS4 (in games that support legacy controllers).

Full documentation can be found at <https://gp2040-ce.info>.

## Features

* Selectable input modes - XInput, DirectInput, and Nintendo Switch.
* Overclocked polling rate for less than 1 ms of input latency in all modes.
* Multiple SOCD cleaning modes - Up Priority (a.k.a. Stickless), Neutral, and Second Input Priority.
* Left and Right stick emulation via D-pad inputs as well as dedicated toggle switches.
* Dual direction via D-pad + LS/RS.
* Reversed input via a button.
* Turbo and Turbo LED with selectable speed
* Per-button RGB LED support.
* PWM Player indicator LED support (XInput only).
* Multiple LED profiles support.
* Support for 128x64 monochrome I2C displays - SSD1306, SH1106, and SH1107 compatible.
* Custom startup splash screen and easy image upload via web configuration.
* Support for passive buzzer speaker (3v or 5v).
* [Built-in, embedded web configuration](https://gp2040-ce.info/#/web-configurator) - No download required!

Visit the [GP2040-CE Usage](https://gp2040-ce.info/#/usage) page for more details.

## Performance

Input latency is tested using the methodology outlined at [WydD's inputlag.science website](https://inputlag.science/controller/methodology), using the default 1000 Hz (1 ms) polling rate in the firmware.

| Version | Mode | Poll Rate | Min | Max | Avg | Stdev | % on time | %1f skip | %2f skip |
| - | - | - | - | - | - | - | - | - | - |
| v0.6.3 | All | 1 ms | 0.46 ms | 1.64 ms | 0.89 ms | 0.29 ms | 95.59% | 4.41% | 0% |

Full results can be found in the [GP2040-CE Firmware Latency Test Results](https://docs.google.com/spreadsheets/d/1eeX0SCOYnUDZMYzt_69wDpjnB_XUtvsfvHJYxxgTj28/edit#gid=1559471406) Google Sheet.

## Installation

Prebuilt MicroPython `UF2` files are available in the [Releases](https://github.com/OpenStickCommunity/GP2040-CE/releases) section.

The default pinout for GP2040-CE is based on an official RaspBerry Pi Pico. That pinout can be found [HERE](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/Pico/assets/PinMapping.png).  This can be found in the [Releases](https://github.com/OpenStickCommunity/GP2040-CE/releases) section under Pico.

In addition to the default UF2 we also offer pre-compiled UF2s from community members that have verified operation with their own devices. Some of these additional configurations include:

> * [Pico Fighting Board](https://github.com/FeralAI/GP2040-Config-PicoFightingBoard/)
> * [DURAL](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/DURAL)
> * [Flatbox Rev 4](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/FlatboxRev4)
> * [Flatbox Rev 5](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/FlatboxRev5)
> * [WaveShare RP2040 Zero](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/WaveshareZero)

The instructions will slightly vary based on your device. These instructions are for a Raspberry Pi Pico.

> If the device has been previously used for something other than GP2040-CE, please flash this file first to clear the on-board storage: [flash_nuke.uf2](https://raw.githubusercontent.com/OpenStickCommunity/GP2040-CE/main/docs/downloads/flash_nuke.uf2). After flashing the nuke file, wait a minute for the clear program to run and the RPI-RP2 drive to reappear.

1. Download the latest `GP2040-CE.uf2` file from the [Releases](https://github.com/OpenStickCommunity/GP2040-CE/releases) section for your board (e.g. `GP2040-CE-PicoFightingBoard_vX.X.X.uf2` for the Raspberry Pi Pico).
1. Unplug your Pico.
1. Hold the BOOTSEL button on the Pico and plug into your computer. A new removable drive named `RPI-RP2` should appear in your file explorer.
1. Drag and drop the `GP2040-CE.uf2` file into the removable drive. This will flash the board.
1. The board is now running the GP2040-CE firmware and will appear as a controller on your computer.

## Support

If you would like to discuss features, issues or anything else related to GP2040 please [create an issue](https://github.com/OpenStickCommunity/GP2040-CE/issues/new) or join the [OpenStick GP2040-CE Discord channel](https://discord.gg/k2pxhke7q8).

### Frequently Asked Questions

#### Which input mode should I use?

Generally speaking, XInput will be the mode of choice for everything except Nintendo Switch and PlayStation 3. XInput mode is the most fully-featured, has the best compatibility with PC games, and is compatible with console adapters like the Brook Wingman product line.  All modes come with the same great performance!

#### What is the extent of PS4 and PS5 support in GP2040-CE?

GP2040-CE will work on the PS4 in PS4 games that implement support for legacy PS3 controllers. Many of the popular PS4 fighting games have this support.  The PS5 does not support legacy PS3 controllers so you will need a converter like the Brook Wingman.

#### Will GP2040-CE natively support PS4, PS5, Xbox One, or Xbox Series consoles?

These consoles implement security measures that prevent unauthorized accessories from being used. The process of cracking or bypassing that security may not be legal everywhere. These consoles could be supported in the future if a user-friendly and completely legal implementation method is found.

#### Can I use multiple controllers with GP2040-CE on the same system?

Yes! Each GP2040-CE board is treated as a separate controller. However, be sure to only run the embedded web configurator for each controller one at a time.

#### Does GP2040-CE really have less than 1 ms of input latency?

Yes! If your platform supports 1000 Hz USB polling, input latency will be less than 1ms. GP2040-CE is configured for 1000 Hz / 1 ms polling by default in all modes, however some systems override or ignore the polling rate the controller requests. The 1000 Hz polling rate is confirmed to work on PC and MiSTer. Even if your platform doesn't support high rate USB polling, GP2040-CE is still reading and processing your inputs as fast as the target system will allow.

#### Do the additional features like RGB LEDs, Player LEDs, and OLED displays affect performance?

No! GP2040-CE dedicates a processing core to just reading and writing player inputs. All secondary functions such as LEDs and displays are controlled by the secondary processor core. No matter how crazy the feature set becomes, GP2040-CE is unlikely to introduce any additional input latency.

#### Why do the buttons have weird labels like B3, A1, S2, etc.?

GP2040-CE uses a generic system for handling button inputs that resembles a traditional PlayStation controller layout with a few extra buttons. This means 4 face buttons (B1-B4), 4 shoulder buttons (L1, L2, R1, R2), Select and Start (S1, S2), 2 stick buttons (L3, R3) and 2 auxiliary buttons for things like Home and Capture (A1, A2) on the Switch. The GP2040-CE documentation and web configurator both provide a dropdown to change the button labels to more familiar controller layouts. You can refer to the button mapping table on the [GP2040 Usage](http://gp2040-ce.info/#/usage?id=buttons) page.

#### What kind of voodoo is that embedded web configurator?

There's no magic here, just some useful libraries working together:

* Single page application using React and Bootstrap is embedded in the GP2040-CE firmware.
* TinyUSB library provides virtual network connection over USB via RNDIS.
* lwIP library provides an HTTP server for the embedded React app and the web configuration API.
* ArduinoJson library is used for serialization and deserialization of web API requests.

## Contributing

Want to help improve GP2040-CE? There are a bunch of ways to contribute!

### Pull Requests

Pull requests are welcome and encouraged for enhancements, bug fixes and documentation updates.

Please respect the coding style of the file(s) you are working in, and enforce the use of the `.editorconfig` file when present.

### Discussions

Join the [OpenStick GP2040-CE channel](https://discord.gg/k2pxhke7q8) to participate!

## Acknowledgements

* [FeralAI](https://github.com/FeralAI) for building [GP2040](https://github.com/FeralAI/GP2040) and laying the foundation for this community project
* Ha Thach's excellent [TinyUSB library](https://github.com/hathach/tinyusb) examples
* fluffymadness's [tinyusb-xinput](https://github.com/fluffymadness/tinyusb-xinput) sample
* Kevin Boone's [blog post on using RP2040 flash memory as emulated EEPROM](https://kevinboone.me/picoflash.html)
* [bitbank2](https://github.com/bitbank2) for the [OneBitDisplay](https://github.com/bitbank2/OneBitDisplay) and [BitBang_I2C](https://github.com/bitbank2/BitBang_I2C) libraries, which were ported for use with the Pico SDK
* [arntsonl](https://github.com/arntsonl) for the amazing cleanup and feature additions that brought us to v0.5.0
* [alirin222](https://github.com/alirin222) for the awesome turbo code ([@alirin222](https://twitter.com/alirin222) on Twitter)
* [DeBug] for improvments to the web-UI and fixing the PS3 homebutton issue
* [TheTrain](https://github.com/TheTrainGoes/GP2040-Projects) and [Fortinbra](https://github.com/Fortinbra) for helping keep our community chugging along
* [PassingLink](https://github.com/passinglink/passinglink) for the technical details and code for PS4 implementation