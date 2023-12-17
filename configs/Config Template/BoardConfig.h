/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"

// The BOARD_CONFIG_LABEL is the name of your device
// A matching entry must be made in /.github/workflows/cmake.yml on line 26
// To compile this custom device or any other device chnage the name in /CMakeLists.txt line 25

#define BOARD_CONFIG_LABEL "Pico"

// This will change the default input mode
// If this is left blank the device will default to XINPUT
// You can also boot to web-config as a defult with INPUT_MODE_CONFIG

#define DEFAULT_INPUT_MODE INPUT_MODE_XINPUT
#define DEFAULT_INPUT_MODE INPUT_MODE_SWITCH
#define DEFAULT_INPUT_MODE INPUT_MODE_PS4
#define DEFAULT_INPUT_MODE INPUT_MODE_HID
#define DEFAULT_INPUT_MODE INPUT_MODE_CONFIG

// If in INPUT_MODE_PS4 you can also choose between controller and arcade stick mode
// Controller mode will act as a DS4 which will work nativly on the PS4 and on the PS5 for PS4 titles
// Arcade stick mode will act as a generic 3rd party device and will nativly on the PS4 and on the PS5 for PS4 titles as well as PS5 titles that support legacy controllers

#define DEFAULT_PS4CONTROLLER_TYPE PS4_CONTROLLER
#define DEFAULT_PS4CONTROLLER_TYPE PS4_ARCADESTICK

// This will change the default direction input mode
// If this is left blank the device will default to DIGITAL (Dpad)

#define DEFAULT_DPAD_MODE DPAD_MODE_DIGITAL
#define DEFAULT_DPAD_MODE DPAD_MODE_LEFT_ANALOG
#define DEFAULT_DPAD_MODE DPAD_MODE_RIGHT_ANALOG

// This will change the default SOCD cleaning mode
// If this is left blank the device will default to neutral prioritiy
// SOCD_MODE_UP_PRIORITY = U+D=U, L+R=N
// SOCD_MODE_NEUTRAL = U+D=N, L+R=N
// SOCD_MODE_SECOND_INPUT_PRIORITY = U>D=D, L>R=R (Last Input Priority, aka Last Win)
// SOCD_MODE_FIRST_INPUT_PRIORITY = U>D=U, L>R=L (First Input Priority, aka First Win)
// SOCD_MODE_BYPASS = U+D=UD, L+R=LR (No cleaning applied)
// Note: PS4, PS3 and Nintendo Switch modes do not support setting SOCD Cleaning to Off and will default to Neutral SOCD Cleaning mode.

#define DEFAULT_SOCD_MODE SOCD_MODE_UP_PRIORITY
#define DEFAULT_SOCD_MODE SOCD_MODE_NEUTRAL
#define DEFAULT_SOCD_MODE SOCD_MODE_SECOND_INPUT_PRIORITY
#define DEFAULT_SOCD_MODE SOCD_MODE_FIRST_INPUT_PRIORITY
#define DEFAULT_SOCD_MODE SOCD_MODE_BYPASS

// Default forced setup mode will force a lock on a specific setup
// FORCED_SETUP_MODE_OFF will not lock anything
// FORCED_SETUP_MODE_LOCK_MODE_SWITCH will prevent the changing of input mode as  defined in DEFAULT_INPUT_MODE
// FORCED_SETUP_MODE_LOCK_WEB_CONFIG will prevent the device from being able to boot into web-config mode
// FORCED_SETUP_MODE_LOCK_BOTH will prevent the device from being able to change input modes and being able to boot into web-config mode

#define DEFAULT_FORCED_SETUP_MODE FORCED_SETUP_MODE_OFF
#define DEFAULT_FORCED_SETUP_MODE FORCED_SETUP_MODE_LOCK_MODE_SWITCH
#define DEFAULT_FORCED_SETUP_MODE FORCED_SETUP_MODE_LOCK_WEB_CONFIG
#define DEFAULT_FORCED_SETUP_MODE FORCED_SETUP_MODE_LOCK_BOTH

// 4 way joystick slider? Fill this section in

// The profile number selection will specify which profile to load on boot
// Fill this section in as well

// ---
// Configuration - Pin Mapping
// ---

