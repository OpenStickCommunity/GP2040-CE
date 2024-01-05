/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef _ERGOSHIFT_REV2_BOARD_CONFIG_H_
#define _ERGOSHIFT_REV2_BOARD_CONFIG_H_

#include "enums.pb.h"

#define BOARD_CONFIG_LABEL "ergoSHIFT Rev.2"

// Main pin mapping Configuration
//                          // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define PIN_DPAD_UP     12  // UP     | UP     | UP      | UP       | UP     | UP     |
#define PIN_DPAD_DOWN   9   // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   | 
#define PIN_DPAD_RIGHT  11  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  | 
#define PIN_DPAD_LEFT   6   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   | 
#define PIN_BUTTON_B1   18  // B1     | A      | B       | Cross    | 2      | K1     |
#define PIN_BUTTON_B2   19  // B2     | B      | A       | Circle   | 3      | K2     |
#define PIN_BUTTON_R2   23  // R2     | RT     | ZR      | R2       | 8      | K3     |
#define PIN_BUTTON_L2   16  // L2     | LT     | ZL      | L2       | 7      | K4     |
#define PIN_BUTTON_B3   20  // B3     | X      | Y       | Square   | 1      | P1     |
#define PIN_BUTTON_B4   25  // B4     | Y      | X       | Triangle | 4      | P2     |
#define PIN_BUTTON_R1   26  // R1     | RB     | R       | R1       | 6      | P3     |
#define PIN_BUTTON_L1   24  // L1     | LB     | L       | L1       | 5      | P4     |
#define PIN_BUTTON_S1   27  // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define PIN_BUTTON_S2   0   // S2     | Start  | Plus    | Start    | 10     | Start  |
#define PIN_BUTTON_L3   10  // L3     | LS     | LS      | L3       | 11     | LS     |
#define PIN_BUTTON_R3   17  // R3     | RS     | RS      | R3       | 12     | RS     |
#define PIN_BUTTON_A1   2   // A1     | Guide  | Home    | PS       | 13     | ~      |
#define PIN_BUTTON_A2   1   // A2     | ~      | Capture | ~        | 14     | ~      |
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

#define HAS_I2C_DISPLAY 1
#define I2C_SDA_PIN 4
#define I2C_SCL_PIN 5
#define I2C_BLOCK i2c0
#define I2C_SPEED 400000
#define DISPLAY_FLIP 0
#define DISPLAY_INVERT 0

#define BUTTON_LAYOUT BUTTON_LAYOUT_STICKLESS
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_STICKLESSB
#define SPLASH_MODE SPLASH_MODE_STATIC
#define SPLASH_CHOICE SPLASH_CHOICE_MAIN
#define SPLASH_DURATION 7500

/**
 * Made with Marlin Bitmap Converter
 * https://marlinfw.org/tools/u8glib/converter.html
 *
 * This bitmap from the file 'ErgoSHIFT-logo-128x64.bmp'
 */

