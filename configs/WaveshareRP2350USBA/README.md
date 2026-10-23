# GP2040 Configuration for Waveshare RP2350-USB-A

Configuration for the [Waveshare RP2350-USB-A](https://www.waveshare.com/wiki/RP2350-USB-A), used as a USB controller converter.

- **USB-C**: connects to the console or PC (GP2040-CE output).
- **USB-A**: plug the input controller here. It is driven by PIO USB on GPIO12 (D+) / GPIO13 (D-), with the Gamepad USB Host add-on enabled by default.
- **GPIO16**: onboard WS2812 RGB LED.

No GPIO buttons are mapped by default.

## Supported input controllers

Whatever the Gamepad USB Host add-on supports (DualShock 4, DualSense, Switch Pro, Stadia, wired Xbox 360 `045E:028E`, 8BitDo Ultimate 2.4G receivers in XInput mode, ...). A USB hub on the USB-A port is supported, e.g. to plug both an input controller and an Xbox One authentication device for Xbox One mode.

## Entering web-config

There are no buttons to hold at boot: once the board is running, hold **S2 + B3 + B4** (Start + X + Y on an Xbox-layout controller) on the controller plugged into the USB-A port for 4 seconds. The same combo returns to gamepad mode.