// This is the main pin definition section.
// This will let you specify which GPIO pin each button is assigned too. 
// You can set any of the main pins as `-1` to disable it.
// The buttons are listed in GP2040 configuration, beside each the listed order is *GP2040 / Xinput / Switch / PS3 / Directinput / Arcade*

//                          // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define PIN_DPAD_UP     2   // UP     | UP     | UP      | UP       | UP     | UP     |
#define PIN_DPAD_DOWN   3   // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   | 
#define PIN_DPAD_RIGHT  4   // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  | 
#define PIN_DPAD_LEFT   5   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   | 
#define PIN_BUTTON_B1   6   // B1     | A      | B       | Cross    | 2      | K1     |
#define PIN_BUTTON_B2   7   // B2     | B      | A       | Circle   | 3      | K2     |
#define PIN_BUTTON_R2   8   // R2     | RT     | ZR      | R2       | 8      | K3     |
#define PIN_BUTTON_L2   9   // L2     | LT     | ZL      | L2       | 7      | K4     |
#define PIN_BUTTON_B3   10  // B3     | X      | Y       | Square   | 1      | P1     |
#define PIN_BUTTON_B4   11  // B4     | Y      | X       | Triangle | 4      | P2     |
#define PIN_BUTTON_R1   12  // R1     | RB     | R       | R1       | 6      | P3     |
#define PIN_BUTTON_L1   13  // L1     | LB     | L       | L1       | 5      | P4     |
#define PIN_BUTTON_S1   16  // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define PIN_BUTTON_S2   17  // S2     | Start  | Plus    | Start    | 10     | Start  |
#define PIN_BUTTON_L3   18  // L3     | LS     | LS      | L3       | 11     | LS     |
#define PIN_BUTTON_R3   19  // R3     | RS     | RS      | R3       | 12     | RS     |
#define PIN_BUTTON_A1   20  // A1     | Guide  | Home    | PS       | 13     | ~      |
#define PIN_BUTTON_A2   21  // A2     | ~      | Capture | ~        | 14     | ~      |
#define PIN_BUTTON_FN   -1  // Hotkey Function                                        |

// ---
// Configuration - Keyboard Mapping
// ---

// Keyboard Mapping Configuration
// List of HID keycodes can be located here: https://github.com/hathach/tinyusb/blob/3623ba1884ddff23e9b64766cb6dd032f1425846/src/class/hid/hid.h#L356
// Even for the modifier keys, HID_KEY entries should be used as the implementation expects those and will convert as necessary.

//                                            // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define KEY_DPAD_UP     HID_KEY_ARROW_UP      // UP     | UP     | UP      | UP       | UP     | UP     |
#define KEY_DPAD_DOWN   HID_KEY_ARROW_DOWN    // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define KEY_DPAD_RIGHT  HID_KEY_ARROW_RIGHT   // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define KEY_DPAD_LEFT   HID_KEY_ARROW_LEFT    // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define KEY_BUTTON_B1   HID_KEY_SHIFT_LEFT    // B1     | A      | B       | Cross    | 2      | K1     |
#define KEY_BUTTON_B2   HID_KEY_Z             // B2     | B      | A       | Circle   | 3      | K2     |
#define KEY_BUTTON_R2   HID_KEY_X             // R2     | RT     | ZR      | R2       | 8      | K3     |
#define KEY_BUTTON_L2   HID_KEY_V             // L2     | LT     | ZL      | L2       | 7      | K4     |
#define KEY_BUTTON_B3   HID_KEY_CONTROL_LEFT  // B3     | X      | Y       | Square   | 1      | P1     |
#define KEY_BUTTON_B4   HID_KEY_ALT_LEFT      // B4     | Y      | X       | Triangle | 4      | P2     |
#define KEY_BUTTON_R1   HID_KEY_SPACE         // R1     | RB     | R       | R1       | 6      | P3     |
#define KEY_BUTTON_L1   HID_KEY_C             // L1     | LB     | L       | L1       | 5      | P4     |
#define KEY_BUTTON_S1   HID_KEY_5             // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define KEY_BUTTON_S2   HID_KEY_1             // S2     | Start  | Plus    | Start    | 10     | Start  |
#define KEY_BUTTON_L3   HID_KEY_EQUAL         // L3     | LS     | LS      | L3       | 11     | LS     |
#define KEY_BUTTON_R3   HID_KEY_MINUS         // R3     | RS     | RS      | R3       | 12     | RS     |
#define KEY_BUTTON_A1   HID_KEY_9             // A1     | Guide  | Home    | PS       | 13     | ~      |
#define KEY_BUTTON_A2   HID_KEY_F2            // A2     | ~      | Capture | ~        | 14     | ~      |
#define KEY_BUTTON_FN   HID_KEY_              // Hotkey Function                                        |

