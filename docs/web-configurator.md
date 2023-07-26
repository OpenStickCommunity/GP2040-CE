# GP2040-CE Web Configurator

Select the button labels to be displayed in the web configurator guide: <label-selector></label-selector>

GP2040-CE contains a built-in web-based configuration application which can be started up by holding <hotkey v-bind:buttons='["S2"]'></hotkey> when plugging your controller into a PC. Then access <http://192.168.7.1> in a web browser to begin configuration. This mode is compatible with Windows, Mac, Linux and SteamOS. When using the web-based configuration on Windows and Mac, RNDIS works on a default install. Linux distributions may need some extra steps to access the web configurator; see [Linux Setup](#linux-setup).

## Home

![GP2040-CE Configurator - Home](assets/images/gpc-home.png)

Here you can see the current version of your firmware and the latest version available on GitHub in the releases section. If a firmware update is available, a link to that release will appear.

The options in the main menu are:

* [Home](#home) - The start page.
* [Settings](#settings) - Adjust settings like input mode, d-pad mode, etc.
* [Configuration > Pin Mapping](#pin-mapping) - Allows for remapping of GPIO pins to different buttons.
* [Configuration > LED Configuration](#led-configuration) - Enable and configure RGB LEDs here.
* [Configuration > Display Configuration](#display-configuration) - Enable and configure display options.
* [Configuration > Add-Ons Configuration](#add-ons-configuration) - Enable and configure available add-ons.
* [Configuration > Data Backup and Restoration](#data-backup-and-restoration) - Backup and restore settings.
* Links - Useful links to the project and documentation
* [DANGER ZONE](#danger-zone) - Don't be afraid of the big red button. If something becomes misconfigured, you can reset your settings here.

## Settings

![GP2040-CE Configurator - Settings](assets/images/gpc-settings.png)

Here you can select the basic settings which are normally available via hotkeys.

* `Input Mode` - Choose the main input mode (XINPUT, DINPUT, Switch, PS4, HID-Keyboard) this connected device will boot into when powered on.  This selection will persist through unplug / replug.
* `D-Pad Mode` - Choose the default D-Pad mode (D-Pad, Left Stick or Right Stick).
* `SOCD Cleaning Mode` - Choose the default SOCD Cleaning Mode (Neutral, Last Win, First Win, OFF).  Please note that PS4, PS3 and Nintendo Switch modes do not support setting SOCD to off and will defualt to Neutral SOCD.
* `Forced Setup Mode` - Allows you to lock out Input Mode, the ability to enter Web-Config or both.  Enabling a web-config lockout will require you to nuke and reload the firmware if you wish to make further changes.
* `4-Way Joystick Mode` - Enables 4-Way Jostick mode which will prevent cardinal directions.


### Hotkeys

An arbitrary number of buttons and directions, plus the optional Function (Fn) button, can be used to define
desired hotkey actions. Select Fn if desired, plus one or more buttons/directions, and associate them with a
hotkey action. The default hotkeys can be modified or removed, and new ones added, up to 12 in total.

The available hotkey actions will expand over time. We may also expand the number of hotkeys available to
configure in the future.

## Pin Mapping

![GP2040-CE Configurator - Pin Mapping](assets/images/gpc-pin-mapping.png)

Here you can remap the GP2040-CE buttons to different GPIO pins on the RP2040 chip. This can be used to simply remap buttons, or bypass a GPIO pin that may have issues on your device.

## LED Configuration

If you have a setup with per-button RGB LEDs, they can be configured here.

### RGB LED Configuration

![GP2040-CE Configurator - LED Configuration](assets/images/gpc-rgb-led-config.png)

* `Data Pin` - The GPIO pin that will drive the data line for your RGB LED chain. Set to `-1` to disable RGB LEDs.
* `LED Format` - The data format used to communicate with your RGB LEDs. If unsure the default `GRB` value is usually safe.
* `LED Layout` - Select the layout for your controls/buttons. This is used for static themes and some per-button animations.
* `LEDs Per Button` - Set the number of LEDs in each button on your chain.
* `Max Brightness` - Set the maximum brightness for the LEDs. Ranges from 0-255.
* `Brightness Steps` - The number of levels of brightness to cycle through when turning brightness up and down.

### RGB LED Button Order

!> Please note that RGB Button LEDs must be the first LEDs configured. They will start at index 0 on the RGB LED strip.

![GP2040-CE Configurator - RGB LED Button Order](assets/images/gpc-rgb-led-button-order.png)

* `LED Button Order` - Configure which buttons and what order they reside on the LED chain.

### Player LEDs (XInput)

Available selections for `Player LED Type` are `None`, `PWM` or `RGB`.

#### PWM Player LEDs

![GP2040-CE Configurator - PWM Player LEDs](assets/images/gpc-pled-pwm.png)

* `PLED #[1-4] Pin` - The GPIO pin the standard LED is connected to.

#### RGB Player LEDs

!> Please note that RGB Player LEDs must be located at an index after the RGB LED Buttons on the LED strip! The Web Config interface will suggest a starting index based on the number of LED buttons mapped in [RGB LED Button Order](#rgb-led-button-order) and the select `LEDs Per Button` value. We hope to remove this limitation in the future.

![GP2040-CE Configurator - PWM Player LEDs](assets/images/gpc-pled-rgb.png)

* `PLED #[1-4] Index` - The index of the LED module on the RGB strip.
* `RGB PLED Color` - Click the box to reveal a color picker, or manually enter the color.

## Custom LED Theme

![GP2040-CE Configurator - Custom LED Theme](assets/images/gpc-rgb-led-custom-theme.png)

* `Enable` - Enables the use of Custom LED Theme.
* `Preview Layout` - Predefined layouts for previewing LED theme. **NOTE:** This is for preview only, does not affect controller operation.
* `Clear All` - Prompts for confirmation to reset the current theme to all buttons black (LEDs off). Make sure you have saved and have a backup if you don't want to lose your customizations.
* `Set All To Color` - Presents a color picker to set all buttons to the same normal or pressed color.
* `Set Gradient` - Sets a horizontal gradient across the action buttons according to the `Preview Layout` selection.
* `Set Pressed Gradient` - Same as `Set Gradient`, but for pressed button state.
* `Save Color` - Save a custom color to the color picker palette.
* `Delete Color` - Deletes a custom color from the color picker palette. Stock colors cannot be deleted.

?> All saved colors and gradient selections are saved to your browser's local storage.

If enabled, the Custom LED Theme will be available as another animation mode and will cycle with the `Previous Animation` and `Next Animation` shortcuts on your controller. You can also use the [Data Backup and Restoration](#data-backup-and-restoration) feature to create and share themes!

## Display Configuration

![GP2040-CE Configurator - Display Configuration](assets/images/gpc-display-config.png)

* `Use Display` - Turns on/off the display module.
* `I2C Block` - The Pico I2C block that will be used. Set based on pins, refer to table on page.
* `SDA Pin` - The GPIO pin used for the I2C SDA channel.
* `SCL Pin` - The GPIO pin used for the I2C SCL channel.
* `I2C Address` - The I2C address of your device, defaults to the very commonly used `0x3C`.
* `I2C Speed` - Sets the speed of I2C communication. Common values are `100000` for standard, `400000` for fast and `800000` ludicrous speed.
* `Flip Display` - Allows you to flip or mirror the display in a variety of ways.
* `Invert Display` - Inverts the pixel colors, effectively giving you a negative image when enabled.
* `Button Layout (Left)` - Changes the onscreen layout for the left side of the display and stick.
* `Button Layout (Right)` - Changes the onscreen layout for the right side of the display and stick.
* `Splash Mode` - Enables or disables a splash screen displaying when the unit is turned on.
* `Splash Duration` - Sets the amount of time the splash screen displays for on boot.
* `Display Saver Timeout` - Will cause the display to turn off after the specified number of minutes. Pressing any input will cause the display to turn back on.
* `Choose File` - This will allow you to upload your own image to be used for the splash screen. It is recommend that you use a two color 128x64 image (or one that is sized appropriately for your display). Uploading any other type of image will result in a conversion and sizing of the image automatically. If the image is inverted upon upload, just check off the `Invert` box.

Check out our collection of great custom splash screens from the community [HERE](community-splash-screens.md)

## Add-Ons Configuration

This section is for custom add-ons that can be enabled to expand the functionality of GP2040-CE.  Please note that not all add-ons may work together.  These should be considered experimental.

### BOOTSEL Button Configuration

![GP2040 Configurator - Add-Ons BOOTSEL Button Configuration](assets/images/gpc-add-ons-bootsel-button.png)

* `BOOTSEL Button` - Choose an input to be mapped to the BOOTSEL button.

Please note that this can only be used on devices that have a BOOTSEL button.  Please also note that the OLED might become unresponsive if this button is set. You can unset it to restore OLED functionality.

### On-board LED Configuration

![GP2040 Configurator - Add-Ons On-Board LED Configuration](assets/images/gpc-add-ons-onboard-led.png)

* `Off` - LED is off
* `Mode Indicator` - LED is solid if unit is powered with connected data, LED blinks rapidly if powered with no data, LED blinks slowly when in web-config mode.
* `Input Test` - LED is off but turns on when any input is pressed (LED turns off when button is released).

### Analog

![GP2040 Configurator - Add-Ons Analog](assets/images/gpc-add-ons-analog.png)

* `Analog Stick 1 X Pin` - The GPIO pin used for the Analog Stick 1 X value.  Only ADC pins 26, 27, 28 and 29 are allowed here.
* `Analog Stick 1 Y Pin` - The GPIO pin used for the Analog Stick 1 Y value.  Only ADC pins 26, 27, 28 and 29 are allowed here.
* `Analog Stick 1 Mode` - Choose if Analog Stick 1 is to be used for Left Analog or Right Analog.  
* `Analog Stick 1 Invert` - Choose if you would like to flip the X or Y axis Analog Stick 1 inputs (or both).
* `Analog Stick 2 X Pin` - The GPIO pin used for the Analog Stick 2 X value.  Only ADC pins 26, 27, 28 and 29 are allowed here.
* `Analog Stick 2 Y Pin` - The GPIO pin used for the Analog Stick 2 Y value.  Only ADC pins 26, 27, 28 and 29 are allowed here.
* `Analog Stick 2 Mode` - Choose if Analog Stick 2 is to be used for Left Analog or Right Analog (must be different than Analog Stick 1).
* `Analog Stick 2 Invert` - Choose if you would like to flip the X or Y axis Analog Stick 2 inputs (or both).
* `Deadzone Size (%)` - Enter the % value of deadzone you would like on the analog sticks.
* `Forced Circularity` - Force the analog sticks to be bound within a perfect circle. This can be beneficial for certain games. However, be aware that this may negatively impact some games which account for sticks moving outside of a circle.
* `Auto Calibration` - Automatically centers the analog sticks. This works by reading in the offset from center during boot and then accounts for that until the next power cycle. This can be helpful for analog sticks experiencing drift.



### Turbo

![GP2040-CE Configurator - Add-Ons Turbo](assets/images/gpc-add-ons-turbo.png)

* `Turbo Pin` - The GPIO pin used for the Turbo button.
* `Turbo Pin LED` - The GPIO pin used for the Turbo LED.
* `Turbo Shot Count` - The number of of presses per second that the Turbo will activate at.
* `Turbo Dial (ADC ONLY)` - The GPIO pin used for the Turbo dial.  Must be one of the ADC pins.

### Turbo - SHMUP MODE

![GP2040 Configurator - Add-Ons Turbo SHMUP MODE](assets/images/gpc-add-ons-turbo-shmup.png)

* `Turbo Always On 1` - The GPIO pin used for a Turbo button that will always be on.
* `Turbo Always On 2` - The GPIO pin used for a Turbo button that will always be on.
* `Turbo Always On 3` - The GPIO pin used for a Turbo button that will always be on.
* `Turbo Always On 4` - The GPIO pin used for a Turbo button that will always be on.
* `Charge Button 1 Pin` - The GPIO pin used for a button that needs to be able to do a charged shot.
* `Charge Button 2 Pin` - The GPIO pin used for a button that needs to be able to do a charged shot.
* `Charge Button 3 Pin` - The GPIO pin used for a button that needs to be able to do a charged shot.
* `Charge Button 4 Pin` - The GPIO pin used for a button that needs to be able to do a charged shot.
* `Charge Button 1 Assignment` - The button that will be able to charge shot regardless of Turbo status.
* `Charge Button 2 Assignment` - The button that will be able to charge shot regardless of Turbo status.
* `Charge Button 3 Assignment` - The button that will be able to charge shot regardless of Turbo status.
* `Charge Button 4 Assignment` - The button that will be able to charge shot regardless of Turbo status.
* `Simultaneous Priority Mode` - In the event both the Turbo and charged buttons are pressed at the time same, which should take priority.

### Joystick Selection Slider

![GP2040-CE Configurator - Add-Ons Joystick Slider](assets/images/gpc-add-ons-joystick-slider.png)

* `Slider LS Pin` - The GPIO pin used to activate the Left Stick while held.
* `Slider RS Pin` - The GPIO pin used to activate the Right Stick while held.

### Input Reverse

![GP2040-CE Configurator - Add-Ons Input Reverse](assets/images/gpc-add-ons-input-reverse.png)

* `Reverse Input Pin` - The GPIO pin used for the Reverse Input button.
* `Reverse Input Pin LED` - The GPIO pin used for the Reverse Input LED.
* `Reverse Up` - Enables the up direction to be reversed when the Reverse Input button is pressed or held.
* `Reverse Down` - Enables the down direction to be reversed when the Reverse Input button is pressed or held.
* `Reverse Left` - Enables the left direction to be reversed when the Reverse Input button is pressed or held.
* `Reverse Right` - Enables the right direction to be reversed when the Reverse Input button is pressed or held.

### I2C Analog ADS1219

![GP2040-CE Configurator - Add-Ons ADS1219](assets/images/gpc-add-ons-ads1219.png)

* `I2C Analog ADS1219 SDA Pin` - The GPIO pin used for I2C Analog ADS1219 SDA.
* `I2C Analog ADS1219 SCL Pin` - The GPIO pin used for I2C Analog ADS1219 SCL.
* `I2C Analog ADS1219 Block` - The block of I2C to use (i2c0 or i2c1).
* `I2C Analog ADS1219 Speed` - Sets the speed of I2C communication. Common values are `100000` for standard, `400000` for fast and `800000` ludicrous speed.
* `I2C Analog ADS1219 Address` - Sets the address for the I2C Analog ADS1219.

### Dual Directional Input

![GP2040 Configuration - Add-Ons Dual Directional Input](assets/images/gpc-add-ons-dual-directional.png)

* `Dual Up Pin` - The GPIO pin used for the secondary Up direction.
* `Dual Down Pin` - The GPIO pin used for the secondary Down direction.
* `Dual Left Pin` - The GPIO pin used for the secondary Left direction.
* `Dual Right Pin` - The GPIO pin used for the secondary Right direction.
* `Dual D-Pad Mode` - Choose if this should act as an additional instance of the D-Pad or as the Left or Right stick.
Values are:
`D-PAD` for D-PAd mode.
`Left Analog` for Left Analog stick mode.
`Right Analog` for Right Analog stick mode.
* `Combination Mode` - Choose how these inputs should be combined.
Values are:
`Mixed` - Combines both the Gamepad input and Dual Directional and allows for all 3 SOCD modes.
`Gamepad` - Gamepad always takes over when pressed, otherwise Gamepad and Dual act independently.
`Dual Directional` - Dual always takes over when pressed, otherwise Gamepad and Dual act independently.
`None` - Gamepad input and dual directional act independently of each other.

### Buzzer Speaker

![GP2040-CE Configurator - Add-Ons Buzzer](assets/images/gpc-add-ons-buzzer.png)

* `Use buzzer` - Turns on/off the buzzer module.
* `Buzzer Pin` - The GPIO pin used for the buzzer.
* `Buzzer Volume` - Audio volume of buzzer. Ranges from 0-100.

### Extra Button Configuration

![GP2040 Configurator - Extra Button](assets/images/gpc-add-ons-extra-button.png)

* `Extra Button Pin` - The GPIO pin used for the extra instance of a button.
* `Extra Button` - The button that will have an extra instance.

### Player Number (X-INPUT ONLY)

![GP2040 Configurator - Player Number](assets/images/gpc-add-ons-player-number.png)

* `Player Number` - Choose what player number this RP2040 based device will be representing.  This is only for X-INPUT mode.

### SOCD Selection Slider

![GP2040 Configurator - SOCD Selection Slider](assets/images/gpc-add-ons-socd-slider.png)

* `SOCD Slider Mode Default` - The default SOCD mode to be used when the slider pin is not activated.
* `SOCD Slider Mode Tne` - The SOCD mode you would like to have enabled for the first slder position.
* `Pin One` - The GPIO pin used for first SOCD mode slider position.
* `SOCD Slider Mode Two` - The SOCD mode you would like to have enabled for the second slder position.
* `Pin Two` - The GPIO pin used for second SOCD mode slider position.

### PS4 Mode

![GP2040 Configurator - PS4 Mode](assets/images/gpc-add-ons-ps4-mode.png)

Please note that GP2040-CE will never provide these files!

Enabling this add-on will allow you to use GP2040-CE on a PS4 with an 8 minute timeout.  If you have the necessary files to upload in this section it will authenticate to a native PS4 device and not time out after 8 minutes.

* `Private Key (PEM)` - Choose your PEM file.
* `Serial Number (16 Bytes in Hex Ascii)` - Choose your serial number file.
* `Signature (256 Bytes in Binary)` - Choose your signature file.

### Wii Extensions

![GP2040 Configurator - Wii Extensions](assets/images/gpc-add-ons-wii-extensions.png)

* `I2C SDA Pin` - The GPIO pin used for Wii Extension SDA.
* `I2C SCL Pin` - The GPIO pin used for Wii Extension SCL.
* `I2C Block` - The block of I2C to use (i2c0 or i2c1).
* `I2C Speed` - Sets the speed of I2C communication. Common values are `100000` for standard, or `400000` for fast.

Supported Extension Controllers and their mapping is as follows:

| GP2040-CE | Nunchuck | Classic      | Guitar Hero Guitar |
|-----------|----------|--------------|--------------------|
| B1        | C        | B            | Green              |
| B2        | Z        | A            | Red                |
| B3        |          | Y            | Blue               |
| B4        |          | X            | Yellow             |
| L1        |          | L            |                    |
| L2        |          | ZL           |                    |
| R1        |          | R            |                    |
| R2        |          | ZR           |                    |
| S1        |          | Select       |                    |
| S2        |          | Start        |                    |
| A1        |          | Home         |                    |
| D-Pad     |          | D-Pad        | Strum Up/Down      |
| Analog    | Left     | Left & Right | Left               |

Classic Controller support includes Classic, Classic Pro, and NES/SNES Mini Controllers. 

Original Classic Controller L & R triggers are analog sensitive, where Pro triggers are not.

### SNES Input

![GP2040 Configurator - SNES Input](assets/images/gpc-add-ons-snespad-input.png)

* `CLOCK Pin` - The GPIO pin used for SNES CLOCK.
* `LATCH Pin` - The GPIO pin used for SNES LATCH.
* `DATA Pin` - The GPIO pin used for SNES DATA.

Supported controller types and their mapping is as follows:

| GP2040-CE | NES      | SNES         | Super NES Mouse    |
|-----------|----------|--------------|--------------------|
| B1        | B        | B            | Left Click         |
| B2        | A        | A            | Right Click        |
| B3        |          | Y            |                    |
| B4        |          | X            |                    |
| L1        |          | L            |                    |
| L2        |          |              |                    |
| R1        |          | R            |                    |
| R2        |          |              |                    |
| S1        | Select   | Select       |                    |
| S2        | Start    | Start        |                    |
| A1        |          |              |                    |
| D-Pad     | D-Pad    | D-Pad        |                    |
| Analog    |          |              | Mouse Movement     |

### Focus Mode Configuration

![GP2040 Configurator - Focus Mode](assets/images/gpc-add-ons-focus-mode.png)

* `Focus Mode Pin` - The GPIO pin used to enable Focus Mode (this needs to always be held so a slider or latching switch is recommended).
* `Lock OLED Screen` - When enabled the OLED screen will not display anything during Focus Mode.
* `Lock RGB LED` - When enabled the RGB LEDs that are controlled by the RP2040 device will not display anything during Focus Mode.
* `Lock Buttons` - When enabled the You can specify specific buttons to not function during Focus Mode.  You can add as many additional buttons as needed here.

## Data Backup and Restoration

![GP2040-CE Configurator - Add-Ons Backup and Restore](assets/images/gpc-backup-and-restore.png)

* `Backup To File` - Allows you to select what to backup to a file (default is all selected).
* `Restore From File` - Allows you to select what to restore from a file (default is all selected).

## DANGER ZONE

![GP2040-CE Configurator - Reset Settings](assets/images/gpc-reset-settings.png)

# Linux Setup

When you plug in your controller while holding <hotkey v-bind:buttons='["S2"]'></hotkey>, you should see it connect in the kernel logs if you run `dmesg`:

```sh
[   72.291060] usb 1-3: new full-speed USB device number 12 using xhci_hcd
[   72.450166] usb 1-3: New USB device found, idVendor=cafe, idProduct=4028, bcdDevice= 1.01
[   72.450172] usb 1-3: New USB device strings: Mfr=1, Product=2, SerialNumber=3
[   72.450174] usb 1-3: Product: TinyUSB Device
[   72.450176] usb 1-3: Manufacturer: TinyUSB
[   72.450177] usb 1-3: SerialNumber: 123456
[   72.484285] rndis_host 1-3:1.0 usb0: register 'rndis_host' at usb-0000:06:00.1-3, RNDIS device, 02:02:84:6a:96:00
[   72.498630] rndis_host 1-3:1.0 enp6s0f1u3: renamed from usb0
```

In the above example, **enp6s0f1u3** is the virtual Ethernet interface for your controller. If you don't see the first `rndis_host` line, make sure `CONFIG_USB_NET_RNDIS_HOST` is compiled in your kernel or as a module.

The web configurator is automatically running, you just need to be able to reach it. Some configurations automatically set up the route, so try <http://192.168.7.1> in your browser now. If it doesn't load, try configuring an IP for the interface manually via: `sudo ifconfig enp6s0f1u3 192.168.7.2`.

Whether or not you had to add an IP manually, you should end up with a route something like this:

```sh
% ip route
default via 10.0.5.1 dev enp5s0 proto dhcp src 10.0.5.38 metric 2
10.0.5.0/24 dev enp5s0 proto dhcp scope link src 10.0.5.38 metric 2
192.168.7.0/24 dev enp6s0f1u3 proto kernel scope link src 192.168.7.2     <---
```

Then the configurator should be reachable in your browser.
