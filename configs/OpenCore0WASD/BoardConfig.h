/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "Open_Core0 WASD"

// Main pin mapping Configuration
//                                                  // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_UP     // UP     | UP     | UP      | UP       | UP     | UP     |
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_DOWN   // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_RIGHT  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_LEFT   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_B1     // B1     | A      | B       | Cross    | 2      | K1     |
#define GPIO_PIN_15 GpioAction::BUTTON_PRESS_B2     // B2     | B      | A       | Circle   | 3      | K2     |
#define GPIO_PIN_16 GpioAction::BUTTON_PRESS_R2     // R2     | RT     | ZR      | R2       | 8      | K3     |
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_L2     // L2     | LT     | ZL      | L2       | 7      | K4     |
#define GPIO_PIN_18 GpioAction::BUTTON_PRESS_B3     // B3     | X      | Y       | Square   | 1      | P1     |
#define GPIO_PIN_19 GpioAction::BUTTON_PRESS_B4     // B4     | Y      | X       | Triangle | 4      | P2     |
#define GPIO_PIN_20 GpioAction::BUTTON_PRESS_R1     // R1     | RB     | R       | R1       | 6      | P3     |
#define GPIO_PIN_21 GpioAction::BUTTON_PRESS_L1     // L1     | LB     | L       | L1       | 5      | P4     |
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_S1     // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_S2     // S2     | Start  | Plus    | Start    | 10     | Start  |
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_L3     // L3     | LS     | LS      | L3       | 11     | LS     |
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_R3     // R3     | RS     | RS      | R3       | 12     | RS     |
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_A1     // A1     | Guide  | Home    | PS       | 13     | ~      |
#define GPIO_PIN_04 GpioAction::BUTTON_PRESS_A2     // A2     | ~      | Capture | ~        | 14     | ~      |
#define GPIO_PIN_13 GpioAction::BUTTON_PRESS_UP     // UP     | UP     | UP      | UP       | UP     | UP     |
#define GPIO_PIN_22 GpioAction::SUSTAIN_FOCUS_MODE

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


#define USB_PERIPHERAL_ENABLED 1
#define USB_PERIPHERAL_PIN_DPLUS 28

#define DEFAULT_INPUT_MODE_R1 INPUT_MODE_XBONE
#define DEFAULT_INPUT_MODE_B4 INPUT_MODE_PS5
#define DEFAULT_PS5AUTHENTICATION_TYPE INPUT_MODE_AUTH_TYPE_USB

#define TURBO_ENABLED 1
#define GPIO_PIN_27 GpioAction::BUTTON_PRESS_TURBO

#define GPIO_PIN_26 GpioAction::SUSTAIN_DP_MODE_LS
#define GPIO_PIN_25 GpioAction::SUSTAIN_DP_MODE_RS

#define SLIDER_SOCD_ENABLED 1
#define GPIO_PIN_24 GpioAction::SUSTAIN_SOCD_MODE_UP_PRIO
#define GPIO_PIN_23 GpioAction::SUSTAIN_SOCD_MODE_SECOND_WIN
#define SLIDER_SOCD_SLOT_DEFAULT SOCD_MODE_NEUTRAL

#define BOARD_LEDS_PIN 8
#define LED_BRIGHTNESS_MAXIMUM 50

#define PLED_TYPE PLED_TYPE_RGB
#define PLED_COLOR 6 // ColorGreen index from Animation.h
#define TURBO_LED_TYPE PLED_TYPE_RGB

#define LIGHT_DATA_NAME_DEFAULT "OpenCore0WASD"
#define LIGHT_DATA_SIZE_DEFAULT 38 //number of sets in the below data
#define LIGHT_DATA_DEFAULT \
0,  1, 4,  3, 10, LightType::LightType_ActionButton, \
1,  1, 3,  4, 9,  LightType::LightType_ActionButton, \
2,  1, 4,  4, 11, LightType::LightType_ActionButton, \
3,  1, 5,  4, 12, LightType::LightType_ActionButton, \
4,  1, 9,  3, 18, LightType::LightType_ActionButton, \
5,  1, 10, 3, 19, LightType::LightType_ActionButton, \
6,  1, 11, 3, 20, LightType::LightType_ActionButton, \
7,  1, 12, 3, 21, LightType::LightType_ActionButton, \
8,  1, 12, 4, 17, LightType::LightType_ActionButton, \
9,  1, 11, 4, 16, LightType::LightType_ActionButton, \
10, 1, 10, 4, 15, LightType::LightType_ActionButton, \
11, 1, 9,  4, 14, LightType::LightType_ActionButton, \
12, 1, 7,  6, 13, LightType::LightType_ActionButton, \
13, 1, 9,  0, 31, LightType::LightType_Player1Light, \
14, 1, 10, 0, 31, LightType::LightType_Player2Light, \
15, 1, 11, 0, 31, LightType::LightType_Player3Light, \
16, 1, 12, 0, 31, LightType::LightType_Player4Light, \
17, 1, 13, 0, 27, LightType::LightType_Turbo, \
18, 1, 15, 0, 0,  LightType::LightType_Case, \
19, 1, 15, 1, 1,  LightType::LightType_Case, \
20, 1, 15, 2, 2,  LightType::LightType_Case, \
21, 1, 15, 3, 3,  LightType::LightType_Case, \
22, 1, 15, 4, 4,  LightType::LightType_Case, \
23, 1, 15, 5, 5,  LightType::LightType_Case, \
24, 1, 15, 6, 6,  LightType::LightType_Case, \
25, 1, 15, 7, 7,  LightType::LightType_Case, \
26, 1, 15, 8, 8,  LightType::LightType_Case, \
27, 1, 15, 9, 9,  LightType::LightType_Case, \
28, 1, 0,  9, 10, LightType::LightType_Case, \
29, 1, 0,  8, 11, LightType::LightType_Case, \
30, 1, 0,  7, 12, LightType::LightType_Case, \
31, 1, 0,  6, 13, LightType::LightType_Case, \
32, 1, 0,  5, 14, LightType::LightType_Case, \
33, 1, 0,  4, 15, LightType::LightType_Case, \
34, 1, 0,  3, 16, LightType::LightType_Case, \
35, 1, 0,  2, 17, LightType::LightType_Case, \
36, 1, 0,  1, 18, LightType::LightType_Case, \
37, 1, 0,  0, 19, LightType::LightType_Case

