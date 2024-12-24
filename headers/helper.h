#ifndef _HELPER_H_
#define _HELPER_H_

#include "pico/time.h"
#include <string>

#include "BoardConfig.h"
#include <stdint.h>
#include "AnimationStation.hpp"
#include "PlayerLEDs.h"

// GP2040-CE Board Config (64 character limit)
#ifndef GP2040_BOARDCONFIG
#define GP2040_BOARDCONFIG "Unknown"
#endif

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
#ifndef PLED_COLOR
#define PLED_COLOR ColorWhite // White
#endif

#ifndef CASE_RGB_TYPE
#define CASE_RGB_TYPE CASE_RGB_TYPE_NONE
#endif

#ifndef CASE_RGB_INDEX
#define CASE_RGB_INDEX -1
#endif

#ifndef CASE_RGB_COLOR
#define CASE_RGB_COLOR ColorGreen // Green
#endif

#ifndef CASE_RGB_COUNT
#define CASE_RGB_COUNT 0
#endif

static inline bool isValidPin(int32_t pin) {
    int32_t numBank0GPIOS = NUM_BANK0_GPIOS;
    return pin >= 0 && pin < numBank0GPIOS; }

#endif