// ---
// Configuration - Profile Settings
// ---

// ---
// Configuration - LED Configuration
// ---

// This is the LEDs section.
// The default `TURBO_LED_PIN` pin is set to `15` ( it is recommended to run through 3V3(OUT) with a resistor)
// The Turbo LED will flash at a speed consistant with the set speed of the Turbo when a Turbo button is active.
// It is recommended to disable the `TURBO_LED_PIN` by setting it to `-1` if you are sensitive of flashing lights.


// The default pin to use for addressible RGB LEDs.
// Setting to -1 will disable addressible RGB LEDs by default.

#define BOARD_LEDS_PIN -1

// The LED_FORMAT specifies how your addressible RGB LEDs need to recieve data.
// LED_FORMAT_GRB = The standard format for addressible RGB LEDs
// LED_FORMAT_RBG = The alternate format, use this one if your addressable RGB LEDs are working but are the wrong color
// LED_FORMAT_GRBW = GRB format with a white LED as well
// LED_FORMAT_RBGW = RGB format with a white LED as well

#define LED_FORMAT LED_FORMAT_GRB
#define LED_FORMAT LED_FORMAT_RBG
#define LED_FORMAT LED_FORMAT_GRBW
#define LED_FORMAT LED_FORMAT_RBGW

// LED Layout 

#define LED_LAYOUT 
???
???

// LEDS_PER_PIXEL will specify how many LEDs are to be used on each button

#define LEDS_PER_PIXEL 1

// This will change how the brightness of the LEDs is handled
// The defualt `LED_BRIGHTNESS_MAXIMUM` value is `50`.  
// This will change how bright the LEDs are with `0` being off and `100` being full brightness.
// The minimum `LED_BRIGHTNESS_MAXIMUM` value is `0`.
// The maximum `LED_BRIGHTNESS_MAXIMUM` value is `100`.

#define LED_BRIGHTNESS_MAXIMUM 50

// This will change how many steps there are when increasing or decreasing the LED brightness via hotkey.
// Setting `LED_BRIGHTNESS_STEPS 5` would mean that there are 5 brightness levels between `0` and your set `LED_BRIGHTNESS_MAXIMUM`
// The defualt `LED_BRIGHTNESS_STEPS` value is `5`.

#define LED_BRIGHTNESS_STEPS 5

// Turn off when suspended?
???
???

// Player LEDs currently only work in Xinput mode
// PLED_TYPE_NONE = Player LEDs are turned off
// PLED_TYPE_PWM = Player LEDs run off of individual GPIO pins (non-addressible RGB LEDs)
// PLED_TYPE_RGB = Player LEDs run off of the above defined `BOARD_LEDS_PIN`
// Note that for PLED_TYPE_RGB pin position must be set starting after the last button that is defined

#define PLED_TYPE PLED_TYPE_NONE
#define PLED_TYPE PLED_TYPE_PWM
#define PLED_TYPE PLED_TYPE_RGB

// When in PLED_TYPE_PWM mode
// #define PLED1_PIN = The GPIO for the first Player LED
// #define PLED2_PIN = The GPIO for the second Player LED
// #define PLED3_PIN = The GPIO for the third Player LED
// #define PLED4_PIN = The GPIO for the fourth Player LED

#define PLED1_PIN -1
#define PLED2_PIN -1
#define PLED3_PIN -1
#define PLED4_PIN -1

// When in PLED_TYPE_RGB mode
// #define PLED1_PIN = The position in the RGB line for the first Player LED
// #define PLED2_PIN = The position in the RGB line for the second Player LED
// #define PLED3_PIN = The position in the RGB line for the third Player LED
// #define PLED4_PIN = The position in the RGB line for the fourth Player LED

