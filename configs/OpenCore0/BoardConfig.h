/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "Open_Core0"

// Main pin mapping Configuration
//                          // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_UP     // UP     | UP     | UP      | UP       | UP     | UP     |
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_DOWN   // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_RIGHT  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_LEFT   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define GPIO_PIN_13 GpioAction::BUTTON_PRESS_B1     // B1     | A      | B       | Cross    | 2      | K1     |
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_B2     // B2     | B      | A       | Circle   | 3      | K2     |
#define GPIO_PIN_15 GpioAction::BUTTON_PRESS_R2     // R2     | RT     | ZR      | R2       | 8      | K3     |
#define GPIO_PIN_16 GpioAction::BUTTON_PRESS_L2     // L2     | LT     | ZL      | L2       | 7      | K4     |
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_B3     // B3     | X      | Y       | Square   | 1      | P1     |
#define GPIO_PIN_18 GpioAction::BUTTON_PRESS_B4     // B4     | Y      | X       | Triangle | 4      | P2     |
#define GPIO_PIN_19 GpioAction::BUTTON_PRESS_R1     // R1     | RB     | R       | R1       | 6      | P3     |
#define GPIO_PIN_20 GpioAction::BUTTON_PRESS_L1     // L1     | LB     | L       | L1       | 5      | P4     |
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_S1     // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_S2     // S2     | Start  | Plus    | Start    | 10     | Start  |
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_L3     // L3     | LS     | LS      | L3       | 11     | LS     |
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_R3     // R3     | RS     | RS      | R3       | 12     | RS     |
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_A1     // A1     | Guide  | Home    | PS       | 13     | ~      |
#define GPIO_PIN_04 GpioAction::BUTTON_PRESS_A2     // A2     | ~      | Capture | ~        | 14     | ~      |
#define GPIO_PIN_26 GpioAction::BUTTON_PRESS_FN     // FN
#define GPIO_PIN_21 GpioAction::SUSTAIN_FOCUS_MODE

// Setting GPIO pins to assigned by add-on
//
#define GPIO_PIN_00 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_01 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_08 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_28 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_29 GpioAction::ASSIGNED_TO_ADDON

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
#define GPIO_PIN_27 GpioAction::BUTTON_PRESS_TURBO

#define GPIO_PIN_25 GpioAction::SUSTAIN_DP_MODE_LS
#define GPIO_PIN_24 GpioAction::SUSTAIN_DP_MODE_RS

#define SLIDER_SOCD_ENABLED 1
#define GPIO_PIN_23 GpioAction::SUSTAIN_SOCD_MODE_UP_PRIO
#define GPIO_PIN_22 GpioAction::SUSTAIN_SOCD_MODE_SECOND_WIN
#define SLIDER_SOCD_SLOT_DEFAULT SOCD_MODE_NEUTRAL

#define USB_PERIPHERAL_ENABLED 1
#define USB_PERIPHERAL_PIN_DPLUS 28

#define DEFAULT_INPUT_MODE_R1 INPUT_MODE_XBONE
#define DEFAULT_INPUT_MODE_B4 INPUT_MODE_PS5
#define DEFAULT_PS5AUTHENTICATION_TYPE INPUT_MODE_AUTH_TYPE_USB

#define BOARD_LEDS_PIN 8

#define LED_BRIGHTNESS_MAXIMUM 50
#define LEDS_PER_PIXEL 2

#define LEDS_DPAD_LEFT   0
#define LEDS_DPAD_DOWN   1
#define LEDS_DPAD_RIGHT  2
#define LEDS_BUTTON_B3   3
#define LEDS_BUTTON_B4   4
#define LEDS_BUTTON_R1   5
#define LEDS_BUTTON_L1   6
#define LEDS_BUTTON_L2   7
#define LEDS_BUTTON_R2   8
#define LEDS_BUTTON_B2   9
#define LEDS_BUTTON_B1   10
#define LEDS_DPAD_UP     11

#define PLED_TYPE PLED_TYPE_RGB
#define PLED1_PIN 24
#define PLED2_PIN 25
#define PLED3_PIN 26
#define PLED4_PIN 27
#define PLED_COLOR ColorGreen

#define TURBO_LED_INDEX 28
#define TURBO_LED_TYPE PLED_TYPE_RGB
#define TURBO_LED_COLOR ColorRed

#define CASE_RGB_TYPE CASE_RGB_TYPE_AMBIENT
#define CASE_RGB_INDEX 29
#define CASE_RGB_COUNT 20
#define AMBIENT_LIGHT_EFFECT AL_CUSTOM_EFFECT_STATIC_COLOR
#define AMBIENT_STATIC_COLOR ANIMATION_COLOR_GREEN

#define HAS_I2C_DISPLAY 1
#define I2C0_ENABLED 1
#define I2C0_PIN_SDA 0
#define I2C0_PIN_SCL 1
#define DISPLAY_I2C_BLOCK i2c0
#define DISPLAY_FLIP 1

#define REVERSE_UP_DEFAULT 1
#define REVERSE_DOWN_DEFAULT 1
#define REVERSE_LEFT_DEFAULT 1
#define REVERSE_RIGHT_DEFAULT 1

#define BUTTON_LAYOUT BUTTON_LAYOUT_BOARD_DEFINED_A
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_BOARD_DEFINED_B
#define SPLASH_MODE SPLASH_MODE_STATIC
#define SPLASH_DURATION 3000

#define DEFAULT_BOARD_LAYOUT_A {\
    {GP_ELEMENT_PIN_BUTTON, {12, 25, 7, 7, 1, 1, 9,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {28, 25, 7, 7, 1, 1, 10,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {42, 32, 7, 7, 1, 1, 11,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {47, 55, 7, 7, 1, 1, 12,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {2, 12, 2, 2, 1, 1, 7,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {9, 12, 2, 2, 1, 1, 6,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {16, 12, 2, 2, 1, 1, 5,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {23, 12, 2, 2, 1, 1, 4,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {30, 12, 2, 2, 1, 1, 3,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {37, 12, 2, 2, 1, 1, 2,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {55, 12, 2, 2, 1, 1, 27,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {62, 12, 2, 2, 1, 1, 26,   GP_SHAPE_ELLIPSE}}\
  }
  
  #define DEFAULT_BOARD_LAYOUT_B {\
    {GP_ELEMENT_PIN_BUTTON, {56,  25, 7, 7, 1, 1, 17,  GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {72,  22, 7, 7, 1, 1, 18,  GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {88,  22, 7, 7, 1, 1, 19,  GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {104, 25, 7, 7, 1, 1, 20,  GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {56,  41, 7, 7, 1, 1, 13,  GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {72,  38, 7, 7, 1, 1, 14,  GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {88,  38, 7, 7, 1, 1, 15,  GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {104, 41, 7, 7, 1, 1, 16,  GP_SHAPE_ELLIPSE}}\
  }

#define FOCUS_MODE_ENABLED 1
#define FOCUS_MODE_BUTTON_MASK GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2 | GAMEPAD_MASK_A1 | GAMEPAD_MASK_A2 | GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3
#define FOCUS_MODE_BUTTON_LOCK_ENABLED 1


#endif
