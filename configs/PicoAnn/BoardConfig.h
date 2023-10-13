/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef PICOANN_CONFIG_H_
#define PICOANN_CONFIG_H_

#include "enums.pb.h"

#define BOARD_CONFIG_LABEL "Pico Ann"

// This is the main pin definition section.
// This will let you specify which GPIO pin each button is assigned too. 
// You can set any of the main pins as `-1` to disable it.
// The Turbo pin and LS + RS slider pins can also be set to `-1` to disable that functionality.
// Please note that only when `PIN_BUTTON_TURBO` is set to `-1` will the `T##` be removed from a connected display.
// Please note that only when `PIN_SLIDER_ONE` and  `PIN_SLIDER_TWO` are set to `-1` will the button combo shortcut for DP/LS/RS work.
// The buttons are listed in GP2040 configuration, beside each the listed order is *GP2040 / Xinput / Switch / PS3 / Directinput / Arcade*

#define PIN_DPAD_UP       1          // UP
#define PIN_DPAD_DOWN     2          // DOWN
#define PIN_DPAD_RIGHT    3          // RIGHT
#define PIN_DPAD_LEFT     0          // LEFT
#define PIN_BUTTON_B1     11         // B1 / A / B / Cross / 2 / K1
#define PIN_BUTTON_B2     12         // B2 / B / A / Circle / 3 / K2
#define PIN_BUTTON_R2     13         // R2 / RT / ZR / R2 / 8 / K3
#define PIN_BUTTON_L2     14         // L2 / LT / ZL / L2 / 7 / K4
#define PIN_BUTTON_B3     7         // B3 / X / Y / Square / 1 / P1
#define PIN_BUTTON_B4     8         // B4 / Y / X / Triangle / 4 / P2
#define PIN_BUTTON_R1     9         // R1 / RB / R / R1 / 6 / P3
#define PIN_BUTTON_L1     10        // L1 / LB / L / L1 / 5 / P4
#define PIN_BUTTON_S1     6         // S1 / Back / Minus / Select / 9 / Coin
#define PIN_BUTTON_S2     5         // S2 / Start / Plus / Start / 10 / Start
#define PIN_BUTTON_L3     21        // L3 / LS / LS / L3 / 11 / LS
#define PIN_BUTTON_R3     22        // R3 / RS / RS / R3 / 12 / RS
#define PIN_BUTTON_A1     4         // A1 / Guide / Home / PS / 13 / ~
#define PIN_BUTTON_A2     20        // A2 / ~ / Capture / ~ / 14 / ~
#define PIN_BUTTON_FN    -1         // Hotkey Function
#define PIN_BUTTON_TURBO  -1        // Turbo
#define PIN_BUTTON_REVERSE -1       // UDLR Reverse
#define PIN_SLIDER_ONE    -1         // Left Stick Slider
#define PIN_SLIDER_TWO    -1         // Right Stick Slider
#define PIN_SLIDER_SOCD_ONE    -1         // SOCD Slider Pin One
#define PIN_SLIDER_SOCD_TWO    -1         // SOCD Slider Pin Two


// This is the SOCD section.
// SOCD stands for `simultaneous opposing cardinal directions`.
// There are three options for `DEFAULT_SOCD_MODE` currently:
// 1 - `SOCD_MODE_NEUTRAL` - This is a neutral SOCD.  EG. when you press `up` + `down` no input will be registered.
// 2 - `SOCD_MODE_UP_PRIORITY` - This is up priority SOCD.  EG. when you press `up` + `down` `up` will be registered.
// 3 - `SOCD_MODE_SECOND_INPUT_PRIORITY` - This is last priority SOCD.  EG. when you press and hold `up` then press `down` `down` will be registered.
// 4 - `SOCD_MODE_FIRST_INPUT_PRIORITY` - This is first priority SOCD.  EG. when you press and hold `up` then press `down` `up` will be registered.

#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
// SOCD Slider Slot Defaults
#define SLIDER_SOCD_SLOT_ONE SOCD_MODE_UP_PRIORITY
#define SLIDER_SOCD_SLOT_TWO  SOCD_MODE_SECOND_INPUT_PRIORITY
#define SLIDER_SOCD_SLOT_DEFAULT SOCD_MODE_NEUTRAL

#define DEFAULT_FORCED_SETUP_MODE FORCED_SETUP_MODE_OFF // 	FORCED_SETUP_MODE_OFF, FORCED_SETUP_MODE_LOCK_MODE_SWITCH, FORCED_SETUP_MODE_LOCK_WEB_CONFIG, FORCED_SETUP_MODE_LOCK_BOTH
#define DEFAULT_LOCK_HOTKEYS false // or true

