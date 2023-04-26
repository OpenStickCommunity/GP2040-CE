# GP2040 Shortcuts

Select the button labels to be displayed in the usage guide: <label-selector></label-selector>

## Main Button Inputs

| GP2040  | XInput | Switch  | PS3          | DirectInput  | Arcade | Keyboard    |
| ------- | ------ | ------- | ------------ | ------------ | ------ | ----------- |
| B1      | A      | B       | Cross        | 2            | K1     | Left Shift  |
| B2      | B      | A       | Circle       | 3            | K2     | Z           |
| B3      | X      | Y       | Square       | 1            | P1     | Left Control|
| B4      | Y      | X       | Triangle     | 4            | P2     | Left Alt    |
| L1      | LB     | L       | L1           | 5            | P4     | C           |
| R1      | RB     | R       | R1           | 6            | P3     | Spacebar    |
| L2      | LT     | ZL      | L2           | 7            | K4     | V           |
| R2      | RT     | ZR      | R2           | 8            | K3     | X           |
| S1      | Back   | Minus   | Select       | 9            | Coin   | 5           |
| S2      | Start  | Plus    | Start        | 10           | Start  | 1           |
| L3      | LS     | LS      | L3           | 11           | LS     | Equal       |
| R3      | RS     | RS      | R3           | 12           | RS     | Minus       |
| A1      | Guide  | Home    | -            | 13           | -      | 9           |
| A2      | -      | Capture | -            | 14           | -      | F2          |

## Button Input Hotkeys

The RGB LED hotkeys can be found [HERE](https://gp2040-ce.info/#/usage?id=rgb-led-hotkeys).

Home button shortcut if you do not have a Home button - <hotkey v-bind:buttons='["S1", "S2", "Up"]'></hotkey>.

Unlike other controllers, Keyboard gets different keys for directional buttons.

| Direction | Keyboard   |
| --------- | ---------- |
| Up        | Up Arrow   |
| Down      | Down Arrow |
| Left      | Left Arrow |
| Right     | Right Arrow|

> You can change the key mappings for Keyboard mode in [Webconfig mode](web-configurator.md)

Bootsel mode on plug-in (to flash your controller for example), hold the <hotkey v-bind:buttons='["S1", "S2", "Up"]'></hotkey> button combination then plug in your controller.

[Webconfig mode](web-configurator.md) on plug-in (to flash your controller for example), hold the <hotkey v-bind:buttons='["S2"]'></hotkey> button then plug in your controller.


Input mode change on plug-in, **hold one of the following buttons as the controller is plugged in:**

* <hotkey v-bind:buttons='["B1"]'></hotkey> for Nintendo Switch
* <hotkey v-bind:buttons='["B2"]'></hotkey> for XInput
* <hotkey v-bind:buttons='["B3"]'></hotkey> for DirectInput/PS3
* <hotkey v-bind:buttons='["B4"]'></hotkey> for Keyboard


D-Pad mode **while the controller is in use by pressing one of the following combinations:**

* <hotkey v-bind:buttons='["S1", "S2", "Down"]'></hotkey> - D-Pad
* <hotkey v-bind:buttons='["S1", "S2", "Left"]'></hotkey> - Emulate Left Analog stick
* <hotkey v-bind:buttons='["S1", "S2", "Right"]'></hotkey> - Emulate Right Analog stick


SOCD mode **while the controller is in use by pressing one of the following combinations:**

* <hotkey v-bind:buttons='["S2", "A1", "Up"]'></hotkey> - **Up Priority mode**: Up + Down = Up, Left + Right = Neutral (Stickless behavior).
* <hotkey v-bind:buttons='["S2", "A1", "Down"]'></hotkey> - **Neutral mode**: Up + Down = Neutral, Left + Right = Neutral.
* <hotkey v-bind:buttons='["S2", "A1", "Left"]'></hotkey> - **Last Input Priority (Last Win)**: Hold Up then hold Down = Down, then release and re-press Up = Up. Applies to both axes.


Invert the Y-axis input of the D-pad.  Press <hotkey v-bind:buttons='["S2", "A1", "Right"]'></hotkey>.

## LED Control Kotkeys

| Hotkey | Description |
| - | - |
| <hotkey v-bind:buttons='["S1", "S2", "B3"]'></hotkey> | Next Animation |
| <hotkey v-bind:buttons='["S1", "S2", "B1"]'></hotkey> | Previous Animation |
| <hotkey v-bind:buttons='["S1", "S2", "B4"]'></hotkey> | Brightness Up |
| <hotkey v-bind:buttons='["S1", "S2", "B2"]'></hotkey> | Brightness Down |
| <hotkey v-bind:buttons='["S1", "S2", "R1"]'></hotkey> | LED Parameter Up |
| <hotkey v-bind:buttons='["S1", "S2", "R2"]'></hotkey> | LED Parameter Down |
| <hotkey v-bind:buttons='["S1", "S2", "L1"]'></hotkey> | Pressed Parameter Up |
| <hotkey v-bind:buttons='["S1", "S2", "L2"]'></hotkey> | Pressed Parameter Down |
