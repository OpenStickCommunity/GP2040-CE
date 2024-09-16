# GP2040 Configuration for the Zero_Rhythm

![Zero Rhythm](Assets/ZeroRhythm%201.jpg)

By default the Zero Rhythm is setup to be used on most Switch games.  You can remap the left and right WASD cluster to be LS and RS inputs for other games via the web-config.

## Main Pin Mapping Configuration

| RP2040 Pin | Action                        | GP2040 | Xinput | Switch | PS3/4/5  | Dinput | Arcade |
|------------|-------------------------------|--------|--------|--------|----------|--------|--------|
| GPIO_PIN_01| GpioAction::BUTTON_PRESS_R2   | R2     | RT     | ZR     | R2       | 8      | K3     |
| GPIO_PIN_02| GpioAction::BUTTON_PRESS_B2   | B2     | B      | A      | Circle   | 3      | K2     |
| GPIO_PIN_03| GpioAction::BUTTON_PRESS_B4   | B4     | Y      | X      | Triangle | 4      | P2     |
| GPIO_PIN_04| GpioAction::BUTTON_PRESS_B1   | B1     | A      | B      | Cross    | 2      | K1     |
| GPIO_PIN_05| GpioAction::BUTTON_PRESS_B3   | B3     | X      | Y      | Square   | 1      | P1     |
| GPIO_PIN_06| GpioAction::BUTTON_PRESS_R1   | R1     | RB     | R      | R1       | 6      | P3     |
| GPIO_PIN_07| GpioAction::BUTTON_PRESS_A2   | A2     | ~      | Capture| ~        | 14     | ~      |
| GPIO_PIN_08| GpioAction::BUTTON_PRESS_A1   | A1     | Guide  | Home   | PS       | 13     | ~      |
| GPIO_PIN_09| GpioAction::BUTTON_PRESS_S1   | S1     | Back   | Minus  | Select   | 9      | Coin   |
| GPIO_PIN_10| GpioAction::BUTTON_PRESS_S2   | S2     | Start  | Plus   | Start    | 10     | Start  |
| GPIO_PIN_11| GpioAction::BUTTON_PRESS_L1   | L1     | LB     | L      | L1       | 5      | P4     |
| GPIO_PIN_14| GpioAction::BUTTON_PRESS_RIGHT| RIGHT  | RIGHT  | RIGHT  | RIGHT    | RIGHT  | RIGHT  |
| GPIO_PIN_15| GpioAction::BUTTON_PRESS_DOWN | DOWN   | DOWN   | DOWN   | DOWN     | DOWN   | DOWN   |
| GPIO_PIN_26| GpioAction::BUTTON_PRESS_UP   | UP     | UP     | UP     | UP       | UP     | UP     |
| GPIO_PIN_27| GpioAction::BUTTON_PRESS_LEFT | LEFT   | LEFT   | LEFT   | LEFT     | LEFT   | LEFT   |
| GPIO_PIN_28| GpioAction::BUTTON_PRESS_L2   | L2     | LT     | ZL     | L2       | 7      | K4     |

GPIO29 is connected to the extra button beside the RP2040 unit. It can be assigned to the function layer if desired.

At this time, there is a small issue with the display layout, and you will need to change the `board config` left and right via web-config to see the display that is shown in the image above.
