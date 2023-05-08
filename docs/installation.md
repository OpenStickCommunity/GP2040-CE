# Installation

The instructions will slightly vary based on your device. These instructions are for a Raspberry Pi Pico.

> If the device has been previously used for something other than GP2040-CE, please flash this file first to clear the on-board storage: [flash_nuke.uf2](https://raw.githubusercontent.com/OpenStickCommunity/GP2040-CE/main/docs/downloads/flash_nuke.uf2). After flashing the nuke file, wait a minute for the clear program to run and the RPI-RP2 drive to reappear.

1. Download the latest `GP2040-CE.uf2` file from the [Download](download) page for your board (e.g. `GP2040-CE_X.Y.Z_Pico.uf2` for the Raspberry Pi Pico).
2. Unplug your Pico.
3. Hold the BOOTSEL button on the Pico and plug into your computer. A new removable drive named `RPI-RP2` should appear in your file explorer.
4. Drag and drop the `GP2040-CE.uf2` file into the removable drive. This will flash the firmware file and automatically restart the Pico.

Congratulations! The board is now running the GP2040-CE firmware and will appear as a controller on your computer. You can test it in a game, using the website like [Gamepad Tester](https://gamepad-tester.com/), or in another application such as the Steam Input Overlay.

If you would like to customize your device, check out the built-in [Web Configurator](web-configurator).
