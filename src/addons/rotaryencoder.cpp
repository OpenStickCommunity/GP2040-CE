#include "addons/rotaryencoder.h"

#include "eventmanager.h"
#include "storagemanager.h"
#include "GPEncoderEvent.h"
#include "types.h"

#include "GamepadEnums.h"
#include "helper.h"
#include "config.pb.h"

#include "hardware/gpio.h"
#include "hardware/sync.h"

// Static members -------------------------------------------------------------

// Quadrature state-transition table. Index = (prevA<<3)|(prevB<<2)|(curA<<1)|curB.
// +1 = valid clockwise transition, -1 = valid counter-clockwise transition,
// 0 = no change OR invalid double-edge (treated as a glitch and discarded).
const int8_t RotaryEncoderInput::QDEC_LUT[16] = {
     0, -1, +1,  0,
    +1,  0,  0, -1,
    -1,  0,  0, +1,
     0, +1, -1,  0,
};

// RP2040 has 30 user GPIOs (0..29); we size to 32 for safety.
int8_t RotaryEncoderInput::pinToEncoder[32] = {
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
    -1, -1, -1, -1, -1, -1, -1, -1,
};

RotaryEncoderInput* RotaryEncoderInput::instance = nullptr;

// ---------------------------------------------------------------------------

bool RotaryEncoderInput::available() {
    const RotaryOptions& options = Storage::getInstance().getAddonOptions().rotaryOptions;
    return options.enabled;
}

static inline uint8_t sampleQuadState(uint8_t pinA, uint8_t pinB) {
    return (uint8_t)((gpio_get(pinA) ? 1 : 0) << 1) | (uint8_t)(gpio_get(pinB) ? 1 : 0);
}

