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
#define ENCODER_ONE_MULTIPLIER 1.0f
#endif

#ifndef ENCODER_ONE_COUNTS_PER_DETENT
#define ENCODER_ONE_COUNTS_PER_DETENT 4
#endif

#ifndef ENCODER_ONE_TYPE
#define ENCODER_ONE_TYPE ENCODER_TYPE_MECHANICAL_DETENTED
#endif

#ifndef ENCODER_ONE_PULSE_HOLD_MS
#define ENCODER_ONE_PULSE_HOLD_MS 30
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
#define ENCODER_TWO_MULTIPLIER 1.0f
#endif

#ifndef ENCODER_TWO_COUNTS_PER_DETENT
#define ENCODER_TWO_COUNTS_PER_DETENT 4
#endif

#ifndef ENCODER_TWO_TYPE
#define ENCODER_TWO_TYPE ENCODER_TYPE_MECHANICAL_DETENTED
#endif

#ifndef ENCODER_TWO_PULSE_HOLD_MS
#define ENCODER_TWO_PULSE_HOLD_MS 30
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
        int32_t minRange = 0;
        int32_t maxRange = 0;
        uint32_t resetAfter = 0;
        bool allowWrapAround = false;
        float multiplier = 1.0f;
        // Number of raw quadrature edges that constitute one logical step.
        // 4 = full quadrature (one step per detent), 2 = half, 1 = every edge.
        uint8_t countsPerDetent = 4;
        // Hold time for DPAD pulses / minimum spacing for VOLUME events, in ms.
        uint32_t pulseHoldMs = 30;
        // Pre-computed: number of logical steps that span the configured output range.
        int32_t stepsPerFullScale = 0;
    } EncoderPinMap;

    // Per-encoder runtime state. Fields touched by the ISR are marked volatile.
    typedef struct {
        // ISR-owned state. prevState is the last sampled (A<<1)|B Gray-code value (0..3).
        volatile uint8_t prevState = 0;
        // Signed accumulator of valid quadrature transitions (+/-1 per edge).
        // Only touched by the ISR (writer) and by the main loop under interrupt-disable
        // when consuming, so volatility plus a brief IRQ-disable read is sufficient.
        volatile int32_t rawCounts = 0;

        // Main-loop-owned state.
        int32_t accumulatedSteps = 0;   // total logical steps since reset
        int32_t prevSteps = 0;          // last value seen by process(), for delta detection
        int32_t rawRemainder = 0;       // raw counts not yet promoted to a full step
        uint32_t changeTime = 0;        // last time accumulatedSteps changed

        // DPAD mode pulse latching.
        int8_t pulseDir = 0;            // -1, 0, or +1
        uint32_t pulseUntil = 0;        // millis at which the current pulse expires
    } EncoderPinState;

    // Quadrature transition table indexed by ((prevA<<3)|(prevB<<2)|(curA<<1)|curB).
    // Values: +1 valid CW edge, -1 valid CCW edge, 0 = no change or invalid (glitch).
    // Defined out-of-line in rotaryencoder.cpp.
    static const int8_t QDEC_LUT[16];

private:
    EncoderPinState encoderState[MAX_ENCODERS];
    EncoderPinMap encoderMap[MAX_ENCODERS] = {
        EncoderPinMap{},
        EncoderPinMap{},
    };

    static RotaryEncoderInput* instance;

    // Raw GPIO IRQ handler. We use gpio_add_raw_irq_handler so we coexist with
    // any other GPIO IRQ user (e.g. the TinyUSB MAX3421 host BSP) that would
    // otherwise be clobbered by the legacy single-callback API. The handler
    // walks encoderMap directly (MAX_ENCODERS is small) instead of using a
    // pin->index reverse map, which keeps the dispatch table out of .data and
    // avoids one indirection per IRQ.
    static void gpioIrqHandler();
    void handleEdge(uint8_t encoderIndex);

    int32_t map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max);
    int32_t wrapMod(int32_t value, int32_t modulus);

    uint16_t mapEncoderValueStick(int8_t index, int32_t steps);
    uint16_t mapEncoderValueTrigger(int8_t index, int32_t steps);

    // Cached gamepad pointer; the Storage-owned Gamepad* doesn't change after
    // boot, so we look it up once in setup() and avoid the per-tick indirection.
    Gamepad* gamepad = nullptr;
};

#endif  // _ROTARYENCODER_H
