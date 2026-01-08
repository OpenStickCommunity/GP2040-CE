/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "OSUMGP-RP2040"

// Main pin mapping configuration
//                                                  // GP2040 | Xinput | Switch  | PS3/4/5  | Dinput | Arcade |
#define GPIO_PIN_17 GpioAction::BUTTON_PRESS_UP     // UP     | UP     | UP      | UP       | UP     | UP     |
#define GPIO_PIN_13 GpioAction::BUTTON_PRESS_DOWN   // DOWN   | DOWN   | DOWN    | DOWN     | DOWN   | DOWN   |
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_RIGHT  // RIGHT  | RIGHT  | RIGHT   | RIGHT    | RIGHT  | RIGHT  |
#define GPIO_PIN_16 GpioAction::BUTTON_PRESS_LEFT   // LEFT   | LEFT   | LEFT    | LEFT     | LEFT   | LEFT   |
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_B1     // B1     | A      | B       | Cross    | 2      | K1     |
#define GPIO_PIN_08 GpioAction::BUTTON_PRESS_B2     // B2     | B      | A       | Circle   | 3      | K2     |
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_R2     // R2     | RT     | ZR      | R2       | 8      | K3     |
#define GPIO_PIN_18 GpioAction::BUTTON_PRESS_L2     // L2     | LT     | ZL      | L2       | 7      | K4     |
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_B3     // B3     | X      | Y       | Square   | 1      | P1     |
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_B4     // B4     | Y      | X       | Triangle | 4      | P2     |
#define GPIO_PIN_03 GpioAction::BUTTON_PRESS_R1     // R1     | RB     | R       | R1       | 6      | P3     |
#define GPIO_PIN_21 GpioAction::BUTTON_PRESS_L1     // L1     | LB     | L       | L1       | 5      | P4     |
#define GPIO_PIN_22 GpioAction::BUTTON_PRESS_S1     // S1     | Back   | Minus   | Select   | 9      | Coin   |
#define GPIO_PIN_02 GpioAction::BUTTON_PRESS_S2     // S2     | Start  | Plus    | Start    | 10     | Start  |
#define GPIO_PIN_19 GpioAction::BUTTON_PRESS_L3     // L3     | LS     | LS      | L3       | 11     | LS     |
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_R3     // R3     | RS     | RS      | R3       | 12     | RS     |
#define GPIO_PIN_15 GpioAction::BUTTON_PRESS_A1     // A1     | Guide  | Home    | PS       | 13     | ~      |
#define GPIO_PIN_20 GpioAction::BUTTON_PRESS_A2     // A2     | ~      | Capture | ~        | 14     | ~      |


// Setting GPIO pins to assigned by add-on
//
#define GPIO_PIN_00 GpioAction::ASSIGNED_TO_ADDON // I2C SDA
#define GPIO_PIN_01 GpioAction::ASSIGNED_TO_ADDON // I2C SCL
#define GPIO_PIN_11 GpioAction::ASSIGNED_TO_ADDON // USB D+
#define GPIO_PIN_12 GpioAction::ASSIGNED_TO_ADDON // USB D-
#define GPIO_PIN_25 GpioAction::ASSIGNED_TO_ADDON // On-board LED
#define GPIO_PIN_26 GpioAction::ASSIGNED_TO_ADDON // ADC1 X
#define GPIO_PIN_27 GpioAction::ASSIGNED_TO_ADDON // ADC2 X
#define GPIO_PIN_28 GpioAction::ASSIGNED_TO_ADDON // ADC1 Y
#define GPIO_PIN_29 GpioAction::ASSIGNED_TO_ADDON // ADC2 Y


// Keyboard mapping configuration
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


// J4 - I2C
// SparkFun Micro OLED Breakout recommended (LCD-22495)
#define HAS_I2C_DISPLAY 1
#define I2C0_ENABLED 1
#define DISPLAY_I2C_ADDR 0x3D
#define I2C0_PIN_SDA 0
#define I2C0_PIN_SCL 1


// J3 - USB Passthrough
#define USB_PERIPHERAL_ENABLED 1
#define USB_PERIPHERAL_PIN_DPLUS 11
#define USB_PERIPHERAL_PIN_ORDER 0


// On-board LED settings
#define BOARD_LED_ENABLED 1
#define BOARD_LED_TYPE ON_BOARD_LED_MODE_MODE_INDICATOR


// Analog settings
#define ANALOG_INPUT_ENABLED 1
#define ANALOG_ADC_1_VRX 26
#define ANALOG_ADC_1_VRY 28
#define ANALOG_ADC_2_VRX 27
#define ANALOG_ADC_2_VRY 29


// Display settings for recommended SparkFun Micro OLED Breakout (LCD-22495)
#define DISPLAY_FLIP 3
#define DISPLAY_INPUT_MODE 0
#define DISPLAY_TURBO_MODE 0
#define DISPLAY_DPAD_MODE 0
#define DISPLAY_SOCD_MODE 0
#define DISPLAY_MACRO_MODE 0
#define DISPLAY_PROFILE_MODE 0
#define BUTTON_LAYOUT BUTTON_LAYOUT_BOARD_DEFINED_A
#define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_BOARD_DEFINED_B

#define DEFAULT_BOARD_LAYOUT_A {\
    {GP_ELEMENT_PIN_BUTTON, {55, 34, 2, 2, 1, 1, 17, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {55, 40, 2, 2, 1, 1, 13, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {52, 37, 2, 2, 1, 1, 16, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {58, 37, 2, 2, 1, 1, 14, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {65, 39, 2, 2, 1, 1, 15, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {58, 23, 2, 2, 1, 1, 22, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {71, 23, 2, 2, 1, 1, 2, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {57, 16, 50, 13, 1, 1, 21, GP_SHAPE_SQUARE}},\
    {GP_ELEMENT_PIN_BUTTON, {48, 14, 44, 6, 1, 1, 18, GP_SHAPE_SQUARE}},\
    {GP_ELEMENT_LEVER, {45, 28, 4, 4, 1, 0, GP_LEVER_MODE_LEFT_ANALOG}}\
}

#define DEFAULT_BOARD_LAYOUT_B {\
    {GP_ELEMENT_PIN_BUTTON, {58, 27, 2, 2, 1, 1, 20, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {71, 27, 2, 2, 1, 1, 4, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {80, 30, 2, 2, 1, 1, 6, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {84, 26, 2, 2, 1, 1, 5, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {88, 30, 2, 2, 1, 1, 8, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {84, 34, 2, 2, 1, 1, 9, GP_SHAPE_ELLIPSE}},\
    {GP_ELEMENT_PIN_BUTTON, {82, 16, 75, 13, 1, 1, 3, GP_SHAPE_SQUARE}},\
    {GP_ELEMENT_PIN_BUTTON, {88, 14, 84, 6, 1, 1, 7, GP_SHAPE_SQUARE}},\
    {GP_ELEMENT_LEVER, {75, 39, 4, 4, 1, 0, GP_LEVER_MODE_RIGHT_ANALOG}}\
}

#endif