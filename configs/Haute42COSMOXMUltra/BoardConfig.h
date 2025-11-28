/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "Haute42 COSMOX M Ultra"

// Main pin mapping Configuration
//                          // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_UP     // UP     | UP     | UP      | UP       | UP     | UP     |
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_DOWN   // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define GPIO_PIN_04 GpioAction::BUTTON_PRESS_RIGHT  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_LEFT   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_B1     // B1     | A      | B       | Cross    | 2      | K1     |
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_B2     // B2     | B      | A       | Circle   | 3      | K2     |
#define GPIO_PIN_08 GpioAction::BUTTON_PRESS_R2     // R2     | RT     | ZR      | R2       | 8      | K3     |
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_L2     // L2     | LT     | ZL      | L2       | 7      | K4     |
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_B3     // B3     | X      | Y       | Square   | 1      | P1     |
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_B4     // B4     | Y      | X       | Triangle | 4      | P2     |
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_R1     // R1     | RB     | R       | R1       | 6      | P3     |
#define GPIO_PIN_13 GpioAction::BUTTON_PRESS_L1     // L1     | LB     | L       | L1       | 5      | P4     |
#define GPIO_PIN_16 GpioAction::BUTTON_PRESS_S1     // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_S2     // S2     | Start  | Plus    | Start    | 10     | Start  |
#define GPIO_PIN_18 GpioAction::BUTTON_PRESS_L3     // L3     | LS     | LS      | L3       | 11     | LS     |
#define GPIO_PIN_19 GpioAction::BUTTON_PRESS_R3     // R3     | RS     | RS      | R3       | 12     | RS     |
#define GPIO_PIN_20 GpioAction::BUTTON_PRESS_A1     // A1     | Guide  | Home    | PS       | 13     | ~      |
#define GPIO_PIN_21 GpioAction::BUTTON_PRESS_A2     // A2     | ~      | Capture | ~        | 14     | ~      |

// Setting GPIO pins to assigned by add-on
//
#define GPIO_PIN_00 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_01 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_23 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_24 GpioAction::ASSIGNED_TO_ADDON
#define GPIO_PIN_28 GpioAction::ASSIGNED_TO_ADDON

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

#define USB_PERIPHERAL_PIN_DPLUS 23
#define USB_PERIPHERAL_PIN_ORDER 0

#define DEFAULT_INPUT_MODE_R1 INPUT_MODE_XBONE
#define DEFAULT_INPUT_MODE_B4 INPUT_MODE_PS5
#define DEFAULT_PS5AUTHENTICATION_TYPE INPUT_MODE_AUTH_TYPE_USB

#define TURBO_ENABLED 1
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_TURBO

#define BOARD_LEDS_PIN 28
#define LED_BRIGHTNESS_MAXIMUM 200
#define LEDS_BRIGHTNESS 5
#define LED_FORMAT LED_FORMAT_GRB
#define LEDS_PER_PIXEL 1
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

// Set our default ambient light
#define CASE_RGB_TYPE CASE_RGB_TYPE_AMBIENT
#define CASE_RGB_INDEX 16
#define CASE_RGB_COUNT 30
#define AMBIENT_LIGHT_EFFECT AL_CUSTOM_EFFECT_GRADIENT
#define AMBIENT_STATIC_COLOR ANIMATION_COLOR_PURPLE

#define HAS_I2C_DISPLAY 1
#define I2C0_ENABLED 1
#define I2C0_PIN_SDA 0
#define I2C0_PIN_SCL 1
#define BUTTON_LAYOUT BUTTON_LAYOUT_BOARD_DEFINED_A
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_BOARD_DEFINED_B
#define SPLASH_MODE SPLASH_MODE_STATIC
#define SPLASH_DURATION 3000

