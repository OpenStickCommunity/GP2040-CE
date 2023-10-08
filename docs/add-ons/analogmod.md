# Analog Modification Input

![GP2040-CE Configuration - Add-Ons Tilt Input](assets/images/gpc-add-ons-analogmod.png)

## Tilt
Tilt 1 and Tilt 2 buttons, when pressed with directional buttons, adjust the analog values.
Default settings tilt the Left stick at 65% for Tilt 1 and 35% for Tilt 2.
The Right stick shifts down for Tilt 1 and up for Tilt 2 by default.
Tilt 1 has priority over Tilt 2 when modifying Left Stick.
Pressing Tilt 1 and Tilt 2 simultaneously with Right Stick will function as the D-Pad.
Tilt has priority over Rotate.
* `Tilt 1 Pin` - The GPIO pin used for the Tilt 1 direction.
* `Tilt 2 Pin` - The GPIO pin used for the Tilt 2 direction.
* `Tilt 1 X-Axis Factor for Left Stick`
* `Tilt 1 Y-Axis Factor for Left Stick`
* `Tilt 1 X-Axis Factor for Right Stick`
* `Tilt 1 Y-Axis Factor for Right Stick`
* `Tilt 2 X-Axis Factor for Left Stick`
* `Tilt 2 Y-Axis Factor for Left Stick`
* `Tilt 2 X-Axis Factor for Right Stick`
* `Tilt 2 Y-Axis Factor for Right Stick`

## Rotate
Rotate 1 and Rotate 2 buttons, when pressed with directional buttons, adjust the analog values.
Rotate 1 makes sticks rotate clockwise and Rotate 2 makes them rotate counter clockwise.
Rotate 1 has priority over Rotate 2 when modifying Left Stick.
Pressing Rotate 1 and Rotate 2 simultaneously with Right Stick will function as the D-Pad.
* `Rotate 1 Pin` - The GPIO pin used for the Rotate 1 (Clockwise Rotation).
* `Rotate 2 Pin` - The GPIO pin used for the Rotate 2 (Counter Clockwise Rotation).
* `Rotate 1 Degree for Left Stick`
* `Rotate 1 Degree for Right Stick`
* `Rotate 2 Degree for Left Stick`
* `Rotate 2 Degree for Right Stick`

## Analog Stick Pins
User can also set dedicated analog stick pins to work with Tilt and Rotate features.
Setting dedicated analog stick pins will overwrite the settings in DPad mode slider addon.
* `Analog Modification Left Analog Up Pin` - The GPIO pin used for the Up direction on the Left analog stick.
* `Analog Modification Left Analog Down Pin` - The GPIO pin used for the Down direction on the Left analog stick.
* `Analog Modification Left Analog Left Pin` - The GPIO pin used for the Left direction on the Left analog stick.
* `Analog Modification Left Analog Right Pin` - The GPIO pin used for the Right direction on the Left analog stick.
* `Analog Modification Right Analog Up Pin` - The GPIO pin used for the Up direction on the Right analog stick.
* `Analog Modification Right Analog Down Pin` - The GPIO pin used for the Down direction on the Right analog stick.
* `Analog Modification Right Analog Left Pin` - The GPIO pin used for the Left direction on the Right analog stick.
* `Analog Modification Right Analog Right Pin` - The GPIO pin used for the Right direction on the Right analog stick.

## SOCD
* `Analog Modification SOCD Mode` - Choose the default Analog Mod SOCD Cleaning Mode (Neutral, Last Win, Up Priority).