#define DEFAULT_PS4CONTROLLER_TYPE PS4_CONTROLLER

// This is the LEDs section.
// The default `TURBO_LED_PIN` pin is set to `15` ( it is recommended to run through 3V3(OUT) with a resistor)
// The Turbo LED will flash at a speed consistant with the set speed of the Turbo when a Turbo button is active.
// It is recommended to disable the `TURBO_LED_PIN` by setting it to `-1` if you are sensitive of flashing lights.
// The default `BOARD_LEDS_PIN` pin is set to `28`.
// The board LED pin will allow you to connect addressible RGB LEDs on the Pico.
// Addressible RGB LEDs should be connected to the `VBUS` pin (#40), an avalible ground pin and the defined `BOARD_LEDS_PIN`.
// Special note - You should only ever use addressible RGB LEDs that are rated for 5v operation on the Pico.
// The defualt `LED_BRIGHTNESS_MAXIMUM` value is `50`.  
// This will change how bright the LEDs are with `0` being off and `100` being full brightness.
// The minimum `LED_BRIGHTNESS_MAXIMUM` value is `0`.
// The maximum `LED_BRIGHTNESS_MAXIMUM` value is `100`.
// The defualt `LED_BRIGHTNESS_STEPS` value is `5`.
// This will change how many brightness steps there are when increasing or decreasing the brightness of the LEDs via the onboard shortcut.
// It is recommend to keep this value at `5` or `10`.
// The defualt `LED_FORMAT` is `LED_FORMAT_GRB`.
// You can also choose the `LED_FORMAT` of `LED_FORMAT_RGB` if your addressible RGB LEDs are not working as intended.
// The defualt `LEDS_PER_PIXEL` is set to `1`.
// This will change how many addressible RGB LEDs there are per button.
// The default LEDS_[BUTTON] is an order and has nothing to do with what GPIO pin something is connected to.
// Unless you are planning on running custom animations I would recommmend you leave this as is.

#define TURBO_LED_PIN -1

#define BOARD_LEDS_PIN -1

#define LED_BRIGHTNESS_MAXIMUM 50
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LEDS_PER_PIXEL 1

#define LEDS_DPAD_LEFT   0
#define LEDS_DPAD_DOWN   1
#define LEDS_DPAD_RIGHT  2
#define LEDS_DPAD_UP     3
#define LEDS_BUTTON_B3   4
#define LEDS_BUTTON_B4   5
#define LEDS_BUTTON_R1   6
#define LEDS_BUTTON_L1   7
#define LEDS_BUTTON_B1   8
#define LEDS_BUTTON_B2   9
#define LEDS_BUTTON_R2   10
#define LEDS_BUTTON_L2   11


// This is the Player LED section.  
// In this section you can specify if Player LEDs will be active, and, if active, which pins will be used for them.
// The defualt is `PLED_TYPE_NONE` which will turn the Player LEDs off.
// The default pin for each Player LED is `-1` which disables it.  
// To enable a `PLED#_PIN`, replace the `-1` with the GPIO pin number that is desired. 
// There are three options for `PLED_TYPE` currently:
// 1 - `PLED_TYPE_NONE` - This will disable the Player LEDs
// 2 - `PLED_TYPE_PWM` - This will enable the Player LEDs ( it is recommended to run through 3V3(OUT) with a resistor)
// 3 - `PLED_TYPE_RGB` - This will enable the Player LEDs as addressible RGB LEDs (please not that this has not been implemented yet) 

#define PLED_TYPE PLED_TYPE_PWM
#define PLED1_PIN 16
#define PLED2_PIN 17
#define PLED3_PIN 18
#define PLED4_PIN 19


// This is the Analog section.
// In this section you can specify if Analog is enabled, and, if endabled, which pins will be used for it.
// The default for `ANALOG_ADC_VRX` and `ANALOG_ADC_VRY` is `-1` which disables them.
// To enable a `ANALOG_ADC_VRX` and `ANALOG_ADC_VRY`, replace the `-1` with the GPIO pin numbers that are desired. 

#define ANALOG_ADC_1_VRX -1
#define ANALOG_ADC_1_VRY -1
#define ANALOG_ADC_1_MODE DPAD_MODE_LEFT_ANALOG
#define ANALOG_ADC_1_INVERT INVERT_NONE

