# Analog Modification Input

![GP2040-CE Configuration - Add-Ons Tilt Input](assets/images/gpc-add-ons-analogmod.png)

## Analog Stick Pins
User can set dedicated analog stick pins to work with Tilt and Rotate features.
Setting dedicated analog stick pins will overwrite the settings in DPad mode slider addon.
If no dedicated analog stick pins are set, Tilt and Rotate features will function when DPad is used as Left or Right Analog.
* `Analog Modification Left Analog Up Pin` - The GPIO pin used for the Up direction on the Left analog stick.
* `Analog Modification Left Analog Down Pin` - The GPIO pin used for the Down direction on the Left analog stick.
* `Analog Modification Left Analog Left Pin` - The GPIO pin used for the Left direction on the Left analog stick.
* `Analog Modification Left Analog Right Pin` - The GPIO pin used for the Right direction on the Left analog stick.
* `Analog Modification Right Analog Up Pin` - The GPIO pin used for the Up direction on the Right analog stick.
* `Analog Modification Right Analog Down Pin` - The GPIO pin used for the Down direction on the Right analog stick.
* `Analog Modification Right Analog Left Pin` - The GPIO pin used for the Left direction on the Right analog stick.
* `Analog Modification Right Analog Right Pin` - The GPIO pin used for the Right direction on the Right analog stick.

## Tilt
Tilt 1 and Tilt 2 buttons, when pressed with directional buttons, adjust the analog values.
Pressing Tilt 1 or 2 with a value of N and a direction results in an input that is N% of the maximum direction.
Pressing Tilt 1 and Tilt 2 simultaneously with Left Stick will have no impact.
Pressing Tilt 1 and Tilt 2 simultaneously with Right Stick will make it function as the D-Pad.

* Left Analog Stick (LS Tilt 1 Factor XY = 35) - Input Left while holding Tilt 1 = Send input at 35% Left
* Left Analog Stick (LS Tilt 1 Factor XY = 35) - Input Up while holding Tilt 1 = Send input at 35% Up
* Left Analog Stick (LS Tilt 2 Factor XY = 65) - Input Left while holding Tilt 1 = Send input at 65% Left
* Left Analog Stick (LS Tilt 2 Factor XY = 65) - Input Up while holding Tilt 1 = Send input at 65% Up
* Right Analog Stick (RS Tilt 1 Factor XY = 35) - Input Left while holding Tilt 1 = Send input at 35% Left
* Right Analog Stick (RS Tilt 1 Factor XY = 35) - Input Up while holding Tilt 1 = Send input at 35% Up
* Right Analog Stick (RS Tilt 2 Factor XY = 65) - Input Left while holding Tilt 1 = Send input at 65% Left
* Right Analog Stick (RS Tilt 2 Factor XY = 65) - Input Up while holding Tilt 1 = Send input at 65% Up

## Rotate
Rotate 1 and Rotate 2 buttons, when pressed with directional buttons, adjust the analog values.
Both Rotate 1 and Rotate 2 will make analog sticks rotate clockwise.
Pressing Rotate 1 and Rotate 2 simultaneously with Left Stick will have no impact.
Pressing Rotate 1 and Rotate 2 simultaneously with Right Stick will make it function as the D-Pad.
* `Rotate 1 Pin` - The GPIO pin used for the Rotate 1 (Clockwise Rotation).
* `Rotate 2 Pin` - The GPIO pin used for the Rotate 2 (Clockwise Rotation).
* `Rotate 1 Degree for Left Stick` - Clockwise rotate degree for Left Stick when Pressing Rotate 1
* `Rotate 1 Degree for Right Stick` - Clockwise rotate degree for Right Stick when Pressing Rotate 1
* `Rotate 2 Degree for Left Stick` - Clockwise rotate degree for Left Stick when Pressing Rotate 2
* `Rotate 2 Degree for Right Stick` - Clockwise rotate degree for Right Stick when Pressing Rotate 2

## SOCD
* `Analog Modification SOCD Mode` - Choose the default Analog Mod SOCD Cleaning Mode (Neutral, Last Win, Up Priority).