#define PLED1_PIN -1
#define PLED2_PIN -1
#define PLED3_PIN -1
#define PLED4_PIN -1


// Turn off when suspended?
???
???

// RGB LED button order will dictate what what order the RGB LEDs are connected in. 
// This must match how you have them wired up off a strip or the order in an embedded design
// If you do not have LEDs on the aux buttons simple do not define them

//                          // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define LEDS_DPAD_LEFT   0  // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   | 
#define LEDS_DPAD_DOWN   1  // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   | 
#define LEDS_DPAD_RIGHT  2  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  | 
#define LEDS_DPAD_UP     3  // UP     | UP     | UP      | UP       | UP     | UP     |
#define LEDS_BUTTON_B3   4  // B3     | X      | Y       | Square   | 1      | P1     |
#define LEDS_BUTTON_B4   5  // B4     | Y      | X       | Triangle | 4      | P2     |
#define LEDS_BUTTON_R1   6  // R1     | RB     | R       | R1       | 6      | P3     |
#define LEDS_BUTTON_L1   7  // L1     | LB     | L       | L1       | 5      | P4     |
#define LEDS_BUTTON_B1   8  // B1     | A      | B       | Cross    | 2      | K1     |
#define LEDS_BUTTON_B2   9  // B2     | B      | A       | Circle   | 3      | K2     |
#define LEDS_BUTTON_R2   10 // R2     | RT     | ZR      | R2       | 8      | K3     |
#define LEDS_BUTTON_L2   11 // L2     | LT     | ZL      | L2       | 7      | K4     |
#define LEDS_BUTTON_S1   12 // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define LEDS_BUTTON_S2   13 // S2     | Start  | Plus    | Start    | 10     | Start  |
#define LEDS_BUTTON_L3   14 // L3     | LS     | LS      | L3       | 11     | LS     |
#define LEDS_BUTTON_R3   15 // R3     | RS     | RS      | R3       | 12     | RS     |
#define LEDS_BUTTON_A1   16 // A1     | Guide  | Home    | PS       | 13     | ~      |
#define LEDS_BUTTON_A2   17 // A2     | ~      | Capture | ~        | 14     | ~      |

// ---
// Custom LED theme
// ---
???
???

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

#define DEFAULT_INPUT_MODE INPUT_MODE_XINPUT //INPUT_MODE_XINPUT (XInput), INPUT_MODE_SWITCH (Nintendo Switch), INPUT_MODE_HID (D-Input), INPUT_MODE_KEYBOARD (Keyboard)
#define DEFAULT_DPAD_MODE DPAD_MODE_DIGITAL  //DPAD_MODE_DIGITAL, DPAD_MODE_LEFT_ANALOG, DPAD_MODE_RIGHT_ANALOG, 

#define DEFAULT_PS4CONTROLLER_TYPE PS4_CONTROLLER



#define TURBO_LED_PIN -1






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
// The default `DISPLAY_INVERT` is `0`.
// This can be changed to `1` to have the color on the display inverted.
// The default `DISPLAY_SAVER_TIMEOUT` is `0`.
// This can be changed to a number in minutes which will be the inactivity timeout for the display to turn off.
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
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1
#define I2C_BLOCK i2c0
#define I2C_SPEED 400000
#define DISPLAY_FLIP 0
#define DISPLAY_INVERT 0
#define DISPLAY_SAVER_TIMEOUT 0

// I2C Analog ADS1219 Add-on Options
#define I2C_ANALOG1219_SDA_PIN -1
#define I2C_ANALOG1219_SCL_PIN -1
#define I2C_ANALOG1219_BLOCK i2c0
#define I2C_ANALOG1219_SPEED 400000
#define I2C_ANALOG1219_ADDRESS 0x40

// Reverse Button section
#define REVERSE_LED_PIN -1
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

#define BOARD_LED_ENABLED 0
#define BOARD_LED_TYPE ON_BOARD_LED_MODE_OFF

// Dual Directional Add-on Options

#define PIN_DUAL_DIRECTIONAL_UP -1
#define PIN_DUAL_DIRECTIONAL_DOWN -1
#define PIN_DUAL_DIRECTIONAL_LEFT -1
#define PIN_DUAL_DIRECTIONAL_RIGHT -1
#define DUAL_DIRECTIONAL_STICK_MODE DPAD_MODE_DIGITAL
#define DUAL_DIRECTIONAL_COMBINE_MODE DUAL_COMBINE_MODE_MIXED

