#ifndef PICO_BOARD_CONFIG_H_
#define PICO_BOARD_CONFIG_H_

#include "enums.pb.h"
#include "class/hid/hid.h"

#define BOARD_CONFIG_LABEL "DiagonalDpad"

// 禁用原来的上下左右方向引脚，避免冲突
#define GPIO_PIN_16 GpioAction::GPIO_INPUT_DISABLED
#define GPIO_PIN_17 GpioAction::GPIO_INPUT_DISABLED
#define GPIO_PIN_18 GpioAction::GPIO_INPUT_DISABLED
#define GPIO_PIN_19 GpioAction::GPIO_INPUT_DISABLED

// 拳脚按键保持标准Pico布局，可自行修改GPIO编号
#define GPIO_PIN_08 GpioAction::BUTTON_PRESS_B1
#define GPIO_PIN_07 GpioAction::BUTTON_PRESS_B2
#define GPIO_PIN_06 GpioAction::BUTTON_PRESS_R2
#define GPIO_PIN_05 GpioAction::BUTTON_PRESS_L2
#define GPIO_PIN_12 GpioAction::BUTTON_PRESS_B3
#define GPIO_PIN_11 GpioAction::BUTTON_PRESS_B4
#define GPIO_PIN_10 GpioAction::BUTTON_PRESS_R1
#define GPIO_PIN_09 GpioAction::BUTTON_PRESS_L1
#define GPIO_PIN_15 GpioAction::BUTTON_PRESS_S1
#define GPIO_PIN_13 GpioAction::BUTTON_PRESS_S2
#define GPIO_PIN_21 GpioAction::BUTTON_PRESS_L3
#define GPIO_PIN_22 GpioAction::BUTTON_PRESS_R3
#define GPIO_PIN_14 GpioAction::BUTTON_PRESS_A1
#define GPIO_PIN_20 GpioAction::BUTTON_PRESS_A2

#endif