void RotaryEncoderInput::setup()
{
    const RotaryOptions& options = Storage::getInstance().getAddonOptions().rotaryOptions;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    instance = this;

    const RotaryPinOptions* perEncoder[MAX_ENCODERS] = {
        &options.encoderOne,
        &options.encoderTwo,
    };

    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        const RotaryPinOptions& src = *perEncoder[i];
        EncoderPinMap& dst = encoderMap[i];

        dst.enabled = src.enabled && (src.pinA != -1) && (src.pinB != -1);
        if (!dst.enabled) continue;

        dst.pinA = src.pinA;
        dst.pinB = src.pinB;
        dst.mode = src.mode;
        dst.pulsesPerRevolution = src.pulsesPerRevolution > 0 ? src.pulsesPerRevolution : 24;
        dst.resetAfter = src.resetAfter;
        dst.allowWrapAround = src.allowWrapAround;
        dst.multiplier = (src.multiplier > 0.0f) ? src.multiplier : 1.0f;
        dst.countsPerDetent = (src.countsPerDetent >= 1 && src.countsPerDetent <= 4) ? (uint8_t)src.countsPerDetent : 4;
        dst.pulseHoldMs = src.pulseHoldMs;

        // Default output ranges by mode.
        if ((dst.mode == ENCODER_MODE_LEFT_TRIGGER) || (dst.mode == ENCODER_MODE_RIGHT_TRIGGER)) {
            gamepad->hasAnalogTriggers = true;
            dst.minRange = GAMEPAD_TRIGGER_MIN;
            dst.maxRange = GAMEPAD_TRIGGER_MAX;
        } else if ((dst.mode == ENCODER_MODE_LEFT_ANALOG_X) || (dst.mode == ENCODER_MODE_LEFT_ANALOG_Y)
                || (dst.mode == ENCODER_MODE_RIGHT_ANALOG_X) || (dst.mode == ENCODER_MODE_RIGHT_ANALOG_Y)) {
            dst.minRange = GAMEPAD_JOYSTICK_MIN;
            dst.maxRange = GAMEPAD_JOYSTICK_MAX;
        } else {
            dst.minRange = 0;
            dst.maxRange = 0;
        }

        // Logical steps spanning a full revolution = (raw edges per rev) / counts-per-detent.
        // For analog modes, multiplier scales how many revolutions cover the full output range.
        // stepsPerFullScale = stepsPerRevolution / multiplier (rounded, min 1).
        const int32_t edgesPerRev = (int32_t)dst.pulsesPerRevolution * 4; // 4 edges per pulse
        const int32_t stepsPerRev = edgesPerRev / dst.countsPerDetent;
        int32_t span = (int32_t)((float)stepsPerRev / dst.multiplier);
        if (span < 1) span = 1;
        dst.stepsPerFullScale = span;
    }

    // Configure GPIOs and enable per-pin quadrature edge interrupts.
    // Use a single shared callback registered once with the SDK; dispatch is
    // by GPIO number via pinToEncoder[].
    bool callbackInstalled = false;
    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        if (!encoderMap[i].enabled) continue;

        const uint8_t pinA = (uint8_t)encoderMap[i].pinA;
        const uint8_t pinB = (uint8_t)encoderMap[i].pinB;

        gpio_init(pinA);
        gpio_set_dir(pinA, GPIO_IN);
        gpio_pull_up(pinA);

        gpio_init(pinB);
        gpio_set_dir(pinB, GPIO_IN);
        gpio_pull_up(pinB);

        // Allow the pull-ups time to settle before sampling the initial state.
        // gpio_pull_up returns immediately; a few NOPs are typically enough.
        for (volatile int n = 0; n < 1000; n++) { /* settle */ }

        // Map both pins to this encoder for IRQ dispatch.
        if (pinA < 32) pinToEncoder[pinA] = (int8_t)i;
        if (pinB < 32) pinToEncoder[pinB] = (int8_t)i;

        // Seed the previous Gray state so the first real edge produces a valid transition.
        encoderState[i].prevState = sampleQuadState(pinA, pinB);
        encoderState[i].rawCounts = 0;
        encoderState[i].accumulatedSteps = 0;
        encoderState[i].prevSteps = 0;
        encoderState[i].rawRemainder = 0;

        const uint32_t edgeMask = GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;
        if (!callbackInstalled) {
            gpio_set_irq_enabled_with_callback(pinA, edgeMask, true, &RotaryEncoderInput::gpioIrqCallback);
            callbackInstalled = true;
        } else {
            gpio_set_irq_enabled(pinA, edgeMask, true);
        }
        gpio_set_irq_enabled(pinB, edgeMask, true);
    }
}

// Static IRQ thunk - called from the SDK on every enabled GPIO edge.
// Keep this function tight: a vigorously spinning 360 P/R encoder generates
// edges every ~140 us, so we must do as little as possible per edge.
void RotaryEncoderInput::gpioIrqCallback(uint gpio, uint32_t /*events*/) {
    if (gpio >= 32) return;
    const int8_t idx = pinToEncoder[gpio];
    if (idx < 0 || idx >= MAX_ENCODERS) return;
    if (instance == nullptr) return;
    instance->handleEdge((uint8_t)idx);
}

void RotaryEncoderInput::handleEdge(uint8_t encoderIndex) {
    EncoderPinMap& m = encoderMap[encoderIndex];
    EncoderPinState& s = encoderState[encoderIndex];

    const uint8_t curState = sampleQuadState((uint8_t)m.pinA, (uint8_t)m.pinB);
    const uint8_t prevState = s.prevState;

    if (curState == prevState) {
        // No change (spurious or already handled by another edge on the partner pin).
        return;
    }

    const uint8_t lutIndex = (uint8_t)((prevState << 2) | curState);
    const int8_t delta = QDEC_LUT[lutIndex & 0x0F];

    // Always advance prevState even on invalid transitions, otherwise we'd get
    // stuck reporting an error forever after a single missed edge.
    s.prevState = curState;

    if (delta != 0) {
        s.rawCounts += delta;
    }
}