// LED Profile 0
#define LEDS_PROFILE0_ENABLED 1
#define LEDS_PROFILE0_BASE_ANIMATION_INDEX AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_RAINBOW_SYNCED
#define LEDS_PROFILE0_PRESSED_ANIMATION_INDEX AnimationPressedEffects::AnimationPressedEffects_PRESSEDEFFECT_STATIC_COLOR
#define LEDS_PROFILE0_CASE_ANIMATION_INDEX AnimationNonPressedEffects::AnimationNonPressedEffects_EFFECT_STATIC_COLOR
#define LEDS_PROFILE0_STATIC_COLOR_UNPRESSED ColorIndexRed // only drawn on the turbo light, base anim is rainbow
#define LEDS_PROFILE0_STATIC_COLOR_PRESSED ColorIndexWhite
#define LEDS_PROFILE0_STATIC_COLOR_CASE ColorIndexGreen
#define LEDS_PROFILE0_USE_CASE_IN_PRESSED true

#define HAS_I2C_DISPLAY 1
#define I2C0_ENABLED 1
#define I2C0_PIN_SDA 0
#define I2C0_PIN_SCL 1
#define DISPLAY_I2C_BLOCK i2c0
#define DISPLAY_FLIP 1

#define BUTTON_LAYOUT BUTTON_LAYOUT_BOARD_DEFINED_A
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_BOARD_DEFINED_B
#define SPLASH_MODE SPLASH_MODE_STATIC
#define SPLASH_DURATION 3000

#define DEFAULT_BOARD_LAYOUT_A_NAME "Open_Core0 WASD Default"
#define DEFAULT_BOARD_LAYOUT_A {\
    {GP_ELEMENT_PIN_BUTTON, {19, 30, 26, 37, 1, 1, 9, GP_SHAPE_SQUARE,22}},\
    {GP_ELEMENT_PIN_BUTTON, {27, 34, 34, 41, 1, 1, 11, GP_SHAPE_SQUARE,22}},\
    {GP_ELEMENT_PIN_BUTTON, {31, 26, 38, 33, 1, 1, 10, GP_SHAPE_SQUARE,22}},\
    {GP_ELEMENT_PIN_BUTTON, {35, 38, 42, 45, 1, 1, 12, GP_SHAPE_SQUARE,22}},\
    {GP_ELEMENT_PIN_BUTTON, {55, 45, 75, 53, 1, 1, 13, GP_SHAPE_SQUARE}},\
    {GP_ELEMENT_PIN_BUTTON, {6, 19, 3, 3, 1, 1, 7, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {14, 19, 3, 3, 1, 1, 6, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {23, 19, 3, 3, 1, 1, 5, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {31, 19, 3, 3, 1, 1, 4, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {39, 19, 3, 3, 1, 1, 3, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {47, 19, 3, 3, 1, 1, 2, GP_SHAPE_ELLIPSE}},\
}

#define DEFAULT_BOARD_LAYOUT_B_NAME "Open_Core0 WASD Default"
#define DEFAULT_BOARD_LAYOUT_B {\
    {GP_ELEMENT_PIN_BUTTON, {81, 19, 3, 3, 1, 1, 27, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {81, 30, 88, 37, 1, 1, 18, GP_SHAPE_SQUARE,68}},\
    {GP_ELEMENT_PIN_BUTTON, {88, 24, 95, 31, 1, 1, 19, GP_SHAPE_SQUARE,68}},\
    {GP_ELEMENT_PIN_BUTTON, {96, 20, 103, 27, 1, 1, 20, GP_SHAPE_SQUARE,68}},\
    {GP_ELEMENT_PIN_BUTTON, {105, 18, 112, 25, 1, 1, 21, GP_SHAPE_SQUARE,68}},\
    {GP_ELEMENT_PIN_BUTTON, {85, 38, 92, 45, 1, 1, 14, GP_SHAPE_SQUARE,68}},\
    {GP_ELEMENT_PIN_BUTTON, {92, 32, 99, 39, 1, 1, 15, GP_SHAPE_SQUARE,68}},\
    {GP_ELEMENT_PIN_BUTTON, {100, 28, 107, 35, 1, 1, 16, GP_SHAPE_SQUARE,68}},\
    {GP_ELEMENT_PIN_BUTTON, {109, 26, 116, 33, 1, 1, 17, GP_SHAPE_SQUARE,68}},\
}

#define FOCUS_MODE_ENABLED 1
#define FOCUS_MODE_BUTTON_MASK GAMEPAD_MASK_S1 | GAMEPAD_MASK_S2 | GAMEPAD_MASK_A1 | GAMEPAD_MASK_A2 | GAMEPAD_MASK_L3 | GAMEPAD_MASK_R3
#define FOCUS_MODE_BUTTON_LOCK_ENABLED 1

#endif