# Firmware Installation

## General Process

> If the device has been previously used for something other than GP2040-CE, you will need to follow the [Flash Nuke Process](#flash-nuke-process) below first to clear the memory entirely in preparation for installing GP2040-CE.

1. Download the compiled firmware (.uf2 file) for your device
2. Unplug your device.
3. Put your device into Bootsel/USB mode. A new removable drive named `RPI-RP2` should appear in your file explorer.
4. Drag and drop the compiled firmware (.uf2 file) downloaded in step 1 into the removable drive.
5. Wait for the device to automatically disconnect.

Congratulations! The board is now running the GP2040-CE firmware and will appear as a controller on your computer. You can test it in a game, using the website like [Gamepad Tester](https://gamepad-tester.com/), or in another application such as the Steam Input Overlay.

If you would like to customize your device, check out the built-in [Web Configurator](web-configurator).

### Example Process (Raspberry Pi Pico)

**The instructions will slightly vary based on your device. These instructions are for a Raspberry Pi Pico.**

1. Download the latest `GP2040-CE_X.X.X_Pico.uf2` file for the Raspberry Pi Pico from the [Download](download) page.
2. Unplug your Pico.
3. Hold the BOOTSEL button on the Pico and plug into your computer.
4. Drag and drop the `GP2040-CE_X.X.X_Pico.uf2` file into the removable drive.
5. Wait for the Pico to automatically disconnect.

## Flash Nuke process

!> *Warning* - Flash nuking your board wipes all your custom configuration for pin mappings and addons.

1. Download the [flash_nuke.uf2 file](https://raw.githubusercontent.com/OpenStickCommunity/GP2040-CE/main/docs/downloads/flash_nuke.uf2)
2. Boot into bootsel/RPI drive (multiple methods to do so)
3. Drag [flash_nuke.uf2 file](https://raw.githubusercontent.com/OpenStickCommunity/GP2040-CE/main/docs/downloads/flash_nuke.uf2) onto the drive
4. ***Wait for the drive to disconnect and reconnect without unplugging***
5. Drag your firmware onto the RPI drive and wait for disconnect
6. Check that the controller connects using this [gamepad tester](https://hardwaretester.com/gamepad) and pressing a button.