void RotaryEncoderInput::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    const uint32_t now = getMillis();

    // Reset per-frame DPAD intent; we set bits below based on currently latched pulses.
    dpadUp = dpadDown = dpadLeft = dpadRight = false;

    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        EncoderPinMap& m = encoderMap[i];
        EncoderPinState& s = encoderState[i];
        if (!m.enabled) continue;

        // Atomically snapshot rawCounts and zero it, so the ISR can keep counting
        // freshly while we process. Brief IRQ-disable on this core.
        uint32_t saved = save_and_disable_interrupts();
        const int32_t raw = s.rawCounts;
        s.rawCounts = 0;
        restore_interrupts(saved);

        if (raw != 0) {
            // Promote raw quadrature edges into logical steps, retaining the remainder.
            const int32_t total = s.rawRemainder + raw;
            const int32_t cpd = m.countsPerDetent ? m.countsPerDetent : 1;
            // Use truncated-toward-zero division so positive and negative motion
            // both accumulate to whole steps symmetrically.
            int32_t newSteps;
            int32_t remainder;
            if (total >= 0) {
                newSteps = total / cpd;
                remainder = total - (newSteps * cpd);
            } else {
                newSteps = -((-total) / cpd);
                remainder = total - (newSteps * cpd);
            }
            s.rawRemainder = remainder;
            if (newSteps != 0) {
                s.accumulatedSteps += newSteps;
                s.changeTime = now;
            }
        }

        // Auto-center after inactivity. Only applies to analog stick modes, where
        // accumulatedSteps == 0 maps to the joystick midpoint - giving the user a
        // spinner that drifts back to neutral when released. Trigger / DPAD / volume
        // modes deliberately keep their last value (a stuck-throttle would feel
        // wrong, and DPAD/volume don't use the accumulator for steady-state output).
        const bool isAutoCenterMode =
            (m.mode == ENCODER_MODE_LEFT_ANALOG_X)  || (m.mode == ENCODER_MODE_LEFT_ANALOG_Y) ||
            (m.mode == ENCODER_MODE_RIGHT_ANALOG_X) || (m.mode == ENCODER_MODE_RIGHT_ANALOG_Y);
        if (isAutoCenterMode && m.resetAfter > 0 && (now - s.changeTime) >= m.resetAfter) {
            s.accumulatedSteps = 0;
            s.rawRemainder = 0;
        }

        const int32_t deltaSteps = s.accumulatedSteps - s.prevSteps;

        switch (m.mode) {
            case ENCODER_MODE_LEFT_ANALOG_X:
                gamepad->state.lx = mapEncoderValueStick(i, s.accumulatedSteps);
                break;
            case ENCODER_MODE_LEFT_ANALOG_Y:
                gamepad->state.ly = mapEncoderValueStick(i, s.accumulatedSteps);
                break;
            case ENCODER_MODE_RIGHT_ANALOG_X:
                gamepad->state.rx = mapEncoderValueStick(i, s.accumulatedSteps);
                break;
            case ENCODER_MODE_RIGHT_ANALOG_Y:
                gamepad->state.ry = mapEncoderValueStick(i, s.accumulatedSteps);
                break;
            case ENCODER_MODE_LEFT_TRIGGER:
                gamepad->state.lt = (uint8_t)mapEncoderValueTrigger(i, s.accumulatedSteps);
                break;
            case ENCODER_MODE_RIGHT_TRIGGER:
                gamepad->state.rt = (uint8_t)mapEncoderValueTrigger(i, s.accumulatedSteps);
                break;
            case ENCODER_MODE_DPAD_X:
            case ENCODER_MODE_DPAD_Y: {
                const int8_t pulse = mapEncoderValueDPad(i, deltaSteps);
                if (pulse != 0) {
                    s.pulseDir = pulse;
                    // Hold for at least pulseHoldMs (default 30 ms) so a host poll
                    // is guaranteed to see the press even at 1 ms input cadence.
                    s.pulseUntil = now + (m.pulseHoldMs ? m.pulseHoldMs : 1);
                }
                if (s.pulseDir != 0 && now < s.pulseUntil) {
                    if (m.mode == ENCODER_MODE_DPAD_X) {
                        if (s.pulseDir > 0) dpadRight = true; else dpadLeft = true;
                    } else {
                        if (s.pulseDir > 0) dpadDown = true; else dpadUp = true;
                    }
                } else {
                    s.pulseDir = 0;
                }
                break;
            }
            case ENCODER_MODE_VOLUME: {
                if (deltaSteps != 0) {
                    const uint32_t throttle = m.pulseHoldMs ? m.pulseHoldMs : 0;
                    if (throttle == 0 || (now - s.lastVolumeEventMs) >= throttle) {
                        EventManager::getInstance().triggerEvent(
                            new GPEncoderChangeEvent(i, deltaSteps > 0 ? 1 : -1));
                        s.lastVolumeEventMs = now;
                    }
                }
                break;
            }
            case ENCODER_MODE_NONE:
            default:
                break;
        }

        // The generic GPEncoderChangeEvent is used by the keyboard driver to drive
        // host volume changes. Only fire it for VOLUME mode (handled above) to avoid
        // accidental volume changes when the encoder is mapped to a stick / dpad / trigger.

        s.prevSteps = s.accumulatedSteps;
    }

    if (dpadUp)    gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (dpadDown)  gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (dpadLeft)  gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (dpadRight) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
}

