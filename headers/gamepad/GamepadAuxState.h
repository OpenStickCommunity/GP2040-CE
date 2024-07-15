#pragma once

#include <stdint.h>
#include <list>
using namespace std;
#include "GamepadEnums.h"
#include "enums.pb.h"

#define GAMEPAD_AUX_MAX_TOUCHPADS 2

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
    bool active;
    uint16_t x;
};

struct GamepadAux1DRelativeSensor
{
    bool enabled;
    bool active;
    int16_t x;
};

struct GamepadAux2DSensor
{
    bool enabled;
    bool active;
    uint16_t x;
    uint16_t y;
};

struct GamepadAux2DRelativeSensor
{
    bool enabled;
    bool active;
    int16_t x;
    int16_t y;
};

struct GamepadAux3DSensor
{
    bool enabled;
    bool active;
    uint16_t x;
    uint16_t y;
    uint16_t z;
};

struct GamepadAux3DRelativeSensor
{
    bool enabled;
    bool active;
    int16_t x;
    int16_t y;
    int16_t z;
};

struct GamepadAux4DSensor
{
    bool enabled;
    bool active;
    uint16_t x;
    uint16_t y;
    uint16_t z;
    uint16_t t;
};

struct GamepadAux4DRelativeSensor
{
    bool enabled;
    bool active;
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t t;
};

struct GamepadAuxRGBSensor
{
    bool enabled;
    bool active;
    GamepadAuxColor color;
    uint8_t durationOn;
    uint8_t durationOff;
};

struct GamepadAuxHapticChannel
{
    bool enabled;
    bool active;
    uint16_t intensity;
};

struct GamepadAuxSensors
{
    GamepadAux3DRelativeSensor mouse;

    GamepadAux3DSensor touchpad[GAMEPAD_AUX_MAX_TOUCHPADS];
    GamepadAux3DSensor gyroscope;
    GamepadAux3DSensor accelerometer;
    GamepadAux3DSensor magnetometer;
    GamepadAux4DSensor timeOfFlight;

    GamepadAuxRGBSensor statusLight;
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
