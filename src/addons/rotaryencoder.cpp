#include "addons/rotaryencoder.h"

#include "eventmanager.h"
#include "storagemanager.h"
#include "GPEncoderEvent.h"
#include "types.h"

#include "GamepadEnums.h"
#include "helper.h"
#include "config.pb.h"
#include "peripheralmanager.h"

#include "hardware/timer.h"

// Which PIO blocks the encoders may use, decided from which features are enabled
// rather than from whatever happens to be free, so placement is the same on every
// boot. (A future PIO manager would supply this instead.)
//   PIO2 (RP2350 only) - nothing else uses it
//   PIO0               - only when USB host is off; PIO-USB takes all of PIO0
//                        later in boot, after add-ons are set up
//   PIO1               - shared with NeoPico, which hard-codes SM0 without
//                        claiming it, so SM0 is reserved here
static uint8_t buildPioCandidates(QuadratureEncoder::PioCandidate * out) {
    uint8_t count = 0;
#if NUM_PIOS > 2
    out[count++] = { pio2, 0 };
#endif
    if (!PeripheralManager::getInstance().isUSBEnabled(0)) out[count++] = { pio0, 0 };
    out[count++] = { pio1, (uint8_t)(1u << 0) };
    return count;
}

bool RotaryEncoderInput::available() {
    const RotaryOptions& options = Storage::getInstance().getAddonOptions().rotaryOptions;
    return options.enabled;
}

void RotaryEncoderInput::setup()
{
    const RotaryOptions& options = Storage::getInstance().getAddonOptions().rotaryOptions;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    encoderMap[0].enabled = options.encoderOne.enabled && ((options.encoderOne.pinA != -1) && (options.encoderOne.pinB != -1));
    if (encoderMap[0].enabled) {
        encoderMap[0].pinA = options.encoderOne.pinA;
        encoderMap[0].pinB = options.encoderOne.pinB;
        encoderMap[0].mode = options.encoderOne.mode;
        encoderMap[0].pulsesPerRevolution = options.encoderOne.pulsesPerRevolution;
        encoderMap[0].resetAfter = options.encoderOne.resetAfter;
        encoderMap[0].allowWrapAround = options.encoderOne.allowWrapAround;
        encoderMap[0].multiplier = options.encoderOne.multiplier;
    }

    encoderMap[1].enabled = options.encoderTwo.enabled && ((options.encoderTwo.pinA != -1) && (options.encoderTwo.pinB != -1));
    if (encoderMap[1].enabled) {
        encoderMap[1].pinA = options.encoderTwo.pinA;
        encoderMap[1].pinB = options.encoderTwo.pinB;
        encoderMap[1].mode = options.encoderTwo.mode;
        encoderMap[1].pulsesPerRevolution = options.encoderTwo.pulsesPerRevolution;
        encoderMap[1].resetAfter = options.encoderTwo.resetAfter;
        encoderMap[1].allowWrapAround = options.encoderTwo.allowWrapAround;
        encoderMap[1].multiplier = options.encoderTwo.multiplier;
    }

    QuadratureEncoder::PioCandidate pioCandidates[NUM_PIOS];
    uint8_t pioCandidateCount = buildPioCandidates(pioCandidates);

    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        encoderValues[i] = 0;
        prevValues[i] = 0;

        // A zero PPR would divide by zero below. The multiplier only feeds the legacy
        // step scaling, which turntable mode doesn't use.
        bool turntableMode = (encoderMap[i].mode == ENCODER_MODE_TURNTABLE);
        if (encoderMap[i].enabled && ((encoderMap[i].pulsesPerRevolution == 0) || (!turntableMode && (encoderMap[i].multiplier <= 0)))) {
            encoderMap[i].enabled = false;
        }

        if (encoderMap[i].enabled) {
            if (!turntableMode) {
                // Same values the old code computed every loop, now computed once.
                double pulsesPerUnit = encoderMap[i].pulsesPerRevolution / (ENCODER_PRECISION * encoderMap[i].multiplier);
                encoderMap[i].increment = (int32_t)(uint32_t)(ENCODER_RADIUS / pulsesPerUnit);
                encoderMap[i].totalPositions = (int32_t)(ENCODER_RADIUS * pulsesPerUnit);
            }

            // Pins (with pull-ups) are configured by QuadratureEncoder. PIO capture
            // when the pins are adjacent and a PIO/DMA slot is free, else GPIO IRQs.
            QuadratureEncoder::Config qc;
            qc.pinA = (uint8_t)encoderMap[i].pinA;
            qc.pinB = (uint8_t)encoderMap[i].pinB;
            qc.pullUp = true;
            qc.pioCandidates = pioCandidates;
            qc.pioCandidateCount = pioCandidateCount;
            qc.allowIrqFallback = true;
            qc.sampleHz = ENCODER_CAPTURE_SAMPLE_HZ;

            if (encoders[i].begin(qc)) {
                encoderState[i].lastRawCount = encoders[i].getCount();
            } else {
                encoderMap[i].enabled = false;
            }
        }

        if (encoderMap[i].enabled && turntableMode) {
            // Works on the full 4x count, independent of the legacy step scaling.
            uint32_t countsPerRev = 4u * encoderMap[i].pulsesPerRevolution;
            uint32_t threshold = (countsPerRev * ENCODER_TURNTABLE_THRESHOLD_DEGREES) / 360u;
            uint32_t holdMs = (encoderMap[i].resetAfter > 0) ? encoderMap[i].resetAfter : ENCODER_TURNTABLE_HOLD_MS;

            QuadratureTurntable::Config tc;
            tc.countsPerRevolution = countsPerRev;
            tc.unitsPerRevolution = ENCODER_TURNTABLE_POSITION_UNITS;
            tc.thresholdCounts = (threshold > 0) ? threshold : 1;
            tc.holdUs = holdMs * 1000u;
            turntables[i].begin(tc);
        }
    
        if ((encoderMap[i].mode == ENCODER_MODE_LEFT_TRIGGER) || (encoderMap[i].mode == ENCODER_MODE_RIGHT_TRIGGER)) {
            gamepad->hasAnalogTriggers = true;
            encoderMap[i].minRange = GAMEPAD_TRIGGER_MIN;
            encoderMap[i].maxRange = GAMEPAD_TRIGGER_MAX;
        } else if ((encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_X) || (encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_Y) || (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_X) || (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_Y)) {
            encoderMap[i].minRange = GAMEPAD_JOYSTICK_MIN;
            encoderMap[i].maxRange = GAMEPAD_JOYSTICK_MAX;
        }
    }
}

