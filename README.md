# GP2040-CE | Community Edition Firmware

GP2040-CE is a gamepad firmware for the Raspberry Pi Pico and other boards based on the RP2040 microcontroller, and provides high performance with a rich feature set across multiple platforms. GP2040-CE is compatible with platforms such as PC, Nintendo Switch, PS3/PS4, Steam Deck, MiSTer and Android.

Full documentation can be found at <https://gp2040-ce.info>.

## Features

* Select from 5 input modes: XInput, Nintendo Switch, PS4, PS3 and Keyboard
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
| v0.7.0 | All modes (except PS4) | 1 ms | 0.53 ms | 1.36 ms | 0.86 ms | 0.25 ms | 95.91% | 4.09% | 0% |
| v0.7.0 | PS4 Mode | 1 ms | 1.45 ms | 2.14 ms | 1.86 ms | 0.16 ms | 90.26% | 9.74% | 0% |

Full results can be found in the [GP2040-CE Firmware Latency Test Results](https://docs.google.com/spreadsheets/d/1OLmVg04e2q4aY0tqSJuaHtD8YMNCQnP-MzzSTtQLVm4) Google Sheet.

## Installation

Prebuilt firmware files in `UF2` format are available in the [Releases](https://github.com/OpenStickCommunity/GP2040-CE/releases) section.

The default pinout for GP2040-CE is based on an official Raspberry Pi Pico. That pinout can be found [HERE](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/Pico/assets/PinMapping.png). This can be found in the [Releases](https://github.com/OpenStickCommunity/GP2040-CE/releases) section under Pico.

In addition to the default UF2 we also offer pre-compiled UF2s from community members that have verified operation with their own devices. Some of these additional configurations include:

> * [Pico Fighting Board](https://github.com/FeralAI/GP2040-Config-PicoFightingBoard/)
> * [DURAL](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/DURAL)
> * [Flatbox Rev 4](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/FlatboxRev4)
> * [Flatbox Rev 5](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/FlatboxRev5)
> * [WaveShare RP2040 Zero](https://github.com/OpenStickCommunity/GP2040-CE/tree/main/configs/WaveshareZero)

The instructions will slightly vary based on your device. These instructions are for a Raspberry Pi Pico.

> If the device has been previously used for something other than GP2040-CE, please flash this file first to clear the on-board storage: [flash_nuke.uf2](https://raw.githubusercontent.com/OpenStickCommunity/GP2040-CE/main/docs/downloads/flash_nuke.uf2). After flashing the nuke file, wait a minute for the clear program to run and the RPI-RP2 drive to reappear.

1. Download the latest `GP2040-CE.uf2` file from the [Releases](https://github.com/OpenStickCommunity/GP2040-CE/releases) section for your board (e.g. `GP2040-CE_X.Y.Z_Pico.uf2` for the Raspberry Pi Pico).
2. Unplug your Pico.
3. Hold the BOOTSEL button on the Pico and plug into your computer. A new removable drive named `RPI-RP2` should appear in your file explorer.
4. Drag and drop the `GP2040-CE.uf2` file into the removable drive. This will flash the firmware file and automatically restart the Pico.
5. The board is now running the GP2040-CE firmware and will appear as a controller on your computer. You can test it in a game, using the website [Gamepad Tester](https://gamepad-tester.com/), or in another application such as the Steam Input Overlay.

## Support

If you would like to discuss features, issues or anything else related to GP2040-CE please [create an issue](https://github.com/OpenStickCommunity/GP2040-CE/issues/new) or join the [OpenStick GP2040-CE Discord channel](https://discord.gg/k2pxhke7q8).

### Frequently Asked Questions

#### Which input mode should I use?

This will depend on the platform you will be playing on:

* Use `XInput Mode` as the preferred mode for compatibility with PC games and 3rd party console adapters
* Use `PS4 Mode` on PS4, and on PS4 games on PS5 systems
* Use `PS3 Mode` on PS3, and on PS4 with games that support legacy controllers
* Use `Switch Mode` on Nintendo Switch
* Use `Keyboard Mode` for MAME cabinets, PC rhythm games, etc.

#### Will GP2040-CE natively support PS5, Xbox One, or Xbox Series consoles?

These consoles implement security measures that prevent unauthorized accessories from being used. The process of cracking or bypassing that security may not be legal everywhere. These consoles could be supported in the future if a user-friendly and completely legal implementation method is found, such as the implementation of [PS4 Mode](https://gp2040-ce.info/#/web-configurator?id=ps4-mode).

#### Can I use multiple controllers with GP2040-CE on the same system?

Yes! Each GP2040-CE board is treated as a separate controller. Be sure to only run the embedded web configurator for each controller one at a time.

If you're installing boards with GP2040-CE in an arcade cabinet, check out the [Player Number addon](https://gp2040-ce.info/#/web-configurator?id=player-number-x-input-only) to force each board to a specific player number.

#### Does GP2040-CE really have less than 1 ms of input latency?

Yes! If your platform supports 1000 Hz USB polling, input latency will be less than 1ms. GP2040-CE is configured for 1000 Hz / 1 ms polling by default in all modes, however some systems override or ignore the polling rate the controller requests. The 1000 Hz polling rate is confirmed to work on PC and MiSTer. Even if your platform doesn't support high rate USB polling, GP2040-CE is still reading and processing your inputs as fast as the target system will allow.

#### Do the additional features like RGB LEDs, Player LEDs, and OLED displays affect performance?

Not at all! The RP2040 processor of the Pico has two cores. GP2040-CE dedicates one of these cores strictly to reading, processing and sending player inputs. All secondary functions such as LEDs and displays are run on the secondary core. No matter how crazy the feature set becomes, GP2040-CE is unlikely to introduce any additional input latency.

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
* [deeebug](https://github.com/deeebug) for improvments to the web-UI and fixing the PS3 homebutton issue
* [TheTrain](https://github.com/TheTrainGoes/GP2040-Projects) and [Fortinbra](https://github.com/Fortinbra) for helping keep our community chugging along
* [PassingLink](https://github.com/passinglink/passinglink) for the technical details and code for PS4 implementation
