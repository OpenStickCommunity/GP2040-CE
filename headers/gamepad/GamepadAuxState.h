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
    double x;
};

struct GamepadAux2DSensor
{
    double x;
    double y;
};

struct GamepadAux3DSensor
{
    double x;
    double y;
    double z;
};

struct GamepadAux4DSensor
{
    double x;
    double y;
    double z;
    double t;
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
