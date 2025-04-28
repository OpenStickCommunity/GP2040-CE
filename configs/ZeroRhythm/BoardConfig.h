/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "Zero_Rhythm"

// Main pin mapping Configuration
//                                                  // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define GPIO_PIN_01 GpioAction::BUTTON_PRESS_R2     // R2     | RT     | ZR      | R2       | 8      | K3     |
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_B2     // B2     | B      | A       | Circle   | 3      | K2     |
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_B4     // B4     | Y      | X       | Triangle | 4      | P2     |
#define GPIO_PIN_04 GpioAction::BUTTON_PRESS_B1     // B1     | A      | B       | Cross    | 2      | K1     |
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_B3     // B3     | X      | Y       | Square   | 1      | P1     |
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_R1     // R1     | RB     | R       | R1       | 6      | P3     |
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_A2     // A2     | ~      | Capture | ~        | 14     | ~      |
#define GPIO_PIN_08 GpioAction::BUTTON_PRESS_A1     // A1     | Guide  | Home    | PS       | 13     | ~      |
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_S1     // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_S2     // S2     | Start  | Plus    | Start    | 10     | Start  |
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_L1     // L1     | LB     | L       | L1       | 5      | P4     |
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_RIGHT  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define GPIO_PIN_15 GpioAction::BUTTON_PRESS_DOWN   // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define GPIO_PIN_26 GpioAction::BUTTON_PRESS_UP     // UP     | UP     | UP      | UP       | UP     | UP     |
#define GPIO_PIN_27 GpioAction::BUTTON_PRESS_LEFT   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define GPIO_PIN_28 GpioAction::BUTTON_PRESS_L2     // L2     | LT     | ZL      | L2       | 7      | K4     |

// Setting GPIO pins to assigned by add-on
//
#define GPIO_PIN_00 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_12 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_13 GpioAction::ASSIGNED_TO_ADDON

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

#define TURBO_ENABLED 1
#define GPIO_PIN_29 GpioAction::BUTTON_PRESS_TURBO

#define BOARD_LEDS_PIN 0
#define LED_BRIGHTNESS_MAXIMUM 100
#define LED_BRIGHTNESS_STEPS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LEDS_PER_PIXEL 1
#define LEDS_BASE_ANIMATION_INDEX 1
#define LEDS_DPAD_UP     0
#define LEDS_BUTTON_L2   1
#define LEDS_DPAD_LEFT   2
#define LEDS_DPAD_DOWN   3
#define LEDS_DPAD_RIGHT  4
#define LEDS_BUTTON_L1   5
#define LEDS_BUTTON_R1   6
#define LEDS_BUTTON_B3   7
#define LEDS_BUTTON_B1   8
#define LEDS_BUTTON_B2   9
#define LEDS_BUTTON_R2   10
#define LEDS_BUTTON_B4   11

#define HAS_I2C_DISPLAY 1
#define I2C0_ENABLED 1
#define I2C0_PIN_SDA 12
#define I2C0_PIN_SCL 13
#define DISPLAY_FLIP 1
#define BUTTON_LAYOUT BUTTON_LAYOUT_BOARD_DEFINED_A
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_BOARD_DEFINED_B
#define SPLASH_MODE SPLASH_MODE_STATIC
#define SPLASH_DURATION 3000

#define DEFAULT_BOARD_LAYOUT_A {\
    {GP_ELEMENT_PIN_BUTTON, { 5, 17, 12, 33, 1, 1, 28,     GP_SHAPE_SQUARE, 24}},\
    {GP_ELEMENT_PIN_BUTTON, {19, 23, 26, 30, 1, 1, 27,     GP_SHAPE_SQUARE, 22}},\
    {GP_ELEMENT_PIN_BUTTON, {27, 27, 34, 34, 1, 1, 15,     GP_SHAPE_SQUARE, 22}},\
    {GP_ELEMENT_PIN_BUTTON, {31, 19, 38, 26, 1, 1, 26,     GP_SHAPE_SQUARE, 22}},\
    {GP_ELEMENT_PIN_BUTTON, {35, 31, 42, 38, 1, 1, 14,     GP_SHAPE_SQUARE, 22}},\
    {GP_ELEMENT_PIN_BUTTON, {42, 42, 49, 59, 1, 1, 11,     GP_SHAPE_SQUARE, 24}},\
    {GP_ELEMENT_PIN_BUTTON, {52, 27,  3,  3, 1, 1, 10,     GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {60, 27,  3,  3, 1, 1, 9,      GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {50, 18,  3,  3, 1, 1, 29,     GP_SHAPE_ELLIPSE}},\
}

#define DEFAULT_BOARD_LAYOUT_B {\
    {GP_ELEMENT_PIN_BUTTON, {122, 17, 115, 33, 1, 1, 1,    GP_SHAPE_SQUARE, 156}},\
    {GP_ELEMENT_PIN_BUTTON, {108, 23, 101, 30, 1, 1, 2,    GP_SHAPE_SQUARE, 158}},\
    {GP_ELEMENT_PIN_BUTTON, {100, 27,  93, 34, 1, 1, 4,    GP_SHAPE_SQUARE, 158}},\
    {GP_ELEMENT_PIN_BUTTON, { 96, 19,  89, 26, 1, 1, 3,    GP_SHAPE_SQUARE, 158}},\
    {GP_ELEMENT_PIN_BUTTON, {92, 31,  85, 38, 1, 1, 5,     GP_SHAPE_SQUARE, 158}},\
    {GP_ELEMENT_PIN_BUTTON, {85, 42,  78, 59, 1, 1, 6,     GP_SHAPE_SQUARE, 156}},\
    {GP_ELEMENT_PIN_BUTTON, {68, 27,  3,  3, 1, 1, 8,      GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {76, 27,  3,  3, 1, 1, 7,      GP_SHAPE_ELLIPSE}},\
}

#endif