/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

 #ifndef PICO_BOARD_CONFIG_H_
 #define PICO_BOARD_CONFIG_H_
 
 #include "enums.pb.h"
 #include "class/hid/hid.h"
 
 #define BOARD_CONFIG_LABEL "Haute42 COSMOX"
 
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
 #define GPIO_PIN_27 GpioAction::BUTTON_PRESS_UP     // UP     | UP     | UP      | UP       | UP     | UP     |
 #define GPIO_PIN_26 GpioAction::BUTTON_PRESS_L3     // L3     | LS     | LS      | L3       | 11     | LS     |
 
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
 #define LED_BRIGHTNESS_MAXIMUM 100
 #define LED_BRIGHTNESS_STEPS 5
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
 
 #define HAS_I2C_DISPLAY 1
 #define I2C0_ENABLED 1
 #define I2C0_PIN_SDA 0
 #define I2C0_PIN_SCL 1
 #define SPLASH_MODE SPLASH_MODE_STATIC
 #define SPLASH_DURATION 3000
 
 #define BUTTON_LAYOUT BUTTON_LAYOUT_BOARD_DEFINED_A
 #define BUTTON_LAYOUT_RIGHT BUTTON_LAYOUT_BOARD_DEFINED_B
 
 // Stickless
 #define DEFAULT_BOARD_LAYOUT_A {\
     {GP_ELEMENT_PIN_BUTTON, {8,  20, 8, 8, 1, 1, 5,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {26, 20, 8, 8, 1, 1, 3,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {41, 29, 8, 8, 1, 1, 4,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {48, 53, 8, 8, 1, 1, 2,     GP_SHAPE_ELLIPSE}}\
 }
 
 #define DEFAULT_BOARD_LAYOUT_B {\
     {GP_ELEMENT_PIN_BUTTON, {57,  20, 8, 8, 1, 1, 10,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {75,  16, 8, 8, 1, 1, 11,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {93,  16, 8, 8, 1, 1, 12,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {111, 20, 8, 8, 1, 1, 13,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {57,  38, 8, 8, 1, 1, 6,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {75,  34, 8, 8, 1, 1, 7,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {93,  34, 8, 8, 1, 1, 8,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {111, 38, 8, 8, 1, 1, 9,    GP_SHAPE_ELLIPSE}}\
}

// Stickless 13 1
// G13, S13
#define DEFAULT_BOARD_LAYOUT_A_ALT0 {\
     {GP_ELEMENT_PIN_BUTTON, {39,  15, 6, 6, 1, 1, 27,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {18,  27, 6, 6, 1, 1, 5,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {32,  27, 6, 6, 1, 1, 3,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {44,  34, 6, 6, 1, 1, 4,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {50,  55, 7, 7, 1, 1, 2,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {65,  13, 2, 2, 1, 1, 18,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {72,  13, 2, 2, 1, 1, 19,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {79,  13, 2, 2, 1, 1, 21,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {86,  13, 2, 2, 1, 1, 20,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {93,  13, 2, 2, 1, 1, 16,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {100, 13, 2, 2, 1, 1, 17,   GP_SHAPE_ELLIPSE}}\
}
#define DEFAULT_BOARD_LAYOUT_B_ALT0 {\
     {GP_ELEMENT_PIN_BUTTON, {56, 27, 6, 6, 1, 1, 10,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {70, 24, 6, 6, 1, 1, 11,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {84, 24, 6, 6, 1, 1, 12,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {98, 27, 6, 6, 1, 1, 13,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {56, 41, 6, 6, 1, 1, 6,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {70, 38, 6, 6, 1, 1, 7,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {84, 38, 6, 6, 1, 1, 8,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {98, 41, 6, 6, 1, 1, 9,     GP_SHAPE_ELLIPSE}}\
}

// Stickless 13 2
// T13, M13, M13+
#define DEFAULT_BOARD_LAYOUT_A_ALT1 {\
     {GP_ELEMENT_PIN_BUTTON, {39,  15, 6, 6, 1, 1, 27,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {18,  27, 6, 6, 1, 1, 5,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {32,  27, 6, 6, 1, 1, 3,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {44,  34, 6, 6, 1, 1, 4,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {50,  55, 6, 6, 1, 1, 2,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {65,  13, 2, 2, 1, 1, 18,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {72,  13, 2, 2, 1, 1, 19,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {79,  13, 2, 2, 1, 1, 21,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {86,  13, 2, 2, 1, 1, 20,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {93,  13, 2, 2, 1, 1, 16,   GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {100, 13, 2, 2, 1, 1, 17,   GP_SHAPE_ELLIPSE}}\
}
#define DEFAULT_BOARD_LAYOUT_B_ALT1 {\
     {GP_ELEMENT_PIN_BUTTON, {56, 27, 6, 6, 1, 1, 10,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {70, 24, 6, 6, 1, 1, 11,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {84, 24, 6, 6, 1, 1, 12,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {98, 27, 6, 6, 1, 1, 13,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {56, 41, 6, 6, 1, 1, 6,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {70, 38, 6, 6, 1, 1, 7,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {84, 38, 6, 6, 1, 1, 8,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {98, 41, 6, 6, 1, 1, 9,     GP_SHAPE_ELLIPSE}}\
}

// Stickless 16 1
// G16, S16
#define DEFAULT_BOARD_LAYOUT_A_ALT2 {\
     {GP_ELEMENT_PIN_BUTTON, {47, 19, 4, 4, 1, 1, 27,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {32, 27, 4, 4, 1, 1, 5,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {42, 27, 4, 4, 1, 1, 3,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {50, 32, 4, 4, 1, 1, 4,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {40, 45, 4, 4, 1, 1, 26,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {64, 17, 4, 4, 1, 1, 18,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {53, 49, 5, 5, 1, 1, 2,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {66, 45, 4, 4, 1, 1, 19,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {77, 15, 2, 2, 1, 1, 21,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {84, 15, 2, 2, 1, 1, 20,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {91, 15, 2, 2, 1, 1, 16,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {98, 15, 2, 2, 1, 1, 17,    GP_SHAPE_ELLIPSE}}\
}

#define DEFAULT_BOARD_LAYOUT_B_ALT2 {\
     {GP_ELEMENT_PIN_BUTTON, {59, 27, 4, 4, 1, 1, 10,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {69, 25, 4, 4, 1, 1, 11,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {79, 25, 4, 4, 1, 1, 12,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {89, 27, 4, 4, 1, 1, 13,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {59, 37, 4, 4, 1, 1, 6,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {69, 35, 4, 4, 1, 1, 7,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {79, 35, 4, 4, 1, 1, 8,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {89, 37, 4, 4, 1, 1, 9,     GP_SHAPE_ELLIPSE}}\
}

// Stickless 16 2
// T16, M16, M16+
#define DEFAULT_BOARD_LAYOUT_A_ALT3 {\
     {GP_ELEMENT_PIN_BUTTON, {47, 19, 4, 4, 1, 1, 27,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {32, 27, 4, 4, 1, 1, 5,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {42, 27, 4, 4, 1, 1, 3,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {50, 32, 4, 4, 1, 1, 4,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {46, 45, 4, 4, 1, 1, 26,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {64, 17, 4, 4, 1, 1, 18,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {56, 49, 4, 4, 1, 1, 2,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {66, 45, 4, 4, 1, 1, 19,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {77, 15, 2, 2, 1, 1, 21,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {84, 15, 2, 2, 1, 1, 20,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {91, 15, 2, 2, 1, 1, 16,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {98, 15, 2, 2, 1, 1, 17,    GP_SHAPE_ELLIPSE}}\
}

#define DEFAULT_BOARD_LAYOUT_B_ALT3 {\
     {GP_ELEMENT_PIN_BUTTON, {59, 27, 4, 4, 1, 1, 10,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {69, 25, 4, 4, 1, 1, 11,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {79, 25, 4, 4, 1, 1, 12,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {89, 27, 4, 4, 1, 1, 13,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {59, 37, 4, 4, 1, 1, 6,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {69, 35, 4, 4, 1, 1, 7,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {79, 35, 4, 4, 1, 1, 8,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {89, 37, 4, 4, 1, 1, 9,     GP_SHAPE_ELLIPSE}}\
}

// Stickless 16 R
#define DEFAULT_BOARD_LAYOUT_A_ALT4 {\
     {GP_ELEMENT_PIN_BUTTON, {47, 19, 4, 4, 1, 1, 27,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {32, 27, 4, 4, 1, 1, 5,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {42, 27, 4, 4, 1, 1, 3,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {50, 32, 4, 4, 1, 1, 4,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {22, 30, 4, 4, 1, 1, 26,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {64, 17, 4, 4, 1, 1, 18,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {53, 49, 5, 5, 1, 1, 2,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {66, 45, 4, 4, 1, 1, 19,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {77, 15, 2, 2, 1, 1, 21,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {84, 15, 2, 2, 1, 1, 20,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {91, 15, 2, 2, 1, 1, 16,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {98, 15, 2, 2, 1, 1, 17,    GP_SHAPE_ELLIPSE}}\
}

#define DEFAULT_BOARD_LAYOUT_B_ALT4 {\
     {GP_ELEMENT_PIN_BUTTON, {59, 27, 4, 4, 1, 1, 10,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {69, 25, 4, 4, 1, 1, 11,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {79, 25, 4, 4, 1, 1, 12,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {89, 27, 4, 4, 1, 1, 13,    GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {59, 37, 4, 4, 1, 1, 6,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {69, 35, 4, 4, 1, 1, 7,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {79, 35, 4, 4, 1, 1, 8,     GP_SHAPE_ELLIPSE}},\
     {GP_ELEMENT_PIN_BUTTON, {89, 37, 4, 4, 1, 1, 9,     GP_SHAPE_ELLIPSE}}\
}
 
 // Keyboard Host enabled by default
 #define KEYBOARD_HOST_ENABLED 1
 
 #define BOARD_LED_ENABLED 1
 #define BOARD_LED_TYPE ON_BOARD_LED_MODE_MODE_INDICATOR
 
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