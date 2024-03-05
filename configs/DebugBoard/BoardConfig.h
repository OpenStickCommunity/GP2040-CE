/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 *
 * Debug setup
 * ------------------------------------------------------------------------------------
 * 2x Raspberry Pi Pico (2nd is for picoprobe debugging)
 * 1x Waveshare 1.3inch LCD Display Module - https://www.waveshare.com/pico-lcd-1.3.htm
 * 1x Waveshare RGB 16x10 LED Matrix - https://www.waveshare.com/pico-rgb-led.htm
 * 1x Waveshare Quad GPIO Expander - https://www.waveshare.com/pico-quad-expander.htm
 *
 */

#ifndef DEBUG_BOARD_CONFIG_H_
#define DEBUG_BOARD_CONFIG_H_

#include "enums.pb.h"

#define BOARD_CONFIG_LABEL "Debug Board"

// Remaining pins (0, 1 reserved for I2C): 16

#define PIN_SHMUP_DIAL 26
#define TURBO_ENABLED 1
#define TURBO_LED_PIN 20

// Main pin mapping Configuration
//                          // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define GPIO_PIN_04 GpioAction::BUTTON_PRESS_UP     // UP     | UP     | UP      | UP       | UP     | UP     |
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_DOWN   // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_RIGHT  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_LEFT   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define GPIO_PIN_22 GpioAction::BUTTON_PRESS_B1     // B1     | A      | B       | Cross    | 2      | K1     |
#define GPIO_PIN_21 GpioAction::BUTTON_PRESS_B2     // B2     | B      | A       | Circle   | 3      | K2     |
// #define GPIO_PIN_19 GpioAction::BUTTON_PRESS_R2     // R2     | RT     | ZR      | R2       | 8      | K3     |
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_L2     // L2     | LT     | ZL      | L2       | 7      | K4     |
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_B3     // B3     | X      | Y       | Square   | 1      | P1     |
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_B4     // B4     | Y      | X       | Triangle | 4      | P2     |
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_R1     // R1     | RB     | R       | R1       | 6      | P3     |
#define GPIO_PIN_15 GpioAction::BUTTON_PRESS_L1     // L1     | LB     | L       | L1       | 5      | P4     |
#define GPIO_PIN_28 GpioAction::BUTTON_PRESS_S1     // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define GPIO_PIN_27 GpioAction::BUTTON_PRESS_S2     // S2     | Start  | Plus    | Start    | 10     | Start  |
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_L3     // L3     | LS     | LS      | L3       | 11     | LS     |
// #define GPIO_PIN_08 GpioAction::BUTTON_PRESS_R3     // R3     | RS     | RS      | R3       | 12     | RS     |
#define GPIO_PIN_16 GpioAction::BUTTON_PRESS_A1     // A1     | Guide  | Home    | PS       | 13     | ~      |
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_A2     // A2     | ~      | Capture | ~        | 14     | ~      |
#define GPIO_PIN_08 GpioAction::BUTTON_PRESS_TURBO

// Keyboard Mapping Configuration
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
#define KEY_BUTTON_FN   -1                    // Hotkey Function                                        |
#define DEFAULT_INPUT_MODE_R1 INPUT_MODE_XBONE
#define DEFAULT_PS4CONTROLLER_TYPE PS4_ARCADESTICK

// #define TURBO_ENABLED 0
// #define GPIO_PIN_14 GpioAction::BUTTON_PRESS_TURBO

#define BOARD_LEDS_PIN 7

#define LED_BRIGHTNESS_MAXIMUM 50
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LEDS_PER_PIXEL 4
#define LEDS_BASE_ANIMATION_INDEX 1

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
#define LEDS_BUTTON_A1   12
#define LEDS_BUTTON_L3   13
#define LEDS_BUTTON_R3   14
#define LEDS_BUTTON_A2   15

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
// #define PLED1_PIN 20
// #define PLED2_PIN 11
// #define PLED3_PIN 18
// #define PLED4_PIN 13


#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1
#define I2C_BLOCK i2c0
#define I2C_SPEED 1000000

#define I2C0_ENABLED 1
#define I2C0_PIN_SDA 0
#define I2C0_PIN_SCL 1

#define I2C1_ENABLED 1
#define I2C1_PIN_SDA 18
#define I2C1_PIN_SCL 19

#define BUTTON_LAYOUT BUTTON_LAYOUT_KEYBOARDA
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_VEWLIX
#define SPLASH_MODE SPLASH_MODE_STATIC
#define SPLASH_DURATION 3000 // Duration in milliseconds

#define BOARD_LED_ENABLED 1
#define BOARD_LED_TYPE ON_BOARD_LED_MODE_MODE_INDICATOR

#endif
