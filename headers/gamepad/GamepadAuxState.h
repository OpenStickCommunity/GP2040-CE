#pragma once

#include <stdint.h>
#include <list>
using namespace std;
#include "GamepadEnums.h"
#include "enums.pb.h"

#define GAMEPAD_AUX_MAX_TOUCHPADS 2

struct GamepadAuxColor
{
    uint8_t alpha = 0;
    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};

struct GamepadAuxPlayerID
{
    bool enabled = false;
    bool active = false;
    uint32_t value = 0;
    uint32_t ledValue = 0;
    uint32_t ledBlinkOn = 0;
    uint32_t ledBlinkOff = 0;
};

struct GamepadAux1DSensor
{
    bool enabled = false;
    bool active = false;
    uint16_t x = 0;
};

struct GamepadAux1DRelativeSensor
{
    bool enabled = false;
    bool active = false;
    int16_t x = 0;
};

struct GamepadAux2DSensor
{
    bool enabled = false;
    bool active = false;
    uint16_t x = 0;
    uint16_t y = 0;
};

struct GamepadAux2DRelativeSensor
{
    bool enabled = false;
    bool active = false;
    int16_t x = 0;
    int16_t y = 0;
};

struct GamepadAux3DSensor
{
    bool enabled = false;
    bool active = false;
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t z = 0;
};

struct GamepadAux3DRelativeSensor
{
    bool enabled = false;
    bool active = false;
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
};

struct GamepadAux4DSensor
{
    bool enabled = false;
    bool active = false;
    uint16_t x = 0;
    uint16_t y = 0;
    uint16_t z = 0;
    uint16_t t = 0;
};

struct GamepadAux4DRelativeSensor
{
    bool enabled = false;
    bool active = false;
    int16_t x = 0;
    int16_t y = 0;
    int16_t z = 0;
    int16_t t = 0;
};

struct GamepadAuxRGBSensor
{
    bool enabled = false;
    bool active = false;
    GamepadAuxColor color;
    uint8_t durationOn = 0;
    uint8_t durationOff = 0;
};

struct GamepadAuxHapticChannel
{
    bool enabled = false;
    bool active = false;
    uint16_t intensity = 0;
};

struct GamepadAuxTurbo
{
    bool enabled = false;
    bool active = false;
    uint8_t activity = 0;
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
    GamepadAuxPlayerID playerID;
    
    GamepadAuxColor primaryColor;
    GamepadAuxColor secondaryColor;

    GamepadAuxSensors sensors;

    GamepadAuxHaptics haptics;

    GamepadAuxTurbo turbo;
};
