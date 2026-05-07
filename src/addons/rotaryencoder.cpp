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
#include "hardware/irq.h"
#include "pico/time.h"

#include <cmath>   // std::isfinite

// Static members -------------------------------------------------------------

// Quadrature state-transition table.
// Index encoding: ((prevA << 1 | prevB) << 2) | (curA << 1 | curB), i.e.
// (prevState << 2) | curState where each "state" is the 2-bit Gray code
// formed from (A << 1) | B. Matches handleEdge() which builds lutIndex as
// (prevState << 2) | curState. Bits laid out [prevA prevB curA curB].
// +1 = valid clockwise transition, -1 = valid counter-clockwise transition,
// 0 = no change OR invalid double-edge (treated as a glitch and discarded).
const int8_t RotaryEncoderInput::QDEC_LUT[16] = {
     0, -1, +1,  0,
    +1,  0,  0, -1,
    -1,  0,  0, +1,
     0, +1, -1,  0,
};

RotaryEncoderInput* RotaryEncoderInput::instance = nullptr;

// ---------------------------------------------------------------------------

bool RotaryEncoderInput::available() {
    const RotaryOptions& options = Storage::getInstance().getAddonOptions().rotaryOptions;
    if (!options.enabled) return false;
    // Fast-path: require at least one sub-encoder plausibly configured. Full
    // validation (cross-encoder pin collisions) still happens in setup();
    // this just keeps an entirely-unconfigured addon from claiming a slot in
    // AddonManager and a no-op process() call every input tick.
    return (options.encoderOne.enabled
            && isValidPin(options.encoderOne.pinA)
            && isValidPin(options.encoderOne.pinB)
            && options.encoderOne.pinA != options.encoderOne.pinB)
        || (options.encoderTwo.enabled
            && isValidPin(options.encoderTwo.pinA)
            && isValidPin(options.encoderTwo.pinB)
            && options.encoderTwo.pinA != options.encoderTwo.pinB);
}

static inline uint8_t sampleQuadState(uint8_t pinA, uint8_t pinB) {
    return (uint8_t)((gpio_get(pinA) ? 1 : 0) << 1) | (uint8_t)(gpio_get(pinB) ? 1 : 0);
}

static inline bool isAnalogStickMode(RotaryEncoderPinMode m) {
    return m == ENCODER_MODE_LEFT_ANALOG_X  || m == ENCODER_MODE_LEFT_ANALOG_Y
        || m == ENCODER_MODE_RIGHT_ANALOG_X || m == ENCODER_MODE_RIGHT_ANALOG_Y;
}

static inline bool isAnalogTriggerMode(RotaryEncoderPinMode m) {
    return m == ENCODER_MODE_LEFT_TRIGGER || m == ENCODER_MODE_RIGHT_TRIGGER;
}

// Configuration clamps. Keep these tight enough that arithmetic in process()
// (e.g. `now + pulseHoldMs`) cannot overflow and that misconfigured values
// can't collapse stepsPerFullScale to a useless 1.
static constexpr float    MIN_MULTIPLIER     = 0.01f;
static constexpr float    MAX_MULTIPLIER     = 100.0f;
static constexpr uint32_t MAX_PULSE_HOLD_MS  = 10000;   // 10 s
static constexpr uint32_t MAX_RESET_AFTER_MS = 60000;   // 60 s