// TILTAdd-on Options
#define PIN_TILT_1 -1
#define TILT1_FACTOR_LEFT_X  35  //Default value for the TILT button to function.
#define TILT1_FACTOR_LEFT_Y 35 //Default value for the TILT button to function.
#define TILT1_FACTOR_RIGHT_X 35  //Default value for the TILT button to function.
#define TILT1_FACTOR_RIGHT_Y 35 //Default value for the TILT button to function.
#define PIN_TILT_2 -1
#define TILT2_FACTOR_LEFT_X  65  //Default value for the TILT button to function.
#define TILT2_FACTOR_LEFT_Y  65  //Default value for the TILT button to function.
#define TILT2_FACTOR_RIGHT_X 65  //Default value for the TILT button to function.
#define TILT2_FACTOR_RIGHT_Y 65  //Default value for the TILT button to function.
#define PIN_TILT_LEFT_ANALOG_UP -1
#define PIN_TILT_LEFT_ANALOG_DOWN -1
#define PIN_TILT_LEFT_ANALOG_LEFT -1
#define PIN_TILT_LEFT_ANALOG_RIGHT -1
#define PIN_TILT_RIGHT_ANALOG_UP -1
#define PIN_TILT_RIGHT_ANALOG_DOWN -1
#define PIN_TILT_RIGHT_ANALOG_LEFT -1
#define PIN_TILT_RIGHT_ANALOG_RIGHT -1
#define TILT_SOCD_MODE SOCD_MODE_NEUTRAL

// BOOTSEL Button Add-on setting
#define BOOTSEL_BUTTON_MASK 0 // 0 means none, get other mask from GamepadState.h

// This is the Buzzer Speaker section.  
// In this section you can specify if Buzzer Speaker will be active, and, if active, which pin will be used for them.
// The default is `BUZZER_ENABLED` which will turn the Buzzer Speaker off.
// The default pin for Buzzer Speaker is `-1` which will turn the Buzzer Speaker off.  
// The default volume for Buzzer Speaker is 100 (max).  
#define BUZZER_ENABLED 0
#define BUZZER_PIN -1
#define BUZZER_VOLUME 100

// Extra Button Add-on setting
#define EXTRA_BUTTON_MASK 0 // 0 means none, get other mask from GamepadState.h
                            // For directions, use GAMEPAD_MASK_DU, GAMEPAD_MASK_DD, GAMEPAD_MASK_DL and GAMEPAD_MASK_DR
#define EXTRA_BUTTON_PIN -1

// Button Lock Add-on setting
#define FOCUS_MODE_BUTTON_MASK 0 // 0 means none, get other mask from GamepadState.h
                            // For directions, use GAMEPAD_MASK_DU, GAMEPAD_MASK_DD, GAMEPAD_MASK_DL and GAMEPAD_MASK_DR
#define FOCUS_MODE_PIN -1
#define FOCUS_MODE_OLED_LOCK_ENABLED 0
#define FOCUS_MODE_RGB_LOCK_ENABLED 0
#define FOCUS_MODE_BUTTON_LOCK_ENABLED 0



// Keyboard Host Addon defaults
#define KEYBOARD_HOST_ENABLED 0
#define KEYBOARD_HOST_PIN_DPLUS -1
#define KEYBOARD_HOST_PIN_5V -1

// PS Passthrough Host Addon defaults
#define PSPASSTHROUGH_HOST_ENABLED 0
#define PSPASSTHROUGH_HOST_PIN_DPLUS -1
#define PSPASSTHROUGH_HOST_PIN_5V -1

