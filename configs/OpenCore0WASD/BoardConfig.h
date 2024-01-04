/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"

#define BOARD_CONFIG_LABEL "Open_Core0 WASD"

// Main pin mapping Configuration
//                          // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define PIN_DPAD_UP     10  // UP     | UP     | UP      | UP       | UP     | UP     |
#define PIN_DPAD_DOWN   11  // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define PIN_DPAD_RIGHT  12  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define PIN_DPAD_LEFT   9   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define PIN_BUTTON_B1   14  // B1     | A      | B       | Cross    | 2      | K1     |
#define PIN_BUTTON_B2   15  // B2     | B      | A       | Circle   | 3      | K2     |
#define PIN_BUTTON_R2   16  // R2     | RT     | ZR      | R2       | 8      | K3     |
#define PIN_BUTTON_L2   17  // L2     | LT     | ZL      | L2       | 7      | K4     |
#define PIN_BUTTON_B3   18  // B3     | X      | Y       | Square   | 1      | P1     |
#define PIN_BUTTON_B4   19  // B4     | Y      | X       | Triangle | 4      | P2     |
#define PIN_BUTTON_R1   20  // R1     | RB     | R       | R1       | 6      | P3     |
#define PIN_BUTTON_L1   21  // L1     | LB     | L       | L1       | 5      | P4     |
#define PIN_BUTTON_S1   6   // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define PIN_BUTTON_S2   7   // S2     | Start  | Plus    | Start    | 10     | Start  |
#define PIN_BUTTON_L3   3   // L3     | LS     | LS      | L3       | 11     | LS     |
#define PIN_BUTTON_R3   2   // R3     | RS     | RS      | R3       | 12     | RS     |
#define PIN_BUTTON_A1   5   // A1     | Guide  | Home    | PS       | 13     | ~      |
#define PIN_BUTTON_A2   4   // A2     | ~      | Capture | ~        | 14     | ~      |
#define PIN_BUTTON_FN   -1  // Hotkey Function                                        |

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

#define DEFAULT_PS4CONTROLLER_TYPE PS4_ARCADESTICK

#define DEFAULT_INPUT_MODE_R1 INPUT_MODE_XBONE

#define TURBO_ENABLED 1
#define PIN_BUTTON_TURBO 27

#define JSLIDER_ENABLED 1
#define PIN_SLIDER_ONE 26
#define PIN_SLIDER_TWO 25

#define PIN_SLIDER_SOCD_ONE 24
#define PIN_SLIDER_SOCD_TWO 23

#define BOARD_LEDS_PIN 8

#define LED_BRIGHTNESS_MAXIMUM 50

#define LEDS_DPAD_UP     0
#define LEDS_DPAD_LEFT   1
#define LEDS_DPAD_DOWN   2
#define LEDS_DPAD_RIGHT  3
#define LEDS_BUTTON_B3   4
#define LEDS_BUTTON_B4   5
#define LEDS_BUTTON_R1   6
#define LEDS_BUTTON_L1   7
#define LEDS_BUTTON_L2   8
#define LEDS_BUTTON_R2   9
#define LEDS_BUTTON_B2   10
#define LEDS_BUTTON_B1   11

#define PLED_TYPE PLED_TYPE_RGB
#define PLED1_PIN 13
#define PLED2_PIN 14
#define PLED3_PIN 15
#define PLED4_PIN 16

#define PLED_COLOR ColorGreen

#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1
#define DISPLAY_FLIP 3

#define BUTTON_LAYOUT BUTTON_LAYOUT_OPENCORE0WASDA
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_OPENCORE0WASDB
#define SPLASH_MODE SPLASH_MODE_STATIC

#define EXTRA_BUTTON_MASK GAMEPAD_MASK_DU
#define EXTRA_BUTTON_PIN 13

#define FOCUS_MODE_ENABLED 1
#define FOCUS_MODE_BUTTON_MASK GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2 | GAMEPAD_MASK_A1 | GAMEPAD_MASK_A2 | GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3
#define FOCUS_MODE_PIN 22
#define FOCUS_MODE_BUTTON_LOCK_ENABLED 1

#define PSPASSTHROUGH_ENABLED 1
#define PSPASSTHROUGH_PIN_DPLUS 28

#define XBONEPASSTHROUGH_ENABLED 1

#endif