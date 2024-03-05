#ifndef _ROTARYENCODER_H
#define _ROTARYENCODER_H

#include "gpaddon.h"

#include "GamepadEnums.h"
#include "types.h"

#ifndef ROTARY_ENCODER_ENABLED
#define ROTARY_ENCODER_ENABLED 0
#endif

#ifndef ENCODER_ONE_ENABLED
#define ENCODER_ONE_ENABLED 0
#endif

#ifndef ENCODER_ONE_PPR
#define ENCODER_ONE_PPR 24
#endif

#ifndef ENCODER_TWO_ENABLED
#define ENCODER_TWO_ENABLED 0
#endif

#ifndef ENCODER_TWO_PPR
#define ENCODER_TWO_PPR 24
#endif

#define MAX_ENCODERS 2

// RotaryEncoderName Module Name
#define RotaryEncoderName "Rotary"

class RotaryEncoderInput : public GPAddon {
public:
    virtual bool available();
	virtual void setup();       // Rotary Setup
    virtual void preprocess() {}
	virtual void process();     // Rotary process
    virtual std::string name() { return RotaryEncoderName; }

    typedef struct {
        bool enabled = false;
        int8_t pinA = -1;
        int8_t pinB = -1;
        // encoder properties
        uint16_t pulsesPerRevolution = 24;
        RotaryEncoderPinMode mode = ENCODER_MODE_NONE;
        int32_t minRange = -1;
        int32_t maxRange = -1;
        uint32_t resetAfter = 0;
    } EncoderPinMap;

    typedef struct {
        bool pinA = false;
        bool pinB = false;
        bool prevA = false;
        bool prevB = false;
        uint32_t updateTime = 0;
        uint32_t changeTime = 0;
        uint8_t delay = 5;
    } EncoderPinState;
private:
    EncoderPinState encoderState[MAX_ENCODERS];
    int32_t encoderValues[MAX_ENCODERS];
    int32_t prevValues[MAX_ENCODERS];
    EncoderPinMap encoderMap[MAX_ENCODERS] = {
        {false, -1, -1, 24, ENCODER_MODE_NONE, -1, -1},
        {false, -1, -1, 24, ENCODER_MODE_NONE, -1, -1},

//        {true, 8, 7, 24, ENCODER_MODE_LEFT_ANALOG_X, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX},
//        {true, 6, 5, 24, ENCODER_MODE_LEFT_ANALOG_Y, GAMEPAD_JOYSTICK_MIN, GAMEPAD_JOYSTICK_MAX},
//        {true, 8, 7, 24, ENCODER_MODE_LEFT_TRIGGER, GAMEPAD_TRIGGER_MIN, GAMEPAD_TRIGGER_MAX},
//        {true, 6, 5, 24, ENCODER_MODE_RIGHT_TRIGGER, GAMEPAD_TRIGGER_MIN, GAMEPAD_TRIGGER_MAX},
//        {true, 8, 7, 24, ENCODER_MODE_DPAD_X, -1, -1},
//        {true, 6, 5, 24, ENCODER_MODE_DPAD_Y, -1, -1},
    };

    int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
    int32_t bounds(int32_t x, int32_t out_min, int32_t out_max);
    uint16_t mapEncoderValueStick(int32_t encoderValue, uint16_t ppr);
    uint16_t mapEncoderValueTrigger(int32_t encoderValue, uint16_t ppr);
    int8_t mapEncoderValueDPad(int32_t encoderValue, uint16_t ppr);

    bool dpadUp = false;
    bool dpadDown = false;
    bool dpadLeft = false;
    bool dpadRight = false;
};

#endif  // _ROTARYENCODER_H