void RotaryEncoderInput::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    uint32_t now = getMillis();

    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        if (encoderMap[i].enabled) {
            // Every edge has already been counted; just take the delta.
            uint32_t raw = encoders[i].getCount();
            int32_t delta = (int32_t)(raw - encoderState[i].lastRawCount);
            encoderState[i].lastRawCount = raw;
            encoderState[i].lastDelta = delta;

            // Convert 4x counts to the legacy step size, carrying the remainder so
            // no counts are lost and direction changes stay exact.
            encoderState[i].pendingCounts += delta;
            int32_t steps = encoderState[i].pendingCounts / ENCODER_LEGACY_COUNT_DIVISOR;
            encoderState[i].pendingCounts -= steps * ENCODER_LEGACY_COUNT_DIVISOR;

            if (steps != 0) {
                // unsigned add: wrap rather than signed-overflow UB in wrap-around mode
                encoderValues[i] = (int32_t)((uint32_t)encoderValues[i] + (uint32_t)(steps * encoderMap[i].increment));
            }
        }
    }

    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        if (encoderMap[i].enabled && (encoderMap[i].mode != ENCODER_MODE_NONE)) {
            uint32_t lastChange = now - encoderState[i].changeTime;

            if (encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_X) {
                gamepad->state.lx = -mapEncoderValueStick(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_Y) {
                gamepad->state.ly = -mapEncoderValueStick(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_X) {
                gamepad->state.rx = -mapEncoderValueStick(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_Y) {
                gamepad->state.ry = -mapEncoderValueStick(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_LEFT_TRIGGER) {
                gamepad->state.lt = mapEncoderValueTrigger(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_RIGHT_TRIGGER) {
                gamepad->state.rt = mapEncoderValueTrigger(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_DPAD_X) {
                int8_t axis = mapEncoderValueDPad(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
                dpadLeft = (axis == 1);
                dpadRight = (axis == -1);
            } else if (encoderMap[i].mode == ENCODER_MODE_DPAD_Y) {
                int8_t axis = mapEncoderValueDPad(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
                dpadUp = (axis == 1);
                dpadDown = (axis == -1);
            } else if (encoderMap[i].mode == ENCODER_MODE_VOLUME) {
                // Prevents NONE kick-out, do nothing for now but rely on GP events
            } else if (encoderMap[i].mode == ENCODER_MODE_TURNTABLE) {
                // Updated every loop (even with no movement) so holds expire on time.
                // Not written to the gamepad state here: input modes read it through
                // getTurntable() and report it as digital or analog as they need.
                turntables[i].update(encoderState[i].lastDelta, time_us_32());
            }

            if ((encoderValues[i] - prevValues[i]) != 0) {
                encoderState[i].changeTime = now;

                if ((encoderValues[i] - prevValues[i]) > 0) {
                    EventManager::getInstance().triggerEvent(new GPEncoderChangeEvent(i, 1));
                } else if ((encoderValues[i] - prevValues[i]) < 0) {
                    EventManager::getInstance().triggerEvent(new GPEncoderChangeEvent(i, -1));
                }
            }

            if ((encoderMap[i].resetAfter > 0) && (lastChange >= encoderMap[i].resetAfter)) {
                encoderValues[i] = 0;
            }
        }

        prevValues[i] = encoderValues[i];
    }

    // only modify the dpad if needed to avoid flicker
    if (dpadUp) gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (dpadDown) gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (dpadLeft) gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (dpadRight) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
}

uint16_t RotaryEncoderInput::mapEncoderValueStick(int8_t index, int32_t encoderValue, uint16_t ppr) {
    // Total number of positions based on PPR (precomputed in setup)
    int32_t totalPositions = encoderMap[index].totalPositions;

    // Calculate range of encoder values corresponding to mapped range
    int32_t minValue = -totalPositions / 2;
    int32_t maxValue = totalPositions / 2;

    if (encoderMap[index].allowWrapAround) {
        return encoderValue;
    } else {
        int32_t mappedValue = map(encoderValue, minValue, maxValue, encoderMap[index].minRange+1, encoderMap[index].maxRange);
        int32_t constrainedValue = mappedValue;
        
        if (constrainedValue < encoderMap[index].minRange+1) constrainedValue = encoderMap[index].minRange+1;
        if (constrainedValue > encoderMap[index].maxRange) constrainedValue = encoderMap[index].maxRange;

        return constrainedValue;
    }
}

uint16_t RotaryEncoderInput::mapEncoderValueTrigger(int8_t index, int32_t encoderValue, uint16_t ppr) {
    // Total number of positions based on PPR (precomputed in setup)
    int32_t totalPositions = encoderMap[index].totalPositions;

    // Calculate range of encoder values corresponding to mapped range
    int32_t minValue = 0;
    int32_t maxValue = totalPositions;

    if (encoderMap[index].allowWrapAround) {
        return encoderValue;
    } else {
        int32_t mappedValue = map(encoderValue, minValue, maxValue, encoderMap[index].minRange, encoderMap[index].maxRange);
        int32_t constrainedValue = bounds(mappedValue, encoderMap[index].minRange, encoderMap[index].maxRange-1);

        return constrainedValue;
    }
}

int8_t RotaryEncoderInput::mapEncoderValueDPad(int8_t index, int32_t encoderValue, uint16_t ppr) {
    // Total number of positions based on PPR (precomputed in setup)
    int32_t totalPositions = encoderMap[index].totalPositions;

    // Calculate range of encoder values corresponding to mapped range
    int32_t minValue = -totalPositions / 2;
    int32_t maxValue = totalPositions / 2;

    // Calculate thresholds for left and right turns
    int32_t leftThreshold = minValue + (maxValue - minValue) / 3;
    int32_t rightThreshold = maxValue - (maxValue - minValue) / 3;

    if (encoderValue < leftThreshold) {
        return -1;
    } else if (encoderValue > rightThreshold) {
        return 1;
    } else {
        return 0;
    }
}

int32_t RotaryEncoderInput::map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    return ((int64_t)x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int32_t RotaryEncoderInput::bounds(int32_t x, int32_t out_min, int32_t out_max) {
    return (x < out_min) ? out_min : ((x > out_max) ? out_max : x);
}

int8_t RotaryEncoderInput::getEncoderIndexByPin(uint8_t pin) {
    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        if (encoderMap[i].enabled && ((encoderMap[i].pinA != -1) && (encoderMap[i].pinB != -1))) {
            if ((encoderMap[i].pinA == pin) || (encoderMap[i].pinB == pin)) return i;
        }
    }
    return -1;
}
