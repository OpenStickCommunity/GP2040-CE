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

// QuadratureEncoder counts every edge of both channels (4 per quadrature cycle).
// The original polled decoder only counted 2 per cycle, and the existing value
// mapping/PPR math was tuned around that, so counts are divided down to keep the
// same feel for existing configurations.
#ifndef ENCODER_LEGACY_COUNT_DIVISOR
#define ENCODER_LEGACY_COUNT_DIVISOR 2
#endif

// PIO capture sample rate. Real edges from a 600 PPR encoder at 10 rev/s are
// ~40 us apart, so 2 MHz has a large margin while ignoring sub-microsecond
// glitches that would otherwise fill the capture buffer with bounce.
#ifndef ENCODER_CAPTURE_SAMPLE_HZ
#define ENCODER_CAPTURE_SAMPLE_HZ 2000000
#endif

// ENCODER_MODE_TURNTABLE: movement needed before the digital direction fires or
// reverses, in degrees of platter rotation (converted with the encoder's PPR;
// at least one count). 2 degrees = 13 counts on a 600 PPR encoder.
#ifndef ENCODER_TURNTABLE_THRESHOLD_DEGREES
#define ENCODER_TURNTABLE_THRESHOLD_DEGREES 2
#endif

// ENCODER_MODE_TURNTABLE: how long the direction stays held after the last movement.
// The encoder's "Reset After" setting overrides this when it's non-zero.
#ifndef ENCODER_TURNTABLE_HOLD_MS
#define ENCODER_TURNTABLE_HOLD_MS 100
#endif

// ENCODER_MODE_TURNTABLE: range of the absolute platter position (0 .. N-1, wrapping).
#ifndef ENCODER_TURNTABLE_POSITION_UNITS
#define ENCODER_TURNTABLE_POSITION_UNITS 256
#endif

#include "QuadratureEncoder.h"
#include "QuadratureTurntable.h"

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

    // Turntable state for an encoder in ENCODER_MODE_TURNTABLE, for input modes to
    // report however they need (digital direction() or analog position()).
    // nullptr if the encoder isn't enabled or isn't in turntable mode.
    const QuadratureTurntable * getTurntable(uint8_t index) const {
        if ((index >= MAX_ENCODERS) || !encoderMap[index].enabled || (encoderMap[index].mode != ENCODER_MODE_TURNTABLE)) return nullptr;
        return &turntables[index];
    }

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
        // derived once in setup() (previously recomputed in soft-float every loop)
        int32_t increment = 0;
        int32_t totalPositions = 0;
    } EncoderPinMap;

    typedef struct {
        uint32_t lastRawCount = 0;  // last QuadratureEncoder count seen (4x, wraps)
        int32_t lastDelta = 0;      // 4x count change in the latest process()
        int32_t pendingCounts = 0;  // counts not yet converted to legacy steps
        uint32_t changeTime = 0;
    } EncoderPinState;
private:
    QuadratureEncoder encoders[MAX_ENCODERS];
    QuadratureTurntable turntables[MAX_ENCODERS];
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
