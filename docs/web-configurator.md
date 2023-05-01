# GP2040-CE Web Configurator

Select the button labels to be displayed in the web configurator guide: <label-selector></label-selector>

GP2040-CE contains a built-in web-based configuration application which can be started up by holding <hotkey v-bind:buttons='["S2"]'></hotkey> when plugging your controller into a PC. Then access <http://192.168.7.1> in a web browser to begin configuration. This mode is compatible with Windows, Mac, Linux and SteamOS.

## Home

![GP2040-CE Configurator - Home](assets/images/gpc-home.png)

Here you can see the current version of your firmware and the latest version available on Github in the releases section. If a firmware update is available, a link to that release will appear.

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

## Pin Mapping

![GP2040-CE Configurator - Pin Mapping](assets/images/gpc-pin-mapping.png)

Here you can remap the GP2040-CE buttons to different GPIO pins on the RP2040 chip. This can be used to simply remap buttons, or bypass a GPIO pin that may have issues on your device.

## LED Configuration

If you have a setup with per-button RGB LEDs, they can be configured here.

![GP2040-CE Configurator - LED Configuration](assets/images/gpc-rgb-led-config.png)

* `Data Pin` - The GPIO pin that will drive the data line for your RGB LED chain. Set to `-1` to disable RGB LEDs.
* `LED Format` - The data format used to communicate with your RGB LEDs. If unsure the default `GRB` value is usually safe.
* `LED Layout` - Select the layout for your controls/buttons. This is used for static themes and some per-button animations.
* `LEDs Per Button` - Set the number of LEDs in each button on your chain.
* `Max Brightness` - Set the maximum brightness for the LEDs. Ranges from 0-255.
* `Brightness Steps` - The number of levels of brightness to cycle through when turning brightness up and down.
* `LED Button Order` - Configure which buttons and what order they reside on the LED chain.

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
* `Display Saver Timeout` - Will cause the display to turn off afer the specified number of minuites.  Pressing any input will cause the diplay to turn back on.
* `Choose File` - This will allow you to upload your own image to be used for the splash screen.  It is recommend that you use a two color 128x64 image (or one that is sized appropiatly for your display).  Uploading any other type of image will result in a conversion and sizing of the image automatically.  If the image is inverted upon upload, just check off the `Invert` box.

Check out our collection of great custom splash screens from the community [HERE](community-splash-screens.md)

## Add-Ons Configuration

This section is for custom add-ons that can be enabled to expand the functionality of GP2040-CE.  Please note that not all add-ons may work together.  These should be considered experimental.

### BOOTSEL Button Configuration

![GP2040 Configurator - Add-Ons BOOTSEL Button Configuration](assets/images/gpc-add-ons-bootsel-button.png)

* `BOOTSEL Button` - Choose an input to be mapped to the BOOTSEL button.

Please note that this can only be used on devices that have a BOOTSEL button.  Please also note that the OLED might become unresponsive if this button is set.  You can unset it to restore OLED fuctionality.

### On-board LED Configuration

![GP2040 Configurator - Add-Ons On-Board LED Configuration](assets/images/gpc-add-ons-onboard-led.png)

* `Off` - LED is off
* `Mode Indicator` - LED is solid if unit is powered with connected data, LED blinks rapidly if powered with no data, LED blinks slowly when in web-config mode.
* `Input Test` - LED is off but turns on when any input is pressed (LED turns off when button is released).

### Analog

![GP2040 Configurator - Add-Ons Analog](assets/images/gpc-add-ons-analog.png)

* `Analog Stick X Pin` - The GPIO pin used for the Analog Stick X value.
* `Analog Stick Y Pin` - The GPIO pin used for the Analog Stick Y value.

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
`Dual Directional` - Dual always takes over when pressed, otherwise Gamepad and Dual act indepedently.
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

* `Slider SOCD Up Priority Pin` - The GPIO pin used for SOCD Up Priority.
* `Slider SOCD Second Input Priority Pin` - The GPIO pin used for SOCD Second Input Priority.

### PS4 Mode

![GP2040 Configurator - PS4 Mode](assets/images/gpc-add-ons-ps4-mode.png)

Please note that GP2040-CE will never provide these files!

Enabling this add-on will allow you to use GP2040-CE on a PS4 with an 8 minute timeout.  If you have the necessary files to upload in this section it will authenticate to a native PS4 device and not time out after 8 minutes.

* `Private Key (PEM)` - Choose your PEM file.
* `Serial Number (16 Bytes in Hex Ascii)` - Choose your serial number file.
* `Signature (256 Bytes in Binary)` - Choose your signature file.

## Data Backup and Restoration

![GP2040-CE Configurator - Add-Ons Backup and Restore](assets/images/gpc-backup-and-restore.png)

* `Backup To File` - Allows you to select what to backup to a file (default is all selected).
* `Restore From File` - Allows you to select what to restore from a file (default is all selected).

## DANGER ZONE

![GP2040-CE Configurator - Reset Settings](assets/images/gpc-reset-settings.png)