void RotaryEncoderInput::setup()
{
    const RotaryOptions& options = Storage::getInstance().getAddonOptions().rotaryOptions;
    gamepad = Storage::getInstance().GetGamepad();

    instance = this;

    // Tear down any previously-installed IRQs and disable them before mutating
    // encoderMap. The original code rewrote encoderMap and then unregistered the
    // old IRQs, which left a window where an interrupt firing on the old pins
    // would walk into a half-initialized config (and the SDK hard_asserts on
    // duplicate registration via gpio_add_raw_irq_handler).
    //
    // We keep this critical section intentionally narrow -- only the IRQ
    // teardown + encoderMap rewrite -- so that the rest of setup() (GPIO init
    // and the 50us pull-up settle wait per encoder) does not block USB / PIO /
    // other latency-sensitive ISRs on this core.
    const uint32_t edgeMask = GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL;
    {
        const uint32_t prevIrqState = save_and_disable_interrupts();
        if (irqsRegistered && registeredPinMask != 0) {
            for (uint8_t pin = 0; pin < 32; pin++) {
                if (registeredPinMask & (1u << pin)) {
                    gpio_set_irq_enabled(pin, edgeMask, false);
                }
            }
            gpio_remove_raw_irq_handler_masked(registeredPinMask, &RotaryEncoderInput::gpioIrqHandler);
            registeredPinMask = 0;
            irqsRegistered = false;
        }

        const RotaryPinOptions* perEncoder[MAX_ENCODERS] = {
            &options.encoderOne,
            &options.encoderTwo,
        };

        for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
            const RotaryPinOptions& src = *perEncoder[i];
            EncoderPinMap& dst = encoderMap[i];

            // Pin validation: must be enabled, both pins valid GPIOs on this chip,
            // pinA != pinB, and (for encoder 1) must not collide with encoder 0's pins.
            bool ok = src.enabled
                && isValidPin(src.pinA)
                && isValidPin(src.pinB)
                && (src.pinA != src.pinB);
            if (ok) {
                for (uint8_t j = 0; j < i; j++) {
                    if (!encoderMap[j].enabled) continue;
                    if (src.pinA == encoderMap[j].pinA || src.pinA == encoderMap[j].pinB ||
                        src.pinB == encoderMap[j].pinA || src.pinB == encoderMap[j].pinB) {
                        ok = false;
                        break;
                    }
                }
            }
            dst.enabled = ok;
            if (!dst.enabled) continue;

            dst.pinA = src.pinA;
            dst.pinB = src.pinB;
            dst.mode = src.mode;
            dst.pulsesPerRevolution = src.pulsesPerRevolution > 0 ? src.pulsesPerRevolution : 24;

            // Clamp resetAfter to a sane upper bound.
            dst.resetAfter = src.resetAfter > MAX_RESET_AFTER_MS ? MAX_RESET_AFTER_MS : src.resetAfter;

            dst.allowWrapAround = src.allowWrapAround;

            // Multiplier: reject NaN/inf, clamp to [MIN_MULTIPLIER, MAX_MULTIPLIER].
            // Anything outside that range collapses or explodes stepsPerFullScale.
            float mult = src.multiplier;
            if (!std::isfinite(mult) || mult <= 0.0f) {
                mult = 1.0f;
            } else if (mult < MIN_MULTIPLIER) {
                mult = MIN_MULTIPLIER;
            } else if (mult > MAX_MULTIPLIER) {
                mult = MAX_MULTIPLIER;
            }
            dst.multiplier = mult;

            // countsPerDetent: when unset (0), fall back to the encoder type default.
            // Mechanical detented EC11-style encoders typically emit 4 quadrature edges
            // per detent; optical / continuous spinners want every edge to count.
            // Only 1, 2, and 4 are valid quadrature divisors; anything else (including
            // 3) would produce uneven step sizes, so we hard-clamp to 4.
            uint32_t cpd = src.countsPerDetent;
            if (cpd == 0) {
                cpd = (src.encoderType == ENCODER_TYPE_OPTICAL_CONTINUOUS) ? 1 : 4;
            }
            dst.countsPerDetent = (cpd == 1 || cpd == 2 || cpd == 4) ? (uint8_t)cpd : 4;

            // Clamp pulseHoldMs so `now + pulseHoldMs` cannot wrap into the past.
            dst.pulseHoldMs = src.pulseHoldMs > MAX_PULSE_HOLD_MS ? MAX_PULSE_HOLD_MS : src.pulseHoldMs;

            // Default output ranges by mode.
            if (isAnalogTriggerMode(dst.mode)) {
                gamepad->hasAnalogTriggers = true;
                dst.minRange = GAMEPAD_TRIGGER_MIN;
                dst.maxRange = GAMEPAD_TRIGGER_MAX;
            } else if (isAnalogStickMode(dst.mode)) {
                dst.minRange = GAMEPAD_JOYSTICK_MIN;
                dst.maxRange = GAMEPAD_JOYSTICK_MAX;
            } else {
                dst.minRange = 0;
                dst.maxRange = 0;
            }

            // Logical steps spanning a full revolution = (raw edges per rev) / counts-per-detent.
            // For analog modes, multiplier scales how many revolutions cover the full output range.
            const int32_t edgesPerRev = (int32_t)dst.pulsesPerRevolution * 4; // 4 edges per pulse
            const int32_t stepsPerRev = edgesPerRev / dst.countsPerDetent;
            int32_t span = (int32_t)((float)stepsPerRev / dst.multiplier);
            if (span < 1) span = 1;
            // Analog stick is centered at 0 and uses [-stepsPerFullScale/2, +stepsPerFullScale/2]
            // to cover [minRange, maxRange]. To make one revolution at multiplier=1 sweep the
            // full joystick range (not half), the span needs to equal a full revolution worth
            // of steps - which means doubling the per-half count we'd otherwise compute.
            // Trigger / DPAD / volume modes don't center, so they use the raw value directly.
            if (isAnalogStickMode(dst.mode)) {
                span = (span > INT32_MAX / 2) ? INT32_MAX : span * 2;
            }
            dst.stepsPerFullScale = span;
        }

        restore_interrupts(prevIrqState);
    }

    // Configure GPIOs and enable per-pin quadrature edge interrupts. We use
    // gpio_add_raw_irq_handler (chained per-GPIO) instead of the legacy
    // single-callback gpio_set_irq_enabled_with_callback so we coexist with
    // any other GPIO IRQ consumer (e.g. the TinyUSB MAX3421 host BSP).
    //
    // GPIO init + pull-up settle wait runs with IRQs ENABLED -- the old IRQs
    // have already been removed above, and the new ones are not yet attached,
    // so there's no encoder ISR work to race with, and we avoid stalling
    // unrelated time-critical IRQs (USB, PIO, etc.) for ~50us per encoder.

    const uint32_t now = getMillis();
    uint32_t newPinMask = 0;
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

        // Give pull-ups time to settle before sampling the initial Gray state.
        busy_wait_us(50);

        // Seed runtime state. Important: changeTime must be initialized to "now"
        // so the auto-center inactivity check in process() doesn't fire on the
        // very first tick (boot can take longer than resetAfter).
        encoderState[i].prevState = sampleQuadState(pinA, pinB);
        encoderState[i].rawCounts = 0;
        encoderState[i].accumulatedSteps = 0;
        encoderState[i].prevSteps = 0;
        encoderState[i].rawRemainder = 0;
        encoderState[i].changeTime = now;
        encoderState[i].pulseDir = 0;
        encoderState[i].pulseUntil = 0;
        encoderState[i].lastValidDelta = 0;

        newPinMask |= (1u << pinA);
        newPinMask |= (1u << pinB);
    }

    // Install the IRQ handlers and enable per-pin edge interrupts under a brief
    // critical section so we never fire a partial set of handlers (which would
    // make the ISR look up a not-yet-fully-populated encoderMap entry).
    {
        const uint32_t prevIrqState = save_and_disable_interrupts();
        for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
            if (!encoderMap[i].enabled) continue;
            const uint8_t pinA = (uint8_t)encoderMap[i].pinA;
            const uint8_t pinB = (uint8_t)encoderMap[i].pinB;
            gpio_add_raw_irq_handler(pinA, &RotaryEncoderInput::gpioIrqHandler);
            gpio_add_raw_irq_handler(pinB, &RotaryEncoderInput::gpioIrqHandler);
            gpio_set_irq_enabled(pinA, edgeMask, true);
            gpio_set_irq_enabled(pinB, edgeMask, true);
        }

        registeredPinMask = newPinMask;
        irqsRegistered = (newPinMask != 0);

        if (irqsRegistered) {
            irq_set_enabled(IO_IRQ_BANK0, true);
        }
        restore_interrupts(prevIrqState);
    }
}