#define ANALOG_ADC_2_VRX -1
#define ANALOG_ADC_2_VRY -1
#define ANALOG_ADC_2_MODE DPAD_MODE_RIGHT_ANALOG
#define ANALOG_ADC_2_INVERT INVERT_NONE

// Reverse Button section
#define REVERSE_LED_PIN -1

// This is the I2C Display section (commonly known as the OLED display section).
// In this section you can specify if a display as been enabled, which pins are assined to it, the block address and speed.
// The default for `HAS_I2C_DISPLAY` is `1` which enables it.
// To disable the display you can change `HAS_I2C_DISPLAY` to `-1`.
// The default `I2C_SDA_PIN` is `0`.
// The defualt `I2C_SCL_PIN` is `1`.
// The defualt `I2C_BLOCK` is `12c0`.  
// If you change the `I2C_SDA_PIN` and `I2C_SCL_PIN` pin mapping, you may need to change the `I2C_BLOCK` as well.
// The defualt `I2C_SPEED` is `400000`.  
// This should be more than fast enough for most displays.
// Some smaller displays (like 0.96" and 1.31") can go up to `800000` or even `1000000`.
// The default `DISPLAY_FLIP` is `0`.
// This can be changed to `1` to have the dispaly output flipped.
// The default `DISPLAY_INVERY` is `0`.
// This can be changed to `1` to have the color on the display inverted.
// The default `BUTTON_LAYOUT` is `BUTTON_LAYOUT_STICK` which will show an arcade stick on the left hand side of the display.
// There are seven options for `BUTTON_LAYOUT` currently:
// 1 - BUTTON_LAYOUT_STICK - This is a basic joystick layout
// 2 - BUTTON_LAYOUT_STICKLESS - This is a basic stickless (all button) layout
// 3 - BUTTON_LAYOUT_BUTTONS_ANGLED - This a WASD button layout that is angled
// 4 - BUTTON_LAYOUT_BUTTONS_BASIC - This a WASD button layout that is straight
// 5 - BUTTON_LAYOUT_KEYBOARD_ANGLED - This is a WASD keyboard layout that is angled
// 6 - BUTTON_LAYOUT_KEYBOARDA - This is a WASD keyboard layout that is straight
// 7 - BUTTON_LAYOUT_DANCEPADA - This is a dance pad layout (must be used with `BUTTON_LAYOUT_DANCEPADB` in `BUTTON_LAYOUT_RIGHT`)
// The default `BUTTON_LAYOUT_RIGHT` is `BUTTON_LAYOUT_NOIR8` which will show eight buttons on the right hand side of the display.
// There are eleven options for `BUTTON_LAYOUT_RIGHT` currently:
// 1 - BUTTON_LAYOUT_ARCADE - This is a standard 8 button arcade layout
// 2 - BUTTON_LAYOUT_STICKLESSB - This is a basic stickless (all button) layout
// 3 - BUTTON_LAYOUT_BUTTONS_ANGLEDB - This is a standard 8 button arcade layout that is angled
// 4 - BUTTON_LAYOUT_VEWLIX - This is the standard 8 button Vewlix layout
// 5 - BUTTON_LAYOUT_VEWLIX7 - This is the standard 7 button Vewlix layout
// 6 - BUTTON_LAYOUT_CAPCOM - This is the standard 8 button Capcom layout
// 7 - BUTTON_LAYOUT_CAPCOM6 - This is the stndard 6 button Capcom layout
// 8 - BUTTON_LAYOUT_SEGA2P - This is the standard 8 button Sega2P layout
// 9 - BUTTON_LAYOUT_NOIR8 - This is the standard 8 button Noir layout
// 10 - BUTTON_LAYOUT_KEYBOARDB - This is a WASD keyboard layout that is straight
// 11 - BUTTON_LAYOUT_DANCEPADB - This is a dance pad layout (must be used with `BUTTON_LAYOUT_DANCEPADA` in `BUTTON_LAYOUT`)
// The default `SPLASH_MODE` is `NOSPLASH`.  
// There are four options for `SPLASH_MODE` currently:
// 1 - `STATICSPLASH` - This will display the static splash image
// 2 - `CLOSEIN` - This will display the static splash image as a top and bottom coming together animation
// 3 - `CLOSEINCUSTOM` - This will display the custom splash image as a top and bottom coming together animation
// 4 - `NOSPLASH` - This will not display a splash screen on boot
// Special note - All of the splash screen images can be changed via `include/bitmaps.h`

