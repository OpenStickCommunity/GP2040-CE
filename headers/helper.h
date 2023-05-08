#ifndef _HELPER_H_
#define _HELPER_H_

#include "pico/time.h"
#include <string>

#include "BoardConfig.h"
#include <stdint.h>
#include "AnimationStation.hpp"
#include "PlayerLEDs.h"
#include "xinput_driver.h"

// GP2040-CE Version (32 character limit)
#define GP2040VERSION "v0.7.1"

const std::string BUTTON_LABEL_UP = "Up";
const std::string BUTTON_LABEL_DOWN = "Down";
const std::string BUTTON_LABEL_LEFT = "Left";
const std::string BUTTON_LABEL_RIGHT = "Right";
const std::string BUTTON_LABEL_B1 = "B1";
const std::string BUTTON_LABEL_B2 = "B2";
const std::string BUTTON_LABEL_B3 = "B3";
const std::string BUTTON_LABEL_B4 = "B4";
const std::string BUTTON_LABEL_L1 = "L1";
const std::string BUTTON_LABEL_R1 = "R1";
const std::string BUTTON_LABEL_L2 = "L2";
const std::string BUTTON_LABEL_R2 = "R2";
const std::string BUTTON_LABEL_S1 = "S1";
const std::string BUTTON_LABEL_S2 = "S2";
const std::string BUTTON_LABEL_L3 = "L3";
const std::string BUTTON_LABEL_R3 = "R3";
const std::string BUTTON_LABEL_A1 = "A1";
const std::string BUTTON_LABEL_A2 = "A2";

#define PLED_REPORT_SIZE 32

#ifndef PLED1_PIN
#define PLED1_PIN -1
#endif
#ifndef PLED2_PIN
#define PLED2_PIN -1
#endif
#ifndef PLED3_PIN
#define PLED3_PIN -1
#endif
#ifndef PLED4_PIN
#define PLED4_PIN -1
#endif
#ifndef PLED_TYPE
#define PLED_TYPE PLED_TYPE_NONE
#endif

#define PLED_MASK_ALL ((1U << PLED1_PIN) | (1U << PLED2_PIN) | (1U << PLED3_PIN) | (1U << PLED4_PIN))

const int PLED_PINS[] = {PLED1_PIN, PLED2_PIN, PLED3_PIN, PLED4_PIN};

#endif