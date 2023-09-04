# GP2040-CE Shortcuts

Select the button labels to be displayed in the usage guide: <label-selector></label-selector>

## Button Map

| GP2040-CE  | XInput | Switch  | PS4          | PS3          | DirectInput  | Arcade |
| ---------- | ------ | ------- | ------------ | ------------ | ------------ | ------ |
| B1         | A      | B       | Cross        | Cross        | 2            | K1     |
| B2         | B      | A       | Circle       | Circle       | 3            | K2     |
| B3         | X      | Y       | Square       | Square       | 1            | P1     |
| B4         | Y      | X       | Triangle     | Triangle     | 4            | P2     |
| L1         | LB     | L       | L1           | L1           | 5            | P4     |
| R1         | RB     | R       | R1           | R1           | 6            | P3     |
| L2         | LT     | ZL      | L2           | L2           | 7            | K4     |
| R2         | RT     | ZR      | R2           | R2           | 8            | K3     |
| S1         | Back   | Minus   | Share        | Select       | 9            | Coin   |
| S2         | Start  | Plus    | Options      | Start        | 10           | Start  |
| L3         | LS     | LS      | L3           | L3           | 11           | LS     |
| R3         | RS     | RS      | R3           | R3           | 12           | RS     |
| A1         | Guide  | Home    | PS           | PS           | 13           | Home   |
| A2         | -      | Capture | Touchpad     | -            | 14           | -      |

Unlike other controllers, Keyboard gets different keys for directional buttons.

| Direction | Keyboard   |
| --------- | ---------- |
| Up        | Up Arrow   |
| Down      | Down Arrow |
| Left      | Left Arrow |
| Right     | Right Arrow|

?> You can change the key mappings for Keyboard mode in [Webconfig mode](web-configurator.md)

## Mode Selection

**To Enter Bootsel Mode (flash mode)**: hold the <hotkey v-bind:buttons='["S1", "S2", "Up"]'></hotkey> buttons as the controller is plugged in.

**To Enter [Webconfig Mode](web-configurator.md)**: hold the <hotkey v-bind:buttons='["S2"]'></hotkey> button as the controller is plugged in.

**To Select Input Mode**: hold one of the following buttons as the controller is plugged in:

* <hotkey v-bind:buttons='["B1"]'></hotkey> for Nintendo Switch
* <hotkey v-bind:buttons='["B2"]'></hotkey> for XInput
* <hotkey v-bind:buttons='["B3"]'></hotkey> for DirectInput/PS3
* <hotkey v-bind:buttons='["B4"]'></hotkey> for PS4
* <hotkey v-bind:buttons='["R2"]'></hotkey> for Keyboard

## Button Input Hotkeys

Home button shortcut if you do not have a Home button - <hotkey v-bind:buttons='["S1", "S2", "Up"]'></hotkey>.

D-Pad mode **while the controller is in use by pressing one of the following default combinations:**

* <hotkey v-bind:buttons='["S1", "S2", "Down"]'></hotkey> - D-Pad
* <hotkey v-bind:buttons='["S1", "S2", "Left"]'></hotkey> - Emulate Left Analog stick
* <hotkey v-bind:buttons='["S1", "S2", "Right"]'></hotkey> - Emulate Right Analog stick

SOCD mode **while the controller is in use by pressing one of the following default combinations:**

* <hotkey v-bind:buttons='["S2", "A1", "Up"]'></hotkey> - **Up Priority mode**: Up + Down = Up, Left + Right = Neutral (Stickless behavior).
* <hotkey v-bind:buttons='["S2", "A1", "Down"]'></hotkey> - **Neutral mode**: Up + Down = Neutral, Left + Right = Neutral.
* <hotkey v-bind:buttons='["S2", "A1", "Left"]'></hotkey> - **Last Input Priority (Last Win)**: Hold Up then hold Down = Down, then release and re-press Up = Up. Applies to both axes.

Invert the Y-axis input of the D-pad.  Press <hotkey v-bind:buttons='["S2", "A1", "Right"]'></hotkey> by default.

### Changing Hotkeys

The D-Pad Mode, SOCD Mode, and Invert D-Pad Y-axis hotkey bindings can be changed or assigned to different
actions in [the web configurator](web-configurator.md#hotkeys).

## LED Control Hotkeys

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
