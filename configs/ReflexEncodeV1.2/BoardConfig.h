/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"

#define BOARD_CONFIG_LABEL "Reflex Encode v1.2"

// Main pin mapping Configuration
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

#define PIN_BUTTON_TURBO 14

#define BOARD_LED_ENABLED 1
#define BOARD_LED_TYPE ON_BOARD_LED_MODE_PS_AUTH

#define DEFAULT_PS4CONTROLLER_TYPE PS4_ARCADESTICK

#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 0
#define I2C_SCL_PIN 1
#define SPLASH_MODE SPLASH_MODE_STATIC
#define SPLASH_CHOICE SPLASH_CHOICE_MAIN


#define DEFAULT_SPLASH \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xB0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xBC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x02,0xDE,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x2F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x77,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFB,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xBD,0xE0,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xDE,0xF0,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xDF,0x78,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x6F,0xB8,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0xDC,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x37,0xEE,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1B,0xF7,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFB,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x39,0xFD,0x80,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3D,0xFE,0xC0,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0xFF,0x60,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x38,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7E,0x7F,0xF0,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0x7F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xBF,0xF8,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0x9F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xDF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xCF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xEF,0xFF,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xEF,0xFF,0x80,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xC0,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x01,0xFF,0xFF,0xF7,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x03,0xFF,0xDD,0xF7,0xFF,0xE0,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x07,0xFF,0xDB,0xFB,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xD7,0xFB,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x0F,0xFF,0xE1,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0x7D,0xFF,0xF8,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x3F,0xFF,0xDB,0xFD,0xFF,0xFC,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xBB,0xFE,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x7F,0xFF,0xBF,0xFE,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0xFF,0xFF,0x5F,0xFE,0xFF,0xFF,0x80,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x01,0xFF,0xFE,0xFF,0xF1,0xFF,0xFF,0x80,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x03,0xFF,0xFE,0xEE,0x03,0x7F,0xFF,0xC0,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x07,0xFF,0xFD,0xE7,0xFB,0x9F,0xFF,0xE0,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x0F,0xFF,0xFB,0xEF,0xFB,0xFF,0xFF,0xF0,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x1F,0xFC,0x0F,0xDF,0xFB,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x1F,0xFF,0xFF,0xDF,0xF1,0xFF,0xFF,0xF8,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x3F,0xFF,0xFF,0xCF,0xE0,0x3F,0xFF,0xFC,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x7F,0xFF,0xFF,0xC0,0x00,0x1F,0xFF,0xFE,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0xFF,0xFF,0xFF,0x80,0x00,0x0F,0xFF,0xFF,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x01,0xFF,0xFF,0xF0,0x00,0x00,0x07,0xFF,0xFF,0x80,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0xFF,0xFF,0xE0,0x00,0x00,0x03,0xFF,0xFF,0xC0,0x00,0x00,0x00, \
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0xFC,0x3F,0xE3,0xFC,0xE0,0x1F,0xF7,0x01,0xC0,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0xFE,0x3F,0xE3,0xFC,0xE0,0x1F,0xF3,0x83,0x80,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0x8E,0x38,0x03,0x80,0xE0,0x1C,0x01,0xC7,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0x8E,0x38,0x03,0x80,0xE0,0x1C,0x01,0xEF,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0x8E,0x38,0x03,0x80,0xE0,0x1C,0x00,0xFE,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0xFC,0x3F,0xC3,0xF8,0xE0,0x1F,0xE0,0x7C,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0xF8,0x3F,0xC3,0xF8,0xE0,0x1F,0xE0,0x7C,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0x9C,0x38,0x03,0x80,0xE0,0x1C,0x00,0xFE,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0x9C,0x38,0x03,0x80,0xE0,0x1C,0x01,0xEF,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0x8E,0x38,0x03,0x80,0xE0,0x1C,0x01,0xC7,0x00,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0x8E,0x3F,0xE3,0x80,0xFF,0x1F,0xF3,0x83,0x80,0x00,0x00,0x00, \
0x00,0x00,0x00,0x03,0x87,0x3F,0xE3,0x80,0xFF,0x1F,0xF7,0x01,0xC0,0x00,0x00,0x00

#endif