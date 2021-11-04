# GP2040 Usage

## Buttons

GP2040 uses a generic button labeling for gamepad state, which is then converted to the appropriate input type before sending. This table provides a map of GP2040 buttons to the supported input types and layouts:

| GP2040  | XInput | Switch  | PS3          | DirectInput  | Arcade |
| ------- | ------ | ------- | ------------ | ------------ | ------ |
| B1      | A      | B       | Cross        | 2            | K1     |
| B2      | B      | A       | Circle       | 3            | K2     |
| B3      | X      | Y       | Square       | 1            | P1     |
| B4      | Y      | X       | Triangle     | 4            | P2     |
| L1      | LB     | L       | L1           | 5            | P4     |
| R1      | RB     | R       | R1           | 6            | P3     |
| L2      | LT     | ZL      | L2           | 7            | K4     |
| R2      | RT     | ZR      | R2           | 8            | K3     |
| S1      | Back   | Minus   | Select       | 9            | Coin   |
| S2      | Start  | Plus    | Start        | 10           | Start  |
| L3      | LS     | LS      | L3           | 11           | LS     |
| R3      | RS     | RS      | R3           | 12           | RS     |
| A1      | Guide  | Home    | -            | 13           | -      |
| A2      | -      | Capture | -            | 14           | -      |

If you do not have a dedicated Home button, you can activate it via the **`S1 + S2 + UP`** button combination.

## Input Modes

To change the input mode, **hold one of the following buttons as the controller is plugged in (Arcade mapping in parentheses):**

* **`B1 (K1)`** for Nintendo Switch
* **`B2 (K2)`** for XInput
* **`B3 (P1)`** for DirectInput/PS3

Input mode is saved across power cycles.

## D-Pad Modes

You can switch between the 3 modes for the D-Pad **while the controller is in use by pressing one of the following combinations:**

* **`S1 + S2 + DOWN`** - D-Pad
* **`S1 + S2 + LEFT`** - Emulate Left Analog stick
* **`S1 + S2 + RIGHT`** - Emulate Right Analog stick

D-Pad mode is saved across power cycles.

## SOCD Modes

Simultaneous Opposite Cardinal Direction (SOCD) cleaning will ensure the controller doesn't send invalid directional inputs to the computer/console, like Left + Right at the same time. There are 3 modes to choose from **while the controller is in use by pressing one of the following combinations:**

* **`L3 + R3 + UP`** - **Up Priority mode**: Up + Down = Up, Left + Right = Neutral (Hitbox behavior)
* **`L3 + R3 + DOWN`** - **Neutral mode**: Up + Down = Neutral, Left + Right = Neutral
* **`L3 + R3 + LEFT`** - **Last Input Priority (Last Win)**: Hold Up then hold Down = Down, then release and re-press Up = Up. Applies to both axes.

SOCD mode is saved across power cycles.

## RGB LEDs

> LED modes are available on the Pico Fighting Board, Crush Counter/OSFRD and custom builds only.

### RGB LED Animations

The following animations are available:

| Name | Description | LED Parameter |
| - | - | - |
| Off | Turn off per-button RGB LEDs | - |
| Static Color | Sets all LEDs to the same color | Cycle through colors: *Red*, *Orange*, *Yellow*, *Lime Green*, *Green*, *Seafoam*, *Aqua*, *Sky Blue*, *Blue*, *Purple*, *Pink*, *Magenta* |
| Rainbow Cycle | All LEDs cycle through the color wheel displaying the same color | Adjust animation speed |
| Rainbow Chase | A fading, rainbow cycling lines travels across the LED chain | Adjust animation speed |
| Static Theme | Set the LEDs to a pre-defined static theme | Cycle through themes: *Static Rainbow*, *Super Famicom*, *Xbox*, *Neo Geo Classic*, *Neo Geo Curved*, *Neo Geo Modern*, *Six Button Fighter*, *Six Button Fighter +*, *Guilty Gear Type-A*, *Guilty Gear Type-D* |

### RGB LED Hotkeys

| Hotkey | Description |
| - | - |
| `S1 + S2 + B3` | Next Animation |
| `S1 + S2 + B1` | Previous Animation |
| `S1 + S2 + B4` | Brightness Up |
| `S1 + S2 + B2` | Brightness Down |
| `S1 + S2 + R1` | LED Parameter Up |
| `S1 + S2 + R2` | LED Parameter Down |
| `S1 + S2 + L1` | Pressed Parameter Up |
| `S1 + S2 + L2` | Pressed Parameter Down |
