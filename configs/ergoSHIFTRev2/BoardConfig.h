/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "ergoSHIFT Rev.2"

// Main pin mapping Configuration
//                                                  // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_UP     // UP     | UP     | UP      | UP       | UP     | UP     |
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_DOWN   // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_RIGHT  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_LEFT   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define GPIO_PIN_18 GpioAction::BUTTON_PRESS_B1     // B1     | A      | B       | Cross    | 2      | K1     |
#define GPIO_PIN_19 GpioAction::BUTTON_PRESS_B2     // B2     | B      | A       | Circle   | 3      | K2     |
#define GPIO_PIN_23 GpioAction::BUTTON_PRESS_R2     // R2     | RT     | ZR      | R2       | 8      | K3     |
#define GPIO_PIN_16 GpioAction::BUTTON_PRESS_L2     // L2     | LT     | ZL      | L2       | 7      | K4     |
#define GPIO_PIN_20 GpioAction::BUTTON_PRESS_B3     // B3     | X      | Y       | Square   | 1      | P1     |
#define GPIO_PIN_25 GpioAction::BUTTON_PRESS_B4     // B4     | Y      | X       | Triangle | 4      | P2     |
#define GPIO_PIN_26 GpioAction::BUTTON_PRESS_R1     // R1     | RB     | R       | R1       | 6      | P3     |
#define GPIO_PIN_24 GpioAction::BUTTON_PRESS_L1     // L1     | LB     | L       | L1       | 5      | P4     |
#define GPIO_PIN_27 GpioAction::BUTTON_PRESS_S1     // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define GPIO_PIN_00 GpioAction::BUTTON_PRESS_S2     // S2     | Start  | Plus    | Start    | 10     | Start  |
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_L3     // L3     | LS     | LS      | L3       | 11     | LS     |
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_R3     // R3     | RS     | RS      | R3       | 12     | RS     |
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_A1     // A1     | Guide  | Home    | PS       | 13     | ~      |
#define GPIO_PIN_01 GpioAction::BUTTON_PRESS_A2     // A2     | ~      | Capture | ~        | 14     | ~      |

// Setting GPIO pins to assigned by add-on
//
#define GPIO_PIN_04 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_05 GpioAction::ASSIGNED_TO_ADDON

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
#define I2C0_ENABLED 1
#define I2C0_PIN_SDA 4
#define I2C0_PIN_SCL 5
#define DISPLAY_I2C_BLOCK i2c0
#define DISPLAY_FLIP 0
#define DISPLAY_INVERT 0

#define BUTTON_LAYOUT BUTTON_LAYOUT_STICKLESS
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_STICKLESSB
#define SPLASH_MODE SPLASH_MODE_STATIC
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