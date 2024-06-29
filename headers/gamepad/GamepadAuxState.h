#pragma once

#include <stdint.h>
#include <list>
using namespace std;
#include "GamepadEnums.h"
#include "enums.pb.h"

struct GamepadAuxColor
{
    uint8_t alpha;
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct GamepadAux1DSensor
{
    bool enabled;
    uint16_t x;
};

struct GamepadAux2DSensor
{
    bool enabled;
    uint16_t x;
    uint16_t y;
};

struct GamepadAux3DSensor
{
    bool enabled;
    uint16_t x;
    uint16_t y;
    uint16_t z;
};

struct GamepadAux4DSensor
{
    bool enabled;
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t t;
};

struct GamepadAuxHapticChannel
{
    bool enabled;
    uint16_t intensity;
};

struct GamepadAuxSensors
{
    GamepadAux2DSensor touchpad;
    GamepadAux3DSensor gyroscope;
    GamepadAux3DSensor accelerometer;
    GamepadAux3DSensor magnetometer;
    GamepadAux4DSensor timeOfFlight;
};

struct GamepadAuxHaptics
{
    GamepadAuxHapticChannel leftActuator;
    GamepadAuxHapticChannel leftTrigger;

    GamepadAuxHapticChannel rightActuator;
    GamepadAuxHapticChannel rightTrigger;
};

struct GamepadAuxState
{
    GamepadAuxColor primaryColor;
    GamepadAuxColor secondaryColor;

    GamepadAuxSensors sensors;

    GamepadAuxHaptics haptics;
};