#define DEFAULT_SPLASH \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x3F,0xE0,0x0F, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0x03,0xC0,0x01, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xC0,0x01,0xE0,0x00, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,0x7F,0x81,0xE0,0x03,0xF8,0x01, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,0x3C,0x00,0x3F,0xFF,0x87,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF8,0x00,0x7C,0x00,0x1F,0xFC,0x00,0x3F, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0x01,0xFC,0x00,0x1F,0xFC,0x00,0x0F, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0x3F,0xFF,0xFF,0x80,0x7F,0xFE,0x00,0x07, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0x73,0x3F,0xFC,0x00,0xFF,0xF8,0x7F,0x80,0x07, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x62,0x6F,0x3F,0xF0,0x00,0x3F,0x80,0x03,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0xDE,0xDF,0x7F,0xF0,0x00,0x1F,0x00,0x00,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xF0,0x77,0xBC,0xDE,0xFF,0xF0,0x00,0x1F,0x80,0x00,0x7F,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xEF,0x6F,0x3D,0xC1,0xFF,0xFC,0x00,0x7F,0xC0,0x00,0x7F,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xC0,0xCF,0x01,0xFF,0xF0,0x0F,0xFF,0xFF,0xF0,0x01,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0x9F,0xDF,0xBB,0xFF,0xC0,0x01,0xFF,0xC0,0x3F,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xBD,0x9F,0xE7,0xFF,0x00,0x00,0xFE,0x00,0x07,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0x83,0xFE,0x1F,0xFF,0x00,0x00,0x7C,0x00,0x01,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0xFC,0x00,0x00,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0x01,0xFE,0x00,0x00,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xF0,0x07,0xFF,0x00,0x00,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xE0,0xFF,0xFF,0x03,0xFF,0xFF,0xFF,0xC0,0x01,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFE,0x00,0x1F,0xF0,0x00,0x7F,0xFE,0x03,0xFC,0x1F,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xF0,0x00,0x0F,0xC0,0x00,0x1F,0xF0,0x00,0x7F,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xE0,0x00,0x07,0x80,0x00,0x1F,0xC0,0x00,0x1F,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xC0,0x00,0x07,0x80,0x00,0x0F,0xC0,0x00,0x0F,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xF0,0x3F,0xC0,0x00,0x0F,0x80,0x00,0x1F,0x80,0x00,0x07,0xFF,0xFF,0xFF,0xFF, \
0xFE,0x00,0x0F,0xC0,0x00,0x1F,0x80,0x00,0x3F,0xC0,0x00,0x07,0xFF,0xFF,0xFF,0xFF, \
0xF8,0x00,0x07,0xC0,0x00,0x7F,0xC0,0x00,0x7F,0xC0,0x00,0x07,0xFF,0xFF,0xFF,0xFF, \
0xE0,0x00,0x07,0xF0,0x03,0xFF,0xF8,0x03,0xFF,0xE0,0x00,0x0F,0xFF,0xFF,0xFF,0xFF, \
0xC0,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xE1,0xF8,0x00,0x1F,0xFF,0xFF,0xFF,0xFF, \
0x80,0x00,0x0F,0xFF,0xFF,0xFF,0xFF,0xFC,0x07,0xFF,0x81,0xFF,0xFF,0xFF,0xFF,0xFF, \
0x00,0x00,0x1F,0xFF,0xFF,0xFF,0xFF,0x20,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0x00,0x00,0x3F,0xFF,0xFF,0xFF,0xE0,0x3E,0x7F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0x00,0x00,0xFF,0xFF,0xFF,0xFF,0x87,0xFE,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0x80,0x03,0xFF,0xFF,0xFF,0x87,0x3F,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xF8,0x7F,0xFF,0xFF,0xFF,0x1F,0x3F,0xFC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xEF,0x9F,0x7F,0xF9,0xFF,0xFF,0xFC,0x00,0x7F,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xDF,0xCF,0x9E,0x41,0xF9,0xFF,0xFF,0xE0,0x00,0x0F,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0x9F,0xCF,0x3E,0x07,0xFB,0xFF,0xFF,0x80,0x00,0x03,0xFF,0xFF,0xFF, \
0xFF,0xFE,0x03,0x9F,0x9F,0x3E,0xFF,0xF3,0xFF,0xFF,0x00,0x00,0x00,0xFF,0xFF,0xFF, \
0xFF,0xF8,0xE7,0xBF,0x9F,0x7C,0xFF,0xF3,0xFF,0xFE,0x00,0x00,0x00,0x7F,0xFF,0xFF, \
0xFF,0xF3,0xFF,0x38,0x3E,0x7C,0xFF,0xE3,0xFF,0xFE,0x00,0x00,0x00,0x1F,0xFF,0xFF, \
0xFF,0xE7,0xFF,0x01,0x3E,0x79,0xFC,0x00,0x7F,0xFC,0x00,0x00,0x00,0x0F,0xFF,0xFF, \
0xFF,0xCF,0xFE,0x3F,0x3C,0xF9,0xE0,0x00,0x0F,0xFC,0x00,0x00,0x00,0x0F,0xFF,0xFF, \
0xFF,0xC7,0xFE,0x7F,0x7C,0xFB,0xC0,0x00,0x07,0xFC,0x00,0x00,0x00,0x07,0xFF,0xFF, \
0xFF,0xC0,0x1E,0xFE,0x7C,0x73,0x00,0x00,0x03,0xFE,0x00,0x00,0x00,0x07,0xFF,0xFF, \
0xFF,0xFF,0x1C,0xFE,0x70,0x7E,0x00,0x00,0x01,0xFE,0x00,0x00,0x00,0x07,0xFF,0xFF, \
0xFF,0xFF,0x9C,0xFC,0xFF,0xFE,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x07,0xFF,0xFF, \
0xFF,0xFF,0xBD,0xFF,0xFF,0xFC,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0x07,0xFF,0xFF, \
0xFF,0xFF,0x39,0xFF,0xFF,0xFC,0x00,0x00,0x00,0xFF,0x80,0x00,0x00,0x07,0xFF,0xFF, \
0xFE,0xFC,0xFF,0xFF,0xFF,0xFC,0x00,0x00,0x00,0xFF,0xE0,0x00,0x00,0x0F,0xFF,0xFF, \
0xFE,0x01,0xFF,0xFF,0xFF,0xFC,0x00,0x00,0x00,0xFF,0xF0,0x00,0x00,0x1F,0xFF,0xFF, \
0xFF,0x3F,0xFF,0xFF,0xFF,0xFC,0x00,0x00,0x00,0xFF,0xFC,0x00,0x00,0x3F,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0x00,0x00,0x01,0xFF,0xFF,0x80,0x00,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFE,0x00,0x00,0x03,0xFF,0xFF,0xF8,0x07,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x00,0x00,0x07,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0x80,0x00,0x0F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xE0,0x00,0x3F,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFC,0x01,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, \
0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF

#endif