// Raw GPIO IRQ handler - chained per pin via gpio_add_raw_irq_handler.
// Keep this function tight: a vigorously spinning 360 P/R encoder generates
// edges every ~140 us, so we must do as little as possible per edge. The
// handler is invoked once per IRQ_BANK0 entry and must service every encoder
// pin that asserted, acknowledging each so the IRQ deasserts.
void RotaryEncoderInput::gpioIrqHandler() {
    if (instance == nullptr) return;
    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        const EncoderPinMap& m = instance->encoderMap[i];
        if (!m.enabled) continue;
        bool fired = false;

        const uint pa = (uint)m.pinA;
        const uint32_t evA = gpio_get_irq_event_mask(pa);
        if (evA) { gpio_acknowledge_irq(pa, evA); fired = true; }

        const uint pb = (uint)m.pinB;
        const uint32_t evB = gpio_get_irq_event_mask(pb);
        if (evB) { gpio_acknowledge_irq(pb, evB); fired = true; }

        if (fired) instance->handleEdge(i);
    }
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
        s.lastValidDelta = delta;
    } else {
        // delta == 0 with curState != prevState means both A and B flipped
        // between samples (a "double edge"). This typically happens when a
        // higher-priority ISR (e.g. the TinyUSB MAX3421 SPI handler) delays
        // us long enough to miss one intermediate edge.
        //
        // Previously we extrapolated the two missed edges using lastValidDelta,
        // but that injects spurious quadrature steps if the user reversed
        // direction during the missed transition. Conservatively drop the
        // step instead: at the cadence the encoder is read this is a one-step
        // jitter at worst, vs. a phantom rotation at best in the bad case.
    }
}