// For details on this, see: https://gp2040-ce.info/development?id=i2c-display-splash
#define DEFAULT_SPLASH \
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x40,0x80,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0xDF,0xC0, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xC1,0xFF,0xA0, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xCF,0xFE,0x80, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xE0,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xAB,0xC0,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2C,0x00,0x00,0xFB,0x83,0xFF,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x01,0xFF,0x00,0x01,0xF3,0x07,0xFC,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1C,0x07,0xFF,0x00,0x03,0xE0,0x3F,0xF4,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x2C,0x38,0x0A,0xE7,0x80,0x03,0xC0,0x5F,0x80,0x00, \
	0x00,0x00,0x00,0x20,0x07,0x81,0xFF,0x39,0xC3,0xC7,0x01,0xD7,0x80,0x3F,0x00,0x00, \
	0x00,0x00,0x00,0xE0,0x1F,0xE7,0xFF,0x79,0xC7,0x87,0x9F,0xA7,0x80,0x6E,0x3C,0x00, \
	0x00,0x00,0xA0,0xC0,0x3F,0xEA,0xE7,0xF3,0xEF,0x07,0x7F,0xC7,0x01,0xCF,0xFF,0x80, \
	0x00,0x03,0x8F,0xFC,0x7F,0xC3,0xC7,0x7F,0xFE,0x0F,0x7E,0x8F,0x87,0xDF,0xFC,0x00, \
	0x00,0x0F,0xFF,0xFF,0x73,0xC7,0x87,0xFF,0xDE,0x0E,0x50,0x07,0x1F,0x1F,0xD0,0x00, \
	0x00,0x1F,0x57,0xFF,0xE7,0x8F,0x07,0xFF,0x1C,0x3E,0x00,0x0F,0xFF,0x7F,0x00,0x00, \
	0x00,0x7E,0x0B,0x8F,0x8F,0x8E,0x0F,0xFF,0x9E,0x7C,0x00,0x07,0xFD,0xFC,0x00,0x00, \
	0x00,0x7C,0x37,0x9F,0x1F,0x1E,0x0F,0x87,0x1E,0xF8,0x00,0x07,0xF0,0x30,0x00,0x00, \
	0x00,0xF1,0xF7,0xFE,0x1E,0x1C,0x3E,0x0E,0x1F,0xF0,0x00,0x01,0x40,0x28,0x00,0x00, \
	0x01,0xEF,0xE7,0xF8,0x3C,0x3E,0x7C,0x0F,0x1F,0xE0,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x03,0xFF,0x8F,0xF0,0x7F,0xFE,0xF8,0x0E,0x0F,0x80,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x03,0xFF,0xCF,0xC0,0xFF,0xFF,0xF0,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x07,0x8B,0xFF,0x01,0xFF,0x8F,0xC0,0x06,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x07,0x87,0xFE,0x01,0xFE,0x0F,0x80,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x07,0x1F,0xDE,0x03,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x07,0xBF,0x9E,0x01,0xC0,0x00,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x07,0xFF,0x9C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x07,0xFB,0x38,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x03,0xE7,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x01,0x47,0x28,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x0E,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x0C,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x08,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00

#endif






syntax = "proto2";

import "nanopb.proto";

enum ButtonLayout
{
    option (nanopb_enumopt).long_names = false;

    BUTTON_LAYOUT_STICK = 0;
    BUTTON_LAYOUT_STICKLESS = 1;
    BUTTON_LAYOUT_BUTTONS_ANGLED = 2;
    BUTTON_LAYOUT_BUTTONS_BASIC = 3;
    BUTTON_LAYOUT_KEYBOARD_ANGLED = 4;
    BUTTON_LAYOUT_KEYBOARDA = 5;
    BUTTON_LAYOUT_DANCEPADA = 6;
    BUTTON_LAYOUT_TWINSTICKA = 7;
    BUTTON_LAYOUT_BLANKA = 8;
    BUTTON_LAYOUT_VLXA = 9;
    BUTTON_LAYOUT_FIGHTBOARD_STICK = 10;
    BUTTON_LAYOUT_FIGHTBOARD_MIRRORED = 11;
    BUTTON_LAYOUT_CUSTOMA = 12;
    BUTTON_LAYOUT_OPENCORE0WASDA = 13;
}

enum ButtonLayoutRight
{
    option (nanopb_enumopt).long_names = false;