#define HAS_I2C_DISPLAY 0
#define I2C_SDA_PIN 26
#define I2C_SCL_PIN 27
#define I2C_BLOCK i2c1
#define I2C_SPEED 400000
#define DISPLAY_FLIP 0
#define DISPLAY_INVERT 0

#define REVERSE_UP_DEFAULT 1
#define REVERSE_DOWN_DEFAULT 1
#define REVERSE_LEFT_DEFAULT 1
#define REVERSE_RIGHT_DEFAULT 1

#define BUTTON_LAYOUT BUTTON_LAYOUT_STICK
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_VEWLIX
#define SPLASH_MODE SPLASH_MODE_NONE
#define SPLASH_CHOICE SPLASH_CHOICE_MAIN
#define SPLASH_DURATION 7500 // Duration in milliseconds

// Board LED Add-on Setting
// BOARD_LED_OFF  - Turns the on-board LED off
// MODE_INDICATOR - On-board LED blinks on various frequencies depending
//                  on the current mode (config, normal, or no USB data)
// INPUT_TEST     - Blinks whenever any input is made

#define BOARD_LED_TYPE ON_BOARD_LED_MODE_OFF

// Dual Directional Add-on Options

#define PIN_DUAL_DIRECTIONAL_UP -1
#define PIN_DUAL_DIRECTIONAL_DOWN -1
#define PIN_DUAL_DIRECTIONAL_LEFT -1
#define PIN_DUAL_DIRECTIONAL_RIGHT -1
#define DUAL_DIRECTIONAL_STICK_MODE DPAD_MODE_DIGITAL
#define DUAL_DIRECTIONAL_COMBINE_MODE DUAL_COMBINE_MODE_MIXED

// BOOTSEL Button Add-on setting
#define BOOTSEL_BUTTON_MASK 0 // 0 means none, get other mask from GamepadState.h

// Extra Button Add-on setting
#define EXTRA_BUTTON_MASK 0 // 0 means none, get other mask from GamepadState.h
                            // For directions, use GAMEPAD_MASK_DU, GAMEPAD_MASK_DD, GAMEPAD_MASK_DL and GAMEPAD_MASK_DR
#define EXTRA_BUTTON_PIN -1

// Keyboard Mapping Configuration
// List of HID keycodes can be located here: https://github.com/hathach/tinyusb/blob/3623ba1884ddff23e9b64766cb6dd032f1425846/src/class/hid/hid.h#L356
// Even for the modifier keys, HID_KEY entries should be used as the implementation expects those and will convert as necessary.
#define KEY_DPAD_UP     HID_KEY_ARROW_UP      // UP
#define KEY_DPAD_DOWN   HID_KEY_ARROW_DOWN    // DOWN
#define KEY_DPAD_RIGHT  HID_KEY_ARROW_RIGHT   // RIGHT
#define KEY_DPAD_LEFT   HID_KEY_ARROW_LEFT    // LEFT
#define KEY_BUTTON_B1   HID_KEY_SHIFT_LEFT    // B1 / A / B / Cross / 2 / K1
#define KEY_BUTTON_B2   HID_KEY_Z             // B2 / B / A / Circle / 3 / K2
#define KEY_BUTTON_R2   HID_KEY_X             // R2 / RT / ZR / R2 / 8 / K3
#define KEY_BUTTON_L2   HID_KEY_V             // L2 / LT / ZL / L2 / 7 / K4
#define KEY_BUTTON_B3   HID_KEY_CONTROL_LEFT  // B3 / X / Y / Square / 1 / P1
#define KEY_BUTTON_B4   HID_KEY_ALT_LEFT      // B4 / Y / X / Triangle / 4 / P2
#define KEY_BUTTON_R1   HID_KEY_SPACE         // R1 / RB / R / R1 / 6 / P3
#define KEY_BUTTON_L1   HID_KEY_C             // L1 / LB / L / L1 / 5 / P4
#define KEY_BUTTON_S1   HID_KEY_5             // S1 / Back / Minus / Select / 9 / Coin
#define KEY_BUTTON_S2   HID_KEY_1             // S2 / Start / Plus / Start / 10 / Start
#define KEY_BUTTON_L3   HID_KEY_EQUAL         // L3 / LS / LS / L3 / 11 / LS
#define KEY_BUTTON_R3   HID_KEY_MINUS         // R3 / RS / RS / R3 / 12 / RS
#define KEY_BUTTON_A1   HID_KEY_9             // A1 / Guide / Home / PS / 13 / ~
#define KEY_BUTTON_A2   HID_KEY_F2            // A2 / ~ / Capture / ~ / 14 / ~

#endif
