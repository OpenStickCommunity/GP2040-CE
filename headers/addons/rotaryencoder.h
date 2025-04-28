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

#ifndef ENCODER_ONE_PIN_A
#define ENCODER_ONE_PIN_A -1
#endif

#ifndef ENCODER_ONE_PIN_B
#define ENCODER_ONE_PIN_B -1
#endif

#ifndef ENCODER_ONE_RESET
#define ENCODER_ONE_RESET 0
#endif

#ifndef ENCODER_ONE_MODE
#define ENCODER_ONE_MODE ENCODER_MODE_NONE
#endif

#ifndef ENCODER_ONE_WRAP
#define ENCODER_ONE_WRAP 0
#endif

#ifndef ENCODER_ONE_MULTIPLIER
#define ENCODER_ONE_MULTIPLIER 1
#endif

#ifndef ENCODER_TWO_ENABLED
#define ENCODER_TWO_ENABLED 0
#endif

#ifndef ENCODER_TWO_PPR
#define ENCODER_TWO_PPR 24
#endif

#ifndef ENCODER_TWO_PIN_A
#define ENCODER_TWO_PIN_A -1
#endif

#ifndef ENCODER_TWO_PIN_B
#define ENCODER_TWO_PIN_B -1
#endif

#ifndef ENCODER_TWO_RESET
#define ENCODER_TWO_RESET 0
#endif

#ifndef ENCODER_TWO_MODE
#define ENCODER_TWO_MODE ENCODER_MODE_NONE
#endif

#ifndef ENCODER_TWO_WRAP
#define ENCODER_TWO_WRAP 0
#endif

#ifndef ENCODER_TWO_MULTIPLIER
#define ENCODER_TWO_MULTIPLIER 1
#endif

#define MAX_ENCODERS 2
#define ENCODER_RADIUS 1440 // 4 phases * 360
#define ENCODER_PRECISION 16

// RotaryEncoderName Module Name
#define RotaryEncoderName "Rotary"

class RotaryEncoderInput : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // Rotary Setup
    virtual void preprocess() {}
    virtual void process();     // Rotary process
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
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
        bool allowWrapAround = false;
        double multiplier = 0;
    } EncoderPinMap;

    typedef struct {
        bool pinA = false;
        bool pinB = false;
        bool prevA = false;
        bool prevB = false;
        uint32_t updateTime = 0;
        uint32_t changeTime = 0;
        uint8_t delay = 1;
    } EncoderPinState;
private:
    EncoderPinState encoderState[MAX_ENCODERS];
    int32_t encoderValues[MAX_ENCODERS];
    int32_t prevValues[MAX_ENCODERS];
    EncoderPinMap encoderMap[MAX_ENCODERS] = {
        {false, -1, -1, 24, ENCODER_MODE_NONE, -1, -1},
        {false, -1, -1, 24, ENCODER_MODE_NONE, -1, -1},
    };

    int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
    int32_t bounds(int32_t x, int32_t out_min, int32_t out_max);
    uint16_t mapEncoderValueStick(int8_t index, int32_t encoderValue, uint16_t ppr);
    uint16_t mapEncoderValueTrigger(int8_t index, int32_t encoderValue, uint16_t ppr);
    int8_t mapEncoderValueDPad(int8_t index, int32_t encoderValue, uint16_t ppr);

    int8_t getEncoderIndexByPin(uint8_t pin);
    
    bool dpadUp = false;
    bool dpadDown = false;
    bool dpadLeft = false;
    bool dpadRight = false;
};

#endif  // _ROTARYENCODER_H