    BUTTON_LAYOUT_ARCADE = 0;
    BUTTON_LAYOUT_STICKLESSB = 1;
    BUTTON_LAYOUT_BUTTONS_ANGLEDB = 2;
    BUTTON_LAYOUT_VEWLIX = 3;
    BUTTON_LAYOUT_VEWLIX7 = 4;
    BUTTON_LAYOUT_CAPCOM = 5;
    BUTTON_LAYOUT_CAPCOM6 = 6;
    BUTTON_LAYOUT_SEGA2P = 7;
    BUTTON_LAYOUT_NOIR8 = 8;
    BUTTON_LAYOUT_KEYBOARDB = 9;
    BUTTON_LAYOUT_DANCEPADB = 10;
    BUTTON_LAYOUT_TWINSTICKB = 11;
    BUTTON_LAYOUT_BLANKB = 12;
    BUTTON_LAYOUT_VLXB = 13;
    BUTTON_LAYOUT_FIGHTBOARD = 14;
    BUTTON_LAYOUT_FIGHTBOARD_STICK_MIRRORED = 15;
    BUTTON_LAYOUT_CUSTOMB = 16;
    BUTTON_LAYOUT_KEYBOARD8B = 17;
    BUTTON_LAYOUT_OPENCORE0WASDB = 18;
}

enum SplashMode
{
    option (nanopb_enumopt).long_names = false;

    SPLASH_MODE_STATIC = 0;
    SPLASH_MODE_CLOSEIN = 1;
    SPLASH_MODE_CLOSEINCUSTOM = 2;
    SPLASH_MODE_NONE = 3;
}

enum SplashChoice
{
    option (nanopb_enumopt).long_names = false;

    SPLASH_CHOICE_MAIN = 0;
    SPLASH_CHOICE_X = 1;
    SPLASH_CHOICE_Y = 2;
    SPLASH_CHOICE_Z = 3;
    SPLASH_CHOICE_CUSTOM = 4;
    SPLASH_CHOICE_LEGACY = 5;
}

enum OnBoardLedMode
{
    option (nanopb_enumopt).long_names = false;
    
    ON_BOARD_LED_MODE_OFF = 0;
    ON_BOARD_LED_MODE_MODE_INDICATOR = 1;
    ON_BOARD_LED_MODE_INPUT_TEST = 2;
    ON_BOARD_LED_MODE_PS_AUTH = 3;
}




enum InvertMode
{
    option (nanopb_enumopt).long_names = false;

    INVERT_NONE = 0;
    INVERT_X = 1;
    INVERT_Y = 2;
    INVERT_XY = 3;
}

enum SOCDMode
{
    option (nanopb_enumopt).long_names = false;

    SOCD_MODE_UP_PRIORITY = 0;				// U+D=U, L+R=N
    SOCD_MODE_NEUTRAL = 1;					// U+D=N, L+R=N
    SOCD_MODE_SECOND_INPUT_PRIORITY = 2;	// U>D=D, L>R=R (Last Input Priority, aka Last Win)
    SOCD_MODE_FIRST_INPUT_PRIORITY = 3;		// U>D=U, L>R=L (First Input Priority, aka First Win)
    SOCD_MODE_BYPASS = 4;					// U+D=UD, L+R=LR (No cleaning applied)
}

enum GpioAction
{
    option (nanopb_enumopt).long_names = false;

    // the lowest value is the default, which should be NONE;
    // reserving some numbers in case we need more not-mapped type values
    NONE                         = -10;
    RESERVED                     = -5;
    ASSIGNED_TO_ADDON            = 0;
    BUTTON_PRESS_UP              = 1;
    BUTTON_PRESS_DOWN            = 2;
    BUTTON_PRESS_LEFT            = 3;
    BUTTON_PRESS_RIGHT           = 4;
    BUTTON_PRESS_B1              = 5;
    BUTTON_PRESS_B2              = 6;
    BUTTON_PRESS_B3              = 7;
    BUTTON_PRESS_B4              = 8;
    BUTTON_PRESS_L1              = 9;
    BUTTON_PRESS_R1              = 10;
    BUTTON_PRESS_L2              = 11;
    BUTTON_PRESS_R2              = 12;
    BUTTON_PRESS_S1              = 13;
    BUTTON_PRESS_S2              = 14;
    BUTTON_PRESS_A1              = 15;
    BUTTON_PRESS_A2              = 16;
    BUTTON_PRESS_L3              = 17;
    BUTTON_PRESS_R3              = 18;
    BUTTON_PRESS_FN              = 19;
    BUTTON_PRESS_DDI_UP          = 20;
    BUTTON_PRESS_DDI_DOWN        = 21;
    BUTTON_PRESS_DDI_LEFT        = 22;
    BUTTON_PRESS_DDI_RIGHT       = 23;
    SUSTAIN_DP_MODE_DP           = 24;
    SUSTAIN_DP_MODE_LS           = 25;
    SUSTAIN_DP_MODE_RS           = 26;
    SUSTAIN_SOCD_MODE_UP_PRIO    = 27;
    SUSTAIN_SOCD_MODE_NEUTRAL    = 28;
    SUSTAIN_SOCD_MODE_SECOND_WIN = 29;
    SUSTAIN_SOCD_MODE_FIRST_WIN  = 30;
    SUSTAIN_SOCD_MODE_BYPASS     = 31;
}

