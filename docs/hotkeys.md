# Hotkeys

Select the button labels to be displayed in the usage guide: <label-selector></label-selector>

> All of the following hotkey bindings can be changed or assigned to different button combinations in [Hotkeys Settings](web-configurator.md#hotkey-settings) section of the Web Configurator.

## No Action

This hotkey does nothing. Use this hotkey to disable a specific hotkey combination.

## DPad Digital

This hotkey changes the DPad mode such the directions to be read as digital inputs on the DPad.

**Default**: <hotkey v-bind:buttons='["S1", "S2", "Down"]'></hotkey>

## DPad Left Analog

This hotkey changes the DPad mode to such the directions to be read as inputs on the Left Analog Stick. The directional input is always the minimum, maximum, or zero value on that axis.

**Default**: <hotkey v-bind:buttons='["S1", "S2", "Left"]'></hotkey>

## DPad Right Analog

This hotkey changes the DPad mode to such the directions to be read as inputs on the Right Analog Stick. The directional input is always the minimum, maximum, or zero value on that axis.

**Default**: <hotkey v-bind:buttons='["S1", "S2", "Right"]'></hotkey>

## Home Button

This hotkey emulates a press of the <hotkey v-bind:buttons='["A1"]'></hotkey> button as not all controllers may have this button natively on the controller.

## Capture Button

This hotkey emulates a press of the <hotkey v-bind:buttons='["A2"]'></hotkey> button as not all controllers may have this button natively on the controller.

## SOCD Up Priority

This hotkey changes the SOCD cleaning method to resolve to a neutral input (no input) on the X-axis, but prioritize the `Up` input on the Y-axis when both directions are pressed simultaneously. 

| 1st Input + 2nd Input |  Result |
|:---------------------:|:-------:|
|      Left + Right     | Neutral |
|      Right + Left     | Neutral |
|       Up + Down       |    Up   |
|       Down + Up       |    Up   |

## SOCD Neutral

This hotkey changes the SOCD cleaning method to resolve to a neutral input (no input) on both the X-axis and Y-axis when both directions are pressed simultaneously. 

| 1st Input + 2nd Input |  Result |
|:---------------------:|:-------:|
|      Left + Right     | Neutral |
|      Right + Left     | Neutral |
|       Up + Down       | Neutral |
|       Down + Up       | Neutral |

## SOCD Last Win

This hotkey changes the SOCD cleaning method to prioritize the second directional input on both the X-axis and Y-axis when both directions are pressed simultaneously. 

| 1st Input + 2nd Input | Result |
|:---------------------:|:------:|
|      Left + Right     |  Right |
|      Right + Left     |  Left  |
|       Up + Down       |  Down  |
|       Down + Up       |   Up   |

## SOCD First Wins

This hotkey changes the SOCD cleaning method to prioritize the first directional input on both the X-axis and Y-axis when both directions are pressed simultaneously. 

| 1st Input + 2nd Input | Result |
|:---------------------:|:------:|
|      Left + Right     |  Left  |
|      Right + Left     |  Right |
|       Up + Down       |   Up   |
|       Down + Up       |  Down  |

## SOCD Cleaning Off

This hotkey changes the SOCD cleaning method to not resolve any directional inputs on both the X-axis and Y-axis when both directions are pressed simultaneously. The controller will send both directional inputs. 

?> This cleaning mode is only available in XInput mode as PS3/DirectInput, PS4 and Nintendo Switch modes do not support setting SOCD Cleaning to Off and will default to Neutral SOCD Cleaning mode.

| 1st Input + 2nd Input |     Result     |
|:---------------------:|:--------------:|
|      Left + Right     | Left and Right |
|      Right + Left     | Left and Right |
|       Up + Down       |   Up and Down  |
|       Down + Up       |   Up and Down  |

## Invert X Axis

This hotkey will invert the X-axis of your controller (i.e. pressing the Right button will result a Left input and vice versa).

## Invert Y Axis

This hotkey will invert the Y-axis of your controller (i.e. pressing the Up button will result in a Down input and vice versa).

**Default**: <hotkey v-bind:buttons='["S2", "A1", "Right"]'></hotkey>

## Toggle 4-Way Joystick Mode

This hotkey will alter the behavior of the directional buttons such that only the cardinal directions will register as inputs and diagonal directions will be disabled and ignored.

## Toggle DDI 4-Way Joystick Mode

This hotkey will alter the behavior of the directional buttons mapped with the [Dual Direction Input](add-ons.md#dual-directional-input) add-on such that only the cardinal directions will register as inputs and diagonal directions will be disabled and ignored.

## Load Profile # 1-4

This hotkey will load various input to pin mapping profiles set in [Profile Settings](web-configurator#profile-settings) in the Web Configurator.

## L3 Button

This hotkey emulates a press of the <hotkey v-bind:buttons='["L3"]'></hotkey> button as not all controllers may have this button natively on the controller.

## R3 Button

This hotkey emulates a press of the <hotkey v-bind:buttons='["R3"]'></hotkey> button as not all controllers may have this button natively on the controller.

## Touchpad Button

This hotkey emulates a press of the <hotkey v-bind:buttons='["A2"]'></hotkey> button as not all controllers may have this button natively on the controller.

## Restart GP2040-CE

This hotkey will restart the controller without pressing the hardware reset button on the board (if present) or requiring the controller to be unplugged from the PC or game console.