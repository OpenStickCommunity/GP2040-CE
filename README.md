<p align="center">
  <a href="https://gp2040-ce.info">
    <img alt="GP2040-CE" src="https://raw.githubusercontent.com/OpenStickCommunity/GP2040-CE/main/docs/assets/images/gp2040-ce-logo.png">
  </a>
</p>

<p align="center">
  Multiplatform Gamepad Firmware for RP2040
</p>

<p align="center">
  <img src="https://img.shields.io/github/license/OpenStickCommunity/GP2040-CE">
  <img src="https://img.shields.io/github/actions/workflow/status/OpenStickCommunity/GP2040-CE/cmake.yml">
  <br>
  <img src="https://img.shields.io/badge/inputlag.science-0.86%20ms-blue">
  <img src="https://img.shields.io/badge/MiSTer%20latency-0.765%20ms-blue">
</p>

<p>
  GP2040-CE (Community Edition) is a gamepad firmware for the Raspberry Pi Pico and other boards based on the RP2040 microcontrollers that combines multiplatform compatibility, low latency and a rich feature set to provide endless customization possibilities without sacrificing performance.
</p>

<p>
  GP2040-CE is compatible with PC, PS3 and PS4, Nintendo Switch, Steam Deck, MiSTer and Android.
</p>

## Links

[Downloads](https://gp2040-ce.info/#/download) | [Installation](https://gp2040-ce.info/#/installation) | [Wiring](https://gp2040-ce.info/#/wiring) | [Usage](https://gp2040-ce.info/#/usage) | [FAQ](https://gp2040-ce.info/#/faq) | [GitHub](https://github.com/OpenStickCommunity/GP2040-CE)

Full documentation can be found at <https://gp2040-ce.info>

## Features

* Select from 5 input modes: XInput, Nintendo Switch, PS4, PS3 and Keyboard
* Overclocked polling rate for less than 1 ms of input latency in all modes.
* Multiple SOCD cleaning modes - Up Priority (a.k.a. Stickless), Neutral, and Second Input Priority.
* Left and Right stick emulation via D-pad inputs as well as dedicated toggle switches.
* Dual direction via D-pad + LS/RS.
* Reversed input via a button.
* [Turbo and Turbo LED](https://gp2040-ce.info/#/web-configurator-add-ons?id=turbo) with selectable speed
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

## Support

If you would like to discuss features, issues or anything else related to GP2040-CE please [create an issue](https://github.com/OpenStickCommunity/GP2040-CE/issues/new) or join the [OpenStick GP2040-CE Discord](https://discord.gg/k2pxhke7q8) support channel.

## Contributing

Want to help improve GP2040-CE? There are a bunch of ways to contribute!

### Community Participation

Have an idea for a cool new feature, or just want to discuss some technical details with the devs? Join the [OpenStick GP2040-CE Discord](https://discord.gg/k2pxhke7q8) server to participate in our active and ever-growing community!

### Pull Requests

Pull requests are welcome and encouraged for enhancements, bug fixes and documentation updates.

Please respect the coding style of the file(s) you are working in, and enforce the use of the `.editorconfig` file when present.

## Acknowledgements

* [FeralAI](https://github.com/FeralAI) for building [GP2040](https://github.com/FeralAI/GP2040) and laying the foundation for this community project
* Ha Thach's excellent [TinyUSB library](https://github.com/hathach/tinyusb) examples
* fluffymadness's [tinyusb-xinput](https://github.com/fluffymadness/tinyusb-xinput) sample
* Kevin Boone's [blog post on using RP2040 flash memory as emulated EEPROM](https://kevinboone.me/picoflash.html)
* [bitbank2](https://github.com/bitbank2) for the [OneBitDisplay](https://github.com/bitbank2/OneBitDisplay) and [BitBang_I2C](https://github.com/bitbank2/BitBang_I2C) libraries, which were ported for use with the Pico SDK
* [arntsonl](https://github.com/arntsonl) for the amazing cleanup and feature additions that brought us to v0.5.0
* [alirin222](https://github.com/alirin222) for the awesome turbo code ([@alirin222](https://twitter.com/alirin222) on Twitter)
* [deeebug](https://github.com/deeebug) for improvements to the web-UI and fixing the PS3 homebutton issue
* [TheTrain](https://github.com/TheTrainGoes/GP2040-Projects) and [Fortinbra](https://github.com/Fortinbra) for helping keep our community chugging along
* [PassingLink](https://github.com/passinglink/passinglink) for the technical details and code for PS4 implementation
