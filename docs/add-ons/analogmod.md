# Analog Modification Input

![GP2040-CE Configuration - Add-Ons Tilt Input](assets/images/gpc-add-ons-analogmod.png)

## Dedicated Analog Stick Pins
User can set dedicated analog stick pins to work with Tilt and Rotate features.
If no dedicated analog stick pins are set, Tilt and Rotate features will function when DPad is used as Left or Right Analog.
Otherwise, dedicated analog stick pins will have priority over DPad used as left or right analog sticks
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
Pressing Tilt 1 and Tilt 2 simultaneously with LS or RS directional buttons will make them function as the D-Pad.

* `Tilt 1 Pin` - The GPIO pin used for the Tilt 1 direction.
* `Tilt 1 Factor Left X` - The percentage of the X-axis input for the Left analog stick sent when `Tilt 1 Pin` is activated. (Min. 0, Max 100)
* `Tilt 1 Factor Left Y` - The percentage of the Y-axis input for the Left analog stick sent when `Tilt 1 Pin` is activated. (Min. 0, Max 100)
* `Tilt 1 Factor Left X` - The percentage of the X-axis input for the Right analog stick sent when `Tilt 1 Pin` is activated. (Min. 0, Max 100)
* `Tilt 1 Factor Left Y` - The percentage of the Y-axis input for the Right analog stick sent when `Tilt 1 Pin` is activated. (Min. 0, Max 100)
* `Tilt 2 Pin` - The GPIO pin used for the Tilt 2 direction.
* `Tilt 2 Factor Left X` - The percentage of the X-axis input for the Left analog stick sent when `Tilt 2 Pin` is activated. (Min. 0, Max 100)
* `Tilt 2 Factor Left Y` - The percentage of the Y-axis input for the Left analog stick sent when `Tilt 2 Pin` is activated. (Min. 0, Max 100)
* `Tilt 2 Factor Left X` - The percentage of the X-axis input for the Right analog stick sent when `Tilt 2 Pin` is activated. (Min. 0, Max 100)
* `Tilt 2 Factor Left Y` - The percentage of the Y-axis input for the Right analog stick sent when `Tilt 2 Pin` is activated. (Min. 0, Max 100)

## Rotate
Rotate 1 and Rotate 2 buttons, when pressed with directional buttons, adjust the analog values.
Both Rotate 1 and Rotate 2 will make analog sticks rotate clockwise.
Pressing Rotate 1 and Rotate 2 simultaneously with LS or RS directional buttons will make them function as the D-Pad.
* `Rotate 1 Pin` - The GPIO pin used for the Rotate 1 (Clockwise Rotation).
* `Rotate 2 Pin` - The GPIO pin used for the Rotate 2 (Clockwise Rotation).
* `Rotate 1 Degree for Left Stick` - Clockwise rotate degree for Left Stick when Pressing Rotate 1
* `Rotate 1 Degree for Right Stick` - Clockwise rotate degree for Right Stick when Pressing Rotate 1
* `Rotate 2 Degree for Left Stick` - Clockwise rotate degree for Left Stick when Pressing Rotate 2
* `Rotate 2 Degree for Right Stick` - Clockwise rotate degree for Right Stick when Pressing Rotate 2

## SOCD
* `Analog Mod Left Stick SOCD Mode` - Choose the SOCD Cleaning Mode for Left Stick (Up Priority, Neutral, Last Win, First Win).
* `Analog Mod Right Stick SOCD Mode` - Choose the SOCD Cleaning Mode for Right Stick (Up Priority, Neutral, Last Win, First Win).