uint16_t RotaryEncoderInput::mapEncoderValueStick(int8_t index, int32_t steps) {
    const EncoderPinMap& m = encoderMap[index];
    const int32_t span = m.stepsPerFullScale > 0 ? m.stepsPerFullScale : 1;

    if (m.allowWrapAround) {
        // Map steps into [0, span) and then linearly across the joystick range.
        const int32_t wrapped = wrapMod(steps, span);
        return (uint16_t)map(wrapped, 0, span - 1, m.minRange, m.maxRange);
    }

    // Center the joystick at midpoint; positive steps go toward maxRange.
    const int32_t halfSpan = span / 2;
    const int32_t minVal = -halfSpan;
    const int32_t maxVal =  halfSpan;
    int32_t mapped = map(steps, minVal, maxVal, m.minRange, m.maxRange);
    if (mapped < m.minRange) mapped = m.minRange;
    if (mapped > m.maxRange) mapped = m.maxRange;
    return (uint16_t)mapped;
}

uint16_t RotaryEncoderInput::mapEncoderValueTrigger(int8_t index, int32_t steps) {
    const EncoderPinMap& m = encoderMap[index];
    const int32_t span = m.stepsPerFullScale > 0 ? m.stepsPerFullScale : 1;

    if (m.allowWrapAround) {
        const int32_t wrapped = wrapMod(steps, span);
        return (uint16_t)map(wrapped, 0, span - 1, m.minRange, m.maxRange);
    }

    int32_t mapped = map(steps, 0, span, m.minRange, m.maxRange);
    if (mapped < m.minRange) mapped = m.minRange;
    if (mapped > m.maxRange) mapped = m.maxRange;
    return (uint16_t)mapped;
}

int8_t RotaryEncoderInput::mapEncoderValueDPad(int8_t /*index*/, int32_t deltaSteps) {
    if (deltaSteps > 0) return +1;
    if (deltaSteps < 0) return -1;
    return 0;
}

int32_t RotaryEncoderInput::map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    if (in_max == in_min) return out_min;
    return (int32_t)(((int64_t)(x - in_min) * (int64_t)(out_max - out_min)) / (int64_t)(in_max - in_min)) + out_min;
}

int32_t RotaryEncoderInput::wrapMod(int32_t value, int32_t modulus) {
    if (modulus <= 0) return 0;
    int32_t r = value % modulus;
    if (r < 0) r += modulus;
    return r;
}