// preprocess() runs BEFORE Gamepad::process() so any DPAD bits we OR into
// gamepad->state.dpad participate in SOCD cleaning (e.g. simultaneous physical
// Left + rotary-driven Right gets resolved by the configured SOCD mode instead
// of leaking opposing-cardinals onto the wire).
//
// We also do the per-tick ISR snapshot + accumulator promotion here so process()
// (which runs after SOCD) can read accumulatedSteps directly when it emits
// analog stick / trigger / volume output. Snapshotting in only one of the two
// hooks is mandatory: rawCounts is zeroed on read, so a second snapshot would
// throw away ISR-collected edges that landed between preprocess() and process().
void RotaryEncoderInput::preprocess()
{
    const uint32_t now = getMillis();

    // Per-frame DPAD intent. Built up below based on currently latched pulses
    // and OR'd into gamepad->state.dpad once at the end so we don't clobber
    // any other addon's DPAD bits.
    uint8_t dpadMask = 0;

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
        if (isAnalogStickMode(m.mode) && m.resetAfter > 0 && (now - s.changeTime) >= m.resetAfter) {
            s.accumulatedSteps = 0;
            s.rawRemainder = 0;
        }

        // DPAD pulse latching happens here so the bits are visible to SOCD cleaning
        // in Gamepad::process(). Stick / trigger / volume output is deferred to
        // process() (after SOCD) because Gamepad::process() rewrites lx/ly/rx/ry
        // for DPAD_MODE_LEFT_ANALOG / RIGHT_ANALOG and would otherwise clobber us.
        if (m.mode == ENCODER_MODE_DPAD_X || m.mode == ENCODER_MODE_DPAD_Y) {
            const int32_t deltaSteps = s.accumulatedSteps - s.prevSteps;
            // Branchless sign of deltaSteps: -1, 0, or +1. Note: if multiple
            // logical steps land in a single tick we still emit just one
            // press; extra steps are intentionally absorbed into the hold.
            const int8_t pulse = (int8_t)((deltaSteps > 0) - (deltaSteps < 0));
            if (pulse != 0) {
                s.pulseDir = pulse;
                // Hold for at least pulseHoldMs (default 30 ms) so a host poll
                // is guaranteed to see the press even at 1 ms input cadence.
                s.pulseUntil = now + (m.pulseHoldMs ? m.pulseHoldMs : 1);
            }
            // Rollover-safe: (int32_t)(now - pulseUntil) < 0 means now is
            // still before pulseUntil, even across the 49.7-day uint32_t wrap.
            if (s.pulseDir != 0 && (int32_t)(now - s.pulseUntil) < 0) {
                if (m.mode == ENCODER_MODE_DPAD_X) {
                    dpadMask |= (s.pulseDir > 0) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT;
                } else {
                    dpadMask |= (s.pulseDir > 0) ? GAMEPAD_MASK_DOWN  : GAMEPAD_MASK_UP;
                }
            } else {
                s.pulseDir = 0;
            }
            // prevSteps for DPAD modes advances here; non-DPAD modes do it in process().
            s.prevSteps = s.accumulatedSteps;
        }
    }

    gamepad->state.dpad |= dpadMask;
}