enum GamepadHotkey
{
    option (nanopb_enumopt).long_names = false;

    HOTKEY_NONE                  = 0;
    HOTKEY_DPAD_DIGITAL          = 1;
    HOTKEY_DPAD_LEFT_ANALOG      = 2;
    HOTKEY_DPAD_RIGHT_ANALOG     = 3;
    HOTKEY_HOME_BUTTON           = 4;
    HOTKEY_CAPTURE_BUTTON        = 5;
    HOTKEY_SOCD_UP_PRIORITY      = 6;
    HOTKEY_SOCD_NEUTRAL          = 7;
    HOTKEY_SOCD_LAST_INPUT       = 8;
    HOTKEY_INVERT_X_AXIS         = 9;
    HOTKEY_INVERT_Y_AXIS         = 10;
    HOTKEY_SOCD_FIRST_INPUT      = 11;
    HOTKEY_SOCD_BYPASS           = 12;
    HOTKEY_TOGGLE_4_WAY_MODE     = 13;
    HOTKEY_TOGGLE_DDI_4_WAY_MODE = 14;
    HOTKEY_LOAD_PROFILE_1        = 15;
    HOTKEY_LOAD_PROFILE_2        = 16;
    HOTKEY_LOAD_PROFILE_3        = 17;
    HOTKEY_LOAD_PROFILE_4        = 18;
    HOTKEY_L3_BUTTON             = 19;
    HOTKEY_R3_BUTTON             = 20;
    HOTKEY_TOUCHPAD_BUTTON       = 21;
    HOTKEY_REBOOT_DEFAULT        = 22;
    HOTKEY_B1_BUTTON             = 23;
    HOTKEY_B2_BUTTON             = 24;
    HOTKEY_B3_BUTTON             = 25;
    HOTKEY_B4_BUTTON             = 26;
    HOTKEY_L1_BUTTON             = 27;
    HOTKEY_R1_BUTTON             = 28;
    HOTKEY_L2_BUTTON             = 29;
    HOTKEY_R2_BUTTON             = 30;
    HOTKEY_S1_BUTTON             = 31;
    HOTKEY_S2_BUTTON             = 32;
    HOTKEY_A1_BUTTON             = 33;
    HOTKEY_A2_BUTTON             = 34;
}



enum ShmupMixMode
{
    option (nanopb_enumopt).long_names = false;

    SHMUP_MIX_MODE_TURBO_PRIORITY = 0;
    SHMUP_MIX_MODE_CHARGE_PRIORITY = 1;
}





enum MacroType
{
    option (nanopb_enumopt).long_names = false;

    ON_PRESS = 1;
    ON_HOLD_REPEAT = 2;
    ON_TOGGLE = 3;
};





#define PIN_BUTTON_TURBO -1         // Turbo
#define PIN_BUTTON_REVERSE -1       // UDLR Reverse
#define PIN_SLIDER_ONE    -1         // Left Stick Slider
#define PIN_SLIDER_TWO    -1         // Right Stick Slider
#define PIN_SLIDER_SOCD_ONE    -1         // SOCD Slider Pin One
#define PIN_SLIDER_SOCD_TWO    -1         // SOCD Slider Pin Two
#define SLIDER_MODE_ONE DPAD_MODE_LEFT_ANALOG
#define SLIDER_MODE_TWO DPAD_MODE_RIGHT_ANALOG
#define SLIDER_MODE_ZERO DPAD_MODE_DIGITAL