#define DEFAULT_BOARD_LAYOUT_A {\
    {GP_ELEMENT_PIN_BUTTON, {47,  19, 4, 4, 1, 1, 27,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {32,  27, 4, 4, 1, 1, 5,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {42,  27, 4, 4, 1, 1, 3,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {50,  32, 4, 4, 1, 1, 4,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {64,  17, 4, 4, 1, 1, 18,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {54,  49, 5, 5, 1, 1, 2,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {42,  48, 4, 4, 1, 1, 26,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {77,  15, 2, 2, 1, 1, 14,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {84,  15, 2, 2, 1, 1, 21,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {91,  15, 2, 2, 1, 1, 20,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {98,  15, 2, 2, 1, 1, 16,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {105, 15, 2, 2, 1, 1, 17,   GP_SHAPE_ELLIPSE}}\
}

#define DEFAULT_BOARD_LAYOUT_B {\
    {GP_ELEMENT_PIN_BUTTON, {59, 27, 4, 4, 1, 1, 10,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {69, 25, 4, 4, 1, 1, 11,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {79, 25, 4, 4, 1, 1, 12,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {89, 27, 4, 4, 1, 1, 13,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {59, 37, 4, 4, 1, 1, 6,     GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {69, 35, 4, 4, 1, 1, 7,     GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {79, 35, 4, 4, 1, 1, 8,     GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {89, 37, 4, 4, 1, 1, 9,     GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {66, 48, 4, 4, 1, 1, 19,    GP_SHAPE_ELLIPSE}}\
}

// C16
#define DEFAULT_BOARD_LAYOUT_A_ALT0 {\
    {GP_ELEMENT_PIN_BUTTON, {47,  19, 4, 4, 1, 1, 27,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {32,  27, 4, 4, 1, 1, 5,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {42,  27, 4, 4, 1, 1, 3,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {50,  32, 4, 4, 1, 1, 4,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {64,  17, 4, 4, 1, 1, 26,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {54,  49, 5, 5, 1, 1, 2,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {42,  48, 4, 4, 1, 1, 18,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {77,  15, 2, 2, 1, 1, 14,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {84,  15, 2, 2, 1, 1, 21,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {91,  15, 2, 2, 1, 1, 20,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {98,  15, 2, 2, 1, 1, 16,   GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {105, 15, 2, 2, 1, 1, 17,   GP_SHAPE_ELLIPSE}}\
}

#define DEFAULT_BOARD_LAYOUT_B_ALT0 {\
    {GP_ELEMENT_PIN_BUTTON, {59, 27, 4, 4, 1, 1, 10,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {69, 25, 4, 4, 1, 1, 11,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {79, 25, 4, 4, 1, 1, 12,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {89, 27, 4, 4, 1, 1, 13,    GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {59, 37, 4, 4, 1, 1, 6,     GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {69, 35, 4, 4, 1, 1, 7,     GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {79, 35, 4, 4, 1, 1, 8,     GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {89, 37, 4, 4, 1, 1, 9,     GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {66, 48, 4, 4, 1, 1, 19,    GP_SHAPE_ELLIPSE}}\
}

// C16-S
#define DEFAULT_BOARD_LAYOUT_A_ALT1 {\
    {GP_ELEMENT_PIN_BUTTON, {19, 30, 20, 24, 1, 1, 18, GP_SHAPE_PILL,120}},\
    {GP_ELEMENT_PIN_BUTTON, {32, 25, 4, 4, 1, 1, 5, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {43, 27, 4, 4, 1, 1, 3, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {53, 31, 4, 4, 1, 1, 4, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {46, 44, 4, 4, 1, 1, 13, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {55, 53, 56, 47, 1, 1, 2, GP_SHAPE_PILL,120}},\
}

#define DEFAULT_BOARD_LAYOUT_B_ALT1 {\
    {GP_ELEMENT_PIN_BUTTON, {74, 25, 4, 4, 1, 1, 10, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {83, 20, 4, 4, 1, 1, 11, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {93, 18, 4, 4, 1, 1, 12, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {74, 36, 4, 4, 1, 1, 6, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {83, 31, 4, 4, 1, 1, 7, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {93, 29, 4, 4, 1, 1, 8, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {81, 44, 4, 4, 1, 1, 9, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {103, 30, 104, 24, 1, 1, 19, GP_SHAPE_PILL,60}},\
    {GP_ELEMENT_PIN_BUTTON, {70, 53, 71, 47, 1, 1, 27, GP_SHAPE_PILL,60}},\
    {GP_ELEMENT_PIN_BUTTON, {64, 30, 4, 4, 1, 1, 26, GP_SHAPE_ELLIPSE}},\
}

// Additional Button Support
#define GPIO_PIN_27 GpioAction::BUTTON_PRESS_UP
#define GPIO_PIN_26 GpioAction::BUTTON_PRESS_L3

// Keyboard Host enabled by default
#define KEYBOARD_HOST_ENABLED 1

#define BOARD_LED_ENABLED 1
#define BOARD_LED_TYPE ON_BOARD_LED_MODE_MODE_INDICATOR

#define LIGHT_DATA_NAME_DEFAULT "Haute42|Cosmox M Ultra" 
#define LIGHT_DATA_SIZE_DEFAULT 46 //number of sets in the below data
#define LIGHT_DATA_DEFAULT \
0,  1,  3,  5,  5, LightType::LightType_ActionButton, \
1,  1,  5,  5,  3, LightType::LightType_ActionButton, \
2,  1,  7,  6,  4, LightType::LightType_ActionButton, \
3,  1,  8,  9,  2, LightType::LightType_ActionButton, \
4,  1,  9,  5, 10, LightType::LightType_ActionButton, \
5,  1, 11,  4, 11, LightType::LightType_ActionButton, \
6,  1, 13,  4, 12, LightType::LightType_ActionButton, \
7,  1, 15,  4, 13, LightType::LightType_ActionButton, \
8,  1,  9,  7,  6, LightType::LightType_ActionButton, \
9,  1, 11,  6,  7, LightType::LightType_ActionButton, \
10, 1, 13,  6,  8, LightType::LightType_ActionButton, \
11, 1, 15,  6,  9, LightType::LightType_ActionButton, \
12, 1,  6,  3, 27, LightType::LightType_ActionButton, \
13, 1,  9,  3, 18, LightType::LightType_ActionButton, \
14, 1, 10,  9, 19, LightType::LightType_ActionButton, \
15, 1,  6,  9, 26, LightType::LightType_ActionButton, \
16, 1,  8,  0,  0, LightType::LightType_Case, \
17, 1,  6,  0,  1, LightType::LightType_Case, \
18, 1,  4,  0,  2, LightType::LightType_Case, \
19, 1,  2,  0,  3, LightType::LightType_Case, \
20, 1,  0,  0,  4, LightType::LightType_Case, \
21, 1,  0,  2,  5, LightType::LightType_Case, \
22, 1,  0,  4,  6, LightType::LightType_Case, \
23, 1,  0,  6,  7, LightType::LightType_Case, \
24, 1,  0,  8,  8, LightType::LightType_Case, \
25, 1,  0, 10,  9, LightType::LightType_Case, \
26, 1,  0, 12, 10, LightType::LightType_Case, \
27, 1,  2, 12, 11, LightType::LightType_Case, \
28, 1,  4, 12, 12, LightType::LightType_Case, \
29, 1,  6, 12, 13, LightType::LightType_Case, \
30, 1,  8, 12, 14, LightType::LightType_Case, \
31, 1, 10, 12, 15, LightType::LightType_Case, \
32, 1, 12, 12, 16, LightType::LightType_Case, \
33, 1, 14, 12, 17, LightType::LightType_Case, \
34, 1, 16, 12, 18, LightType::LightType_Case, \
35, 1, 18, 12, 19, LightType::LightType_Case, \
36, 1, 18, 10, 20, LightType::LightType_Case, \
37, 1, 18,  8, 21, LightType::LightType_Case, \
38, 1, 18,  6, 22, LightType::LightType_Case, \
39, 1, 18,  4, 23, LightType::LightType_Case, \
40, 1, 18,  2, 24, LightType::LightType_Case, \
41, 1, 18,  0, 25, LightType::LightType_Case, \
42, 1, 16,  0, 26, LightType::LightType_Case, \
43, 1, 14,  0, 27, LightType::LightType_Case, \
44, 1, 12,  0, 28, LightType::LightType_Case, \
45, 1, 10,  0, 29, LightType::LightType_Case

#define LIGHT_DATA_NAME_1 "Haute42|Cosmox C16" 
#define LIGHT_DATA_SIZE_1 46 //number of sets in the below data
#define LIGHT_DATA_1 \
0,  1,  3,  5,  5, LightType::LightType_ActionButton, \
1,  1,  5,  5,  3, LightType::LightType_ActionButton, \
2,  1,  7,  6,  4, LightType::LightType_ActionButton, \
3,  1,  8,  9,  2, LightType::LightType_ActionButton, \
4,  1,  9,  5, 10, LightType::LightType_ActionButton, \
5,  1, 11,  4, 11, LightType::LightType_ActionButton, \
6,  1, 13,  4, 12, LightType::LightType_ActionButton, \
7,  1, 15,  4, 13, LightType::LightType_ActionButton, \
8,  1,  9,  7,  6, LightType::LightType_ActionButton, \
9,  1, 11,  6,  7, LightType::LightType_ActionButton, \
10, 1, 13,  6,  8, LightType::LightType_ActionButton, \
11, 1, 15,  6,  9, LightType::LightType_ActionButton, \
12, 1,  6,  3, 27, LightType::LightType_ActionButton, \
13, 1,  6,  9, 18, LightType::LightType_ActionButton, \
14, 1, 10,  9, 19, LightType::LightType_ActionButton, \
15, 1,  9,  3, 26, LightType::LightType_ActionButton, \
16, 1,  8,  0,  0, LightType::LightType_Case, \
17, 1,  6,  0,  1, LightType::LightType_Case, \
18, 1,  4,  0,  2, LightType::LightType_Case, \
19, 1,  2,  0,  3, LightType::LightType_Case, \
20, 1,  0,  0,  4, LightType::LightType_Case, \
21, 1,  0,  2,  5, LightType::LightType_Case, \
22, 1,  0,  4,  6, LightType::LightType_Case, \
23, 1,  0,  6,  7, LightType::LightType_Case, \
24, 1,  0,  8,  8, LightType::LightType_Case, \
25, 1,  0, 10,  9, LightType::LightType_Case, \
26, 1,  0, 12, 10, LightType::LightType_Case, \
27, 1,  2, 12, 11, LightType::LightType_Case, \
28, 1,  4, 12, 12, LightType::LightType_Case, \
29, 1,  6, 12, 13, LightType::LightType_Case, \
30, 1,  8, 12, 14, LightType::LightType_Case, \
31, 1, 10, 12, 15, LightType::LightType_Case, \
32, 1, 12, 12, 16, LightType::LightType_Case, \
33, 1, 14, 12, 17, LightType::LightType_Case, \
34, 1, 16, 12, 18, LightType::LightType_Case, \
35, 1, 18, 12, 19, LightType::LightType_Case, \
36, 1, 18, 10, 20, LightType::LightType_Case, \
37, 1, 18,  8, 21, LightType::LightType_Case, \
38, 1, 18,  6, 22, LightType::LightType_Case, \
39, 1, 18,  4, 23, LightType::LightType_Case, \
40, 1, 18,  2, 24, LightType::LightType_Case, \
41, 1, 18,  0, 25, LightType::LightType_Case, \
42, 1, 16,  0, 26, LightType::LightType_Case, \
43, 1, 14,  0, 27, LightType::LightType_Case, \
44, 1, 12,  0, 28, LightType::LightType_Case, \
45, 1, 10,  0, 29, LightType::LightType_Case

#define LIGHT_DATA_NAME_2 "Haute42|Cosmox C16-S" 
#define LIGHT_DATA_SIZE_2 46 //number of sets in the below data
#define LIGHT_DATA_2 \
0,  1,  3,  4,  5, LightType::LightType_ActionButton, \
1,  1,  5,  4,  3, LightType::LightType_ActionButton, \
2,  1,  7,  5,  4, LightType::LightType_ActionButton, \
3,  1,  8,  9,  2, LightType::LightType_ActionButton, \
4,  1, 11,  4, 10, LightType::LightType_ActionButton, \
5,  1, 13,  3, 11, LightType::LightType_ActionButton, \
6,  1, 15,  3, 12, LightType::LightType_ActionButton, \
7,  1,  6,  8, 13, LightType::LightType_ActionButton, \
8,  1, 11,  6,  6, LightType::LightType_ActionButton, \
9,  1, 13,  5,  7, LightType::LightType_ActionButton, \
10, 1, 15,  5,  8, LightType::LightType_ActionButton, \
11, 1, 12,  8,  9, LightType::LightType_ActionButton, \
12, 1, 10,  9, 27, LightType::LightType_ActionButton, \
13, 1,  1,  4, 18, LightType::LightType_ActionButton, \
14, 1, 17,  4, 19, LightType::LightType_ActionButton, \
15, 1,  9,  5, 26, LightType::LightType_ActionButton, \
16, 1,  8,  0,  0, LightType::LightType_Case, \
17, 1,  6,  0,  1, LightType::LightType_Case, \
18, 1,  4,  0,  2, LightType::LightType_Case, \
19, 1,  2,  0,  3, LightType::LightType_Case, \
20, 1,  0,  0,  4, LightType::LightType_Case, \
21, 1,  0,  2,  5, LightType::LightType_Case, \
22, 1,  0,  4,  6, LightType::LightType_Case, \
23, 1,  0,  6,  7, LightType::LightType_Case, \
24, 1,  0,  8,  8, LightType::LightType_Case, \
25, 1,  0, 10,  9, LightType::LightType_Case, \
26, 1,  0, 12, 10, LightType::LightType_Case, \
27, 1,  2, 12, 11, LightType::LightType_Case, \
28, 1,  4, 12, 12, LightType::LightType_Case, \
29, 1,  6, 12, 13, LightType::LightType_Case, \
30, 1,  8, 12, 14, LightType::LightType_Case, \
31, 1, 10, 12, 15, LightType::LightType_Case, \
32, 1, 12, 12, 16, LightType::LightType_Case, \
33, 1, 14, 12, 17, LightType::LightType_Case, \
34, 1, 16, 12, 18, LightType::LightType_Case, \
35, 1, 18, 12, 19, LightType::LightType_Case, \
36, 1, 18, 10, 20, LightType::LightType_Case, \
37, 1, 18,  8, 21, LightType::LightType_Case, \
38, 1, 18,  6, 22, LightType::LightType_Case, \
39, 1, 18,  4, 23, LightType::LightType_Case, \
40, 1, 18,  2, 24, LightType::LightType_Case, \
41, 1, 18,  0, 25, LightType::LightType_Case, \
42, 1, 16,  0, 26, LightType::LightType_Case, \
43, 1, 14,  0, 27, LightType::LightType_Case, \
44, 1, 12,  0, 28, LightType::LightType_Case, \
45, 1, 10,  0, 29, LightType::LightType_Case

#define LIGHT_DATA_NAME_3 "Haute42|Cosmox M Ultra Gen 2" 
#define LIGHT_DATA_SIZE_3 46 //number of sets in the below data
#define LIGHT_DATA_3 \
0,  1,  4,  4,  5, LightType::LightType_ActionButton, \
1,  1,  6,  4,  3, LightType::LightType_ActionButton, \
2,  1,  8,  5,  4, LightType::LightType_ActionButton, \
3,  1,  9,  9,  2, LightType::LightType_ActionButton, \
4,  1, 10,  4, 10, LightType::LightType_ActionButton, \
5,  1, 12,  3, 11, LightType::LightType_ActionButton, \
6,  1, 14,  3, 12, LightType::LightType_ActionButton, \
7,  1, 16,  4, 13, LightType::LightType_ActionButton, \
8,  1, 10,  6,  6, LightType::LightType_ActionButton, \
9,  1, 12,  5,  7, LightType::LightType_ActionButton, \
10, 1, 14,  5,  8, LightType::LightType_ActionButton, \
11, 1, 16,  6,  9, LightType::LightType_ActionButton, \
12, 1,  7,  2, 27, LightType::LightType_ActionButton, \
13, 1,  7,  8, 18, LightType::LightType_ActionButton, \
14, 1, 11,  8, 19, LightType::LightType_ActionButton, \
15, 1,  2,  5, 26, LightType::LightType_ActionButton, \
16, 1,  8,  0,  0, LightType::LightType_Case, \
17, 1,  6,  0,  1, LightType::LightType_Case, \
18, 1,  4,  0,  2, LightType::LightType_Case, \
19, 1,  2,  0,  3, LightType::LightType_Case, \
20, 1,  0,  0,  4, LightType::LightType_Case, \
21, 1,  0,  2,  5, LightType::LightType_Case, \
22, 1,  0,  4,  6, LightType::LightType_Case, \
23, 1,  0,  6,  7, LightType::LightType_Case, \
24, 1,  0,  8,  8, LightType::LightType_Case, \
25, 1,  0, 10,  9, LightType::LightType_Case, \
26, 1,  0, 12, 10, LightType::LightType_Case, \
27, 1,  2, 12, 11, LightType::LightType_Case, \
28, 1,  4, 12, 12, LightType::LightType_Case, \
29, 1,  6, 12, 13, LightType::LightType_Case, \
30, 1,  8, 12, 14, LightType::LightType_Case, \
31, 1, 10, 12, 15, LightType::LightType_Case, \
32, 1, 12, 12, 16, LightType::LightType_Case, \
33, 1, 14, 12, 17, LightType::LightType_Case, \
34, 1, 16, 12, 18, LightType::LightType_Case, \
35, 1, 18, 12, 19, LightType::LightType_Case, \
36, 1, 18, 10, 20, LightType::LightType_Case, \
37, 1, 18,  8, 21, LightType::LightType_Case, \
38, 1, 18,  6, 22, LightType::LightType_Case, \
39, 1, 18,  4, 23, LightType::LightType_Case, \
40, 1, 18,  2, 24, LightType::LightType_Case, \
41, 1, 18,  0, 25, LightType::LightType_Case, \
42, 1, 16,  0, 26, LightType::LightType_Case, \
43, 1, 14,  0, 27, LightType::LightType_Case, \
44, 1, 12,  0, 28, LightType::LightType_Case, \
45, 1, 10,  0, 29, LightType::LightType_Case

#define DEFAULT_SPLASH \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xfe, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xfe, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc0, 0xff, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xfe, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xff, 0x83, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xfe, 0x7f, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7e, 0x7f, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x7f, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0e, 0x7f, 0xff, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00

#endif