void RotaryEncoderInput::process()
{
    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        EncoderPinMap& m = encoderMap[i];
        EncoderPinState& s = encoderState[i];
        if (!m.enabled) continue;

        // accumulatedSteps was already updated by preprocess() this tick.
        // DPAD modes were also fully handled there, so skip them here.
        if (m.mode == ENCODER_MODE_DPAD_X || m.mode == ENCODER_MODE_DPAD_Y) {
            continue;
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
            case ENCODER_MODE_VOLUME: {
                if (deltaSteps != 0) {
                    // Collapse a burst of N steps into a single event with magnitude N
                    // (capped) instead of allocating N events. KeyboardDriver multiplies
                    // its per-event volume key-press count by magnitude.
                    static constexpr int32_t VOLUME_BURST_CAP = 16;
                    int32_t mag = deltaSteps > 0 ? deltaSteps : -deltaSteps;
                    if (mag > VOLUME_BURST_CAP) mag = VOLUME_BURST_CAP;
                    const int8_t dir = deltaSteps > 0 ? 1 : -1;
                    EventManager::getInstance().triggerEvent(
                        new GPEncoderChangeEvent(i, dir, (uint8_t)mag));
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
}

uint16_t RotaryEncoderInput::mapEncoderValueStick(int8_t index, int32_t steps) {
    const EncoderPinMap& m = encoderMap[index];
    const int32_t span = m.stepsPerFullScale > 0 ? m.stepsPerFullScale : 1;

    if (m.allowWrapAround) {
        // Map steps into [0, span) and then linearly across the joystick range.
        // wrap and non-wrap intentionally use the same [0, span] -> [minRange, maxRange]
        // mapping; in the wrap case maxRange is unreachable by exactly one step,
        // which is the desired "rolls over instead of saturating" behavior.
        const int32_t wrapped = wrapMod(steps, span);
        return (uint16_t)map(wrapped, 0, span, m.minRange, m.maxRange);
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
        // See note in mapEncoderValueStick: wrap and non-wrap share the same
        // [0, span] -> [minRange, maxRange] mapping; maxRange unreachable by
        // one step in the wrap case is intentional.
        const int32_t wrapped = wrapMod(steps, span);
        return (uint16_t)map(wrapped, 0, span, m.minRange, m.maxRange);
    }

    int32_t mapped = map(steps, 0, span, m.minRange, m.maxRange);
    if (mapped < m.minRange) mapped = m.minRange;
    if (mapped > m.maxRange) mapped = m.maxRange;
    return (uint16_t)mapped;
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

// Public drain entry. Called from gp2040.cpp's config-mode loop so any quadrature edges
// captured by the GPIO ISR while the user is in web-config are discarded continuously
// instead of building up into a single large burst when web-config exits.
void RotaryEncoderInput::DrainPendingCounts() {
    if (instance == nullptr) return;
    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        EncoderPinMap& m = instance->encoderMap[i];
        EncoderPinState& s = instance->encoderState[i];
        if (!m.enabled) continue;
        // Snapshot+zero rawCounts under a brief IRQ disable, mirroring preprocess().
        uint32_t saved = save_and_disable_interrupts();
        s.rawCounts = 0;
        restore_interrupts(saved);
        // Do not touch accumulatedSteps / pulseDir / changeTime here; we only drop
        // the *new* edges that arrived during config mode.
    }
}

void RotaryEncoderInput::reinit() {
    // Mirror setup() so a profile change reapplies the latest RotaryOptions and re-binds
    // GPIO IRQs cleanly. setup() already handles the "previous registration present"
    // case via registeredPinMask + gpio_remove_raw_irq_handler_masked.
    setup();
}
