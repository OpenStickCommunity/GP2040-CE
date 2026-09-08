#include "addons/he_trigger.h"
#include "storagemanager.h"

#include "hardware/adc.h"
#include "pico/time.h"

#define ADC_MAX ((1 << 12) - 1) // 4095

// Multiplexers and the RP2040's sample-and-hold capacitor both need a moment to
// settle after the selected channel changes. Without this the first conversion
// after a switch is a blend of the previous channel and the new one, which shows
// up as crosstalk between adjacent buttons.
#define HETRIGGER_SETTLE_US 2

HETriggerAddon* HETriggerAddon::instance = nullptr;

bool HETriggerAddon::available() {
    return Storage::getInstance().getAddonOptions().heTriggerOptions.enabled;
}

void HETriggerAddon::setup() {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;

    // How many multiplexers the channels are spread across. In direct mode each
    // trigger is wired straight to an ADC pin, so the ADC count is the limit.
    if ( options.muxChannels <= 1 ) {
        this->muxTotal = 4;
    } else {
        this->muxTotal = HETRIGGER_COUNT / options.muxChannels;
        if ( this->muxTotal > 4 )
            this->muxTotal = 4;
    }

    // Init the ADC options
    muxPinArray[0] = options.muxADCPin0;
    muxPinArray[1] = options.muxADCPin1;
    muxPinArray[2] = options.muxADCPin2;
    muxPinArray[3] = options.muxADCPin3;
    for(int i = 0; i < muxTotal; i++) {
        if ( muxPinArray[i] >= 26 && muxPinArray[i] <= 29 ) {
            adc_gpio_init(muxPinArray[i]);
        }
    }

    // Init our select pins
    switch(options.muxChannels) {
        case 4:
            this->selectPins = 2;
            break;
        case 8:
            this->selectPins = 3;
            break;
        case 16:
            this->selectPins = 4;
            break;
        case 1:
        default:
            this->selectPins = 0;
            break;
    }

    selectPinArray[0] = options.selectPin0;
    selectPinArray[1] = options.selectPin1;
    selectPinArray[2] = options.selectPin2;
    selectPinArray[3] = options.selectPin3;
    for(int i = 0; i < selectPins; i++) {
        if ( selectPinArray[i] != -1 ) {
            gpio_init(selectPinArray[i]);
            gpio_set_dir(selectPinArray[i], GPIO_OUT);
            gpio_put(selectPinArray[i], 0);
        }
    }

    lastADCSelected = -1;
    emaSmoothingFactor = (float)options.smoothingFactor / 100.f; // 99 = max smoothing factor
    activeProfile = options.activeProfile < HE_PROFILE_COUNT ? options.activeProfile : 0;

    rebuildGeometry();

    // Seed the smoothing filter and the trigger state from a real reading, so the
    // first frame does not see a step from zero. This runs regardless of whether
    // smoothing is enabled: the rapid trigger state machine needs its extrema
    // initialized either way.
    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        // Profile-control channels are driven from the base bindings, so they need
        // seeding even when the active profile leaves them unbound.
        const int32_t baseAction = options.triggers[he].action;
        const bool isProfileControl =
            (baseAction >= HE_ACTION_PROFILE_CYCLE && baseAction <= HE_ACTION_PROFILE_4);
        if (actionFor(he) == GpioAction::NONE && !isProfileControl)
            continue;

        const uint32_t channel = (options.muxChannels <= 1) ? 0 : (he % options.muxChannels);
        const uint32_t adcIndex = (options.muxChannels <= 1) ? he : (he / options.muxChannels);
        if (adcIndex >= 4) continue;
        if (muxPinArray[adcIndex] < 26 || muxPinArray[adcIndex] > 29) continue;

        if (options.muxChannels > 1) selectChannel(channel);
        if (lastADCSelected != muxPinArray[adcIndex]) {
            adc_select_input(muxPinArray[adcIndex] - 26);
            lastADCSelected = muxPinArray[adcIndex];
        }
        busy_wait_us_32(HETRIGGER_SETTLE_US);

        const uint16_t raw = adc_read();
        emaSmoothingReads[he] = raw;

        const int16_t travel = toTravel(he, raw);
        travelPeak[he] = travel;
        travelTrough[he] = travel;
        triggerActive[he] = false;
        rtArmed[he] = false;
    }
}

void HETriggerAddon::reinit() {
    // Calibration or bindings may have changed underneath us. Recompute the
    // cached geometry and drop any in-flight press state, since it was measured
    // against the old numbers.
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    emaSmoothingFactor = (float)options.smoothingFactor / 100.f;
    rebuildGeometry();
    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        triggerActive[he] = false;
        rtArmed[he] = false;
        menuActionHeld[he] = false;
        profileActionHeld[he] = false;
    }
}

// Precompute everything the per-frame path needs, so the hot loop does no
// division and no polarity branching.
void HETriggerAddon::rebuildGeometry() {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;

    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        HETriggerInfo & trigger = options.triggers[he];

        int32_t idle = trigger.idle;
        int32_t span = (int32_t)trigger.pressed - idle;

        // An uncalibrated or nonsensical span would make the travel mapping
        // meaningless (and could divide by zero), so fall back to full scale and
        // take the direction from the stored polarity flag.
        if (span > -64 && span < 64) {
            idle = trigger.is_polarized ? ADC_MAX : 0;
            span = trigger.is_polarized ? -ADC_MAX : ADC_MAX;
        }
        const int32_t absSpan = (span < 0) ? -span : span;

        travelIdle[he] = (int16_t)idle;
        // Carrying the sign of the span here is what makes a polarized switch
        // normalize to the same 0..TRAVEL_MAX range without a runtime branch.
        travelSpanRecip[he] = ((int32_t)TRAVEL_MAX << 16) / span;

        // Resolved through the active profile, which may override any of these.
        actuationTravel[he]   = (int16_t)((int32_t)tuningFor(he, HETuningField::ACTUATION_POINT) * TRAVEL_MAX / 100);
        pressSensTravel[he]   = (int16_t)((int32_t)tuningFor(he, HETuningField::RT_PRESS)        * TRAVEL_MAX / 100);
        releaseSensTravel[he] = (int16_t)((int32_t)tuningFor(he, HETuningField::RT_RELEASE)      * TRAVEL_MAX / 100);
        rapidTriggerOn[he]    = (tuningFor(he, HETuningField::RAPID_TRIGGER) != 0);
        deadzoneTravel[he]    = (int16_t)((int32_t)trigger.travelDeadzone       * TRAVEL_MAX / 100);

        // `noise` is stored in raw ADC counts; convert it into travel units so it
        // can be compared against the sensitivities.
        int32_t noise = ((int32_t)trigger.noise * TRAVEL_MAX) / absSpan;
        if (noise < 1) noise = 1;
        if (noise > 100) noise = 100;
        noiseTravel[he] = (int16_t)noise;

        // A sensitivity below the sensor's own noise floor would let noise alone
        // satisfy the press/release test, so the button would machine-gun. Clamp
        // both to the measured floor.
        if (pressSensTravel[he] < noiseTravel[he])   pressSensTravel[he] = noiseTravel[he];
        if (releaseSensTravel[he] < noiseTravel[he]) releaseSensTravel[he] = noiseTravel[he];
    }
}

// Map a raw ADC reading onto monotonic travel: 0 = fully released, TRAVEL_MAX =
// fully pressed. Polarity is handled here and only here, so every caller
// downstream can treat "bigger means pressed harder" as an invariant.
int16_t HETriggerAddon::toTravel(uint8_t he, uint16_t raw) {
    const int32_t delta = (int32_t)raw - (int32_t)travelIdle[he];
    // 64-bit intermediate: a narrow calibrated span makes the reciprocal large
    // (a 64-count span gives ~1.02e6), and a reading far outside that span would
    // overflow a 32-bit product -- which wraps negative and clamps the channel to
    // zero travel, i.e. a button that never responds.
    int32_t travel = (int32_t)(((int64_t)delta * travelSpanRecip[he]) >> 16);
    if (travel < 0) travel = 0;
    if (travel > TRAVEL_MAX) travel = TRAVEL_MAX;
    return (int16_t)travel;
}

void HETriggerAddon::selectChannel(uint8_t channel) {
    for(int i = 0; i < selectPins; i++) {
        if ( selectPinArray[i] != -1 ) {
            gpio_put(selectPinArray[i], (channel >> i) & 0x01);
        }
    }
}

uint16_t HETriggerAddon::emaSmoothing(uint16_t value, uint16_t previous) {
    // Rounded, and without the normalize-to-0..1-then-rescale round trip the
    // previous version used. That round trip truncated on every iteration, so the
    // filter settled short of a held reading and never recovered: at the default
    // smoothing factor a fully pressed switch converged about 19 counts below its
    // calibrated maximum, which is ~2% of a typical travel span permanently
    // missing from the top -- enough to stop an analog binding reaching the rail.
    const float smoothed = (emaSmoothingFactor * (float)value) +
                           ((1.0f - emaSmoothingFactor) * (float)previous);
    return (uint16_t)(smoothed + 0.5f);
}

// Resolve a channel's binding under the currently active HE profile, falling
// back to the base bindings whenever the profile data is missing or short.
int32_t HETriggerAddon::actionFor(uint8_t he) {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    const int32_t baseAction = options.triggers[he].action;

    if (activeProfile == 0) return baseAction;

    const uint8_t index = activeProfile - 1;
    // Fall back to the base bindings only when the profile has no data at all --
    // an unpopulated slot, not a deliberate choice. Once a profile is populated,
    // an unassigned channel means exactly that: leaving a button unbound in one
    // profile is a legitimate thing to want, so NONE is returned as-is.
    if (index >= options.profileSets_count) return baseAction;
    if (options.profileSets[index].actions_count == 0) return baseAction;
    if (he >= options.profileSets[index].actions_count) return GpioAction::NONE;

    return options.profileSets[index].actions[he];
}

// Per-channel tuning can be overridden per profile. 0 means "not set for this
// profile", so the base switch value is used -- the same fallback rule bindings
// follow, which is what keeps configs written before these fields existed working
// without a migration.
uint32_t HETriggerAddon::tuningFor(uint8_t he, HETuningField field) {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    HETriggerInfo & base = options.triggers[he];

    if (activeProfile != 0) {
        const uint8_t index = activeProfile - 1;
        if (index < options.profileSets_count) {
            HETriggerProfile & profile = options.profileSets[index];
            uint32_t stored = 0;
            switch (field) {
                case HETuningField::RAPID_TRIGGER:
                    if (he < profile.rapidTrigger_count) stored = profile.rapidTrigger[he];
                    // Stored as 1 = off, 2 = on, so that 0 remains "not set".
                    if (stored != 0) return (stored == 2) ? 1 : 0;
                    break;
                case HETuningField::ACTUATION_POINT:
                    if (he < profile.actuationPoint_count) stored = profile.actuationPoint[he];
                    if (stored != 0) return stored;
                    break;
                case HETuningField::RT_PRESS:
                    if (he < profile.rtPressSensitivity_count) stored = profile.rtPressSensitivity[he];
                    if (stored != 0) return stored;
                    break;
                case HETuningField::RT_RELEASE:
                    if (he < profile.rtReleaseSensitivity_count) stored = profile.rtReleaseSensitivity[he];
                    if (stored != 0) return stored;
                    break;
            }
        }
    }

    switch (field) {
        case HETuningField::RAPID_TRIGGER:  return base.rapidTrigger ? 1 : 0;
        case HETuningField::ACTUATION_POINT: return base.actuationPoint;
        case HETuningField::RT_PRESS:        return base.rtPressSensitivity;
        case HETuningField::RT_RELEASE:      return base.rtReleaseSensitivity;
    }
    return 0;
}

void HETriggerAddon::setHEProfile(uint8_t profile) {
    if (profile >= HE_PROFILE_COUNT || profile == activeProfile) return;

    activeProfile = profile;

    // Tuning can differ per profile, and rebuildGeometry caches the resolved
    // values, so the cache is stale the moment the profile changes.
    rebuildGeometry();

    // Drop all press state. Bindings just changed underneath every channel, so a
    // button held across the switch would otherwise leave the *previous*
    // profile's action latched on with no channel still bound to release it.
    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        triggerActive[he] = false;
        rtArmed[he] = false;
        menuActionHeld[he] = false;
    }

    // Remember the choice for next boot, but do not write flash yet. Storage::save
    // writes immediately -- it has no debounce of its own -- and a save erases and
    // rewrites the whole config block. Cycling profiles is a frequent in-game
    // action, so committing on every press would burn erase cycles quickly.
    // Instead mark it pending and let preprocess() commit once the user has
    // settled on a profile.
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    if (options.activeProfile != activeProfile) {
        profileSavePending = true;
        profileSaveDeadline = make_timeout_time_ms(HETRIGGER_PROFILE_SAVE_DELAY_MS);
    } else {
        // Cycled back to what is already stored; nothing to persist.
        profileSavePending = false;
    }
}

// Commit a pending profile change once the user has stopped cycling.
void HETriggerAddon::updateProfilePersistence() {
    if (!profileSavePending) return;
    if (!time_reached(profileSaveDeadline)) return;

    profileSavePending = false;

    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    if (options.activeProfile == activeProfile) return;

    options.activeProfile = activeProfile;
    options.has_activeProfile = true;
    EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(false));
}

void HETriggerAddon::cycleHEProfile() {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    const uint8_t total = 1 + options.profileSets_count;

    uint8_t next = activeProfile;
    for(uint8_t i = 0; i < total; i++) {
        next = (next + 1) % total;
        if (next == 0) break;                                 // base is always enabled
        if (options.profileSets[next - 1].enabled) break;      // skip disabled profiles
    }
    setHEProfile(next);
}

// Actuation and rapid trigger, entirely in travel space.
void HETriggerAddon::updateTrigger(uint8_t he, int16_t travel) {
    HETriggerInfo & trigger =
        Storage::getInstance().getAddonOptions().heTriggerOptions.triggers[he];

    // Hard top-out. At rest the switch is released, full stop. Resetting the
    // extrema here is what stops a slow creep from leaving a key latched, and it
    // re-zeroes the trough so the next press is measured from true rest.
    if (travel <= deadzoneTravel[he]) {
        triggerActive[he] = false;
        rtArmed[he] = false;
        travelPeak[he] = travel;
        travelTrough[he] = travel;
        return;
    }

    if (!rapidTriggerOn[he]) {
        // Plain actuation with a hysteresis band, so a switch resting exactly on
        // the actuation point does not chatter.
        const int16_t hysteresis = noiseTravel[he];
        if (!triggerActive[he]) {
            if (travel >= actuationTravel[he] + hysteresis) triggerActive[he] = true;
        } else {
            if (travel <= actuationTravel[he] - hysteresis) triggerActive[he] = false;
        }
        travelPeak[he] = travel;
        travelTrough[he] = travel;
        return;
    }

    // Until the switch has crossed the actuation point once, behave like a plain
    // threshold. This is what keeps light resting pressure near the top of travel
    // from generating input.
    if (!rtArmed[he]) {
        if (travel >= actuationTravel[he]) {
            rtArmed[he] = true;
            triggerActive[he] = true;
            travelPeak[he] = travel;
            travelTrough[he] = travel;
        } else {
            if (travel < travelTrough[he]) travelTrough[he] = travel;
            travelPeak[he] = travel;
        }
        return;
    }

    // Releasing back past the actuation point ends the rapid trigger zone, unless
    // continuous mode keeps it live all the way down to the deadzone.
    //
    // The disarm point sits one noise width *below* the actuation point rather
    // than exactly on it. Without that gap, a finger resting near the actuation
    // point makes sensor noise cross the boundary repeatedly, and each crossing
    // disarms and rearms -- which reads as the button flickering on and off.
    if (!trigger.continuousRapidTrigger && travel < actuationTravel[he] - noiseTravel[he]) {
        triggerActive[he] = false;
        rtArmed[he] = false;
        travelPeak[he] = travel;
        travelTrough[he] = travel;
        return;
    }

    // Track the local extrema. The opposite extremum is pulled along to *at most*
    // one sensitivity away rather than reset to the current position: resetting
    // outright would let the many tiny reversals in a slow, noisy press keep
    // pushing the trough up, so the press would never accumulate enough travel to
    // fire. Clamping makes micro-reversals free while genuine reversals still
    // re-datum the next movement.
    if (travel > travelPeak[he]) {
        travelPeak[he] = travel;
        if (travelPeak[he] - travelTrough[he] > releaseSensTravel[he]) {
            travelTrough[he] = travelPeak[he] - releaseSensTravel[he];
        }
    }
    if (travel < travelTrough[he]) {
        travelTrough[he] = travel;
        if (travelPeak[he] - travelTrough[he] > pressSensTravel[he]) {
            travelPeak[he] = travelTrough[he] + pressSensTravel[he];
        }
    }

    // Fire against the live extremum. This is the actual rapid trigger behaviour:
    // the press is measured from wherever the finger reversed, not from a fixed
    // depth, so a partial release followed by a partial press re-triggers.
    if (!triggerActive[he]) {
        if (travel - travelTrough[he] >= pressSensTravel[he]) {
            triggerActive[he] = true;
            travelPeak[he] = travel;
        }
    } else {
        if (travelPeak[he] - travel >= releaseSensTravel[he]) {
            triggerActive[he] = false;
            travelTrough[he] = travel;
        }
    }

    // Bottom-out. Clamp the peak so a subsequent release is measured from the true
    // bottom rather than from an overshoot.
    if (travel >= TRAVEL_MAX - deadzoneTravel[he]) {
        triggerActive[he] = true;
        travelPeak[he] = TRAVEL_MAX;
    }
}

// A channel participates in calibration if it is bound in *any* profile, so that
// switching profiles later does not reveal an uncalibrated button.
bool HETriggerAddon::isChannelAssigned(uint8_t he) {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;

    const int32_t baseAction = options.triggers[he].action;
    if (baseAction != GpioAction::NONE) return true;

    for(uint16_t p = 0; p < options.profileSets_count; p++) {
        if (he < options.profileSets[p].actions_count &&
                options.profileSets[p].actions[he] != GpioAction::NONE) {
            return true;
        }
    }
    return false;
}

void HETriggerAddon::startCalibration() {
    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        calData[he] = HECalChannel{};
    }

    // Re-read the mux/ADC pins from config. setup() cached them at boot from the
    // saved config, but the wizard pushes the current form values (which may not
    // be saved yet) before starting, so the cached copies can be stale.
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
    muxPinArray[0] = options.muxADCPin0;
    muxPinArray[1] = options.muxADCPin1;
    muxPinArray[2] = options.muxADCPin2;
    muxPinArray[3] = options.muxADCPin3;
    selectPinArray[0] = options.selectPin0;
    selectPinArray[1] = options.selectPin1;
    selectPinArray[2] = options.selectPin2;
    selectPinArray[3] = options.selectPin3;
    switch(options.muxChannels) {
        case 4:  this->selectPins = 2; break;
        case 8:  this->selectPins = 3; break;
        case 16: this->selectPins = 4; break;
        case 1:
        default: this->selectPins = 0; break;
    }
    lastADCSelected = -1;

    calMode = HECalMode::IDLE_BASELINE;
    calPhaseStart = get_absolute_time();
    calTimeout = make_timeout_time_ms(HETRIGGER_CAL_TIMEOUT_MS);

    // Release anything currently held; calibration suppresses gamepad output and
    // we do not want a stuck button surviving into the session.
    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        triggerActive[he] = false;
        rtArmed[he] = false;
        menuActionHeld[he] = false;
    }
}

void HETriggerAddon::advanceCalibration() {
    if (calMode != HECalMode::IDLE_BASELINE) return;

    // Resolve the idle statistics before switching phases. The mean is the
    // baseline; the standard deviation is the noise floor, which is needed both
    // for the deadzone and to clamp the rapid trigger sensitivities.
    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        HECalChannel & channel = calData[he];
        if (channel.sampleCount == 0) continue;

        const uint64_t n = channel.sampleCount;
        const uint64_t mean = channel.sum / n;
        channel.idleMean = (uint16_t)mean;

        // Variance via n*sum(x^2) - sum(x)^2, all in 64-bit.
        //
        // The textbook E[x^2] - E[x]^2 form is unusable here: both terms are around
        // 3.4M for a typical 1840-count baseline while their difference is only ~37,
        // and truncating the mean to an integer before squaring throws away far more
        // than that difference. Keeping the sums intact until the final division
        // avoids the cancellation -- the earlier form reported ~10x the true sigma.
        const uint64_t sumSq = channel.sumSquares;
        const uint64_t sum = channel.sum;
        uint64_t variance = 0;
        if (n > 0 && (sumSq * n) > (sum * sum)) {
            variance = ((sumSq * n) - (sum * sum)) / (n * n);
        }

        // integer square root
        uint32_t stddev = 0;
        while ((uint64_t)(stddev + 1) * (stddev + 1) <= variance) stddev++;
        channel.idleStdDev = (uint16_t)stddev;
        channel.unstable = (stddev > HETRIGGER_CAL_UNSTABLE_STDDEV);

        // Reset the accumulators; the press phase reuses them for its own counting.
        channel.sampleCount = 0;
        channel.sum = 0;
        channel.sumSquares = 0;
        channel.maxDeviation = 0;
        channel.moved = false;
    }

    calMode = HECalMode::PRESS_CAPTURE;
    calPhaseStart = get_absolute_time();
    calTimeout = make_timeout_time_ms(HETRIGGER_CAL_TIMEOUT_MS);
}

void HETriggerAddon::startMonitor() {
    // Reuse the calibration plumbing: the config-mode loop already ticks the
    // sweep, and monitoring needs the same per-channel sampling. Geometry is
    // rebuilt so the travel shown matches the currently stored calibration.
    rebuildGeometry();
    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        triggerActive[he] = false;
        rtArmed[he] = false;
        lastTravel[he] = 0;
    }
    calMode = HECalMode::MONITOR;
    calTimeout = make_timeout_time_ms(HETRIGGER_CAL_TIMEOUT_MS);
}

void HETriggerAddon::stopMonitor() {
    if (calMode != HECalMode::MONITOR) return;
    calMode = HECalMode::OFF;
    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        triggerActive[he] = false;
        rtArmed[he] = false;
    }
}

void HETriggerAddon::finishCalibration() {
    if (calMode == HECalMode::PRESS_CAPTURE) calMode = HECalMode::DONE;
}

void HETriggerAddon::abortCalibration() {
    calMode = HECalMode::OFF;
    // Restore normal operation from whatever the stored calibration says.
    reinit();
}

uint32_t HETriggerAddon::getCalibrationElapsedMs() {
    if (calMode == HECalMode::OFF) return 0;
    return (uint32_t)(absolute_time_diff_us(calPhaseStart, get_absolute_time()) / 1000);
}

void HETriggerAddon::accumulateCalibration(uint8_t he, uint16_t raw) {
    HECalChannel & channel = calData[he];

    // Smooth the *displayed* value only. The statistics below deliberately use the
    // raw reading: the variance needs independent samples to be meaningful, and
    // filtering would attenuate exactly the brief peak the press phase looks for.
    // A jittering number in the UI helps nobody, though, so the tile gets a
    // lightly filtered version instead.
    channel.lastRaw = (channel.lastRaw == 0)
                          ? raw
                          : (uint16_t)(((uint32_t)channel.lastRaw * 3 + raw) / 4);

    if (calMode == HECalMode::IDLE_BASELINE) {
        channel.sampleCount++;
        channel.sum += raw;
        channel.sumSquares += (uint64_t)raw * raw;
        return;
    }

    if (calMode == HECalMode::PRESS_CAPTURE) {
        channel.sampleCount++;
        const int32_t deviation = (int32_t)raw - (int32_t)channel.idleMean;
        const int32_t absDeviation = (deviation < 0) ? -deviation : deviation;
        const int32_t absMax = (channel.maxDeviation < 0) ? -channel.maxDeviation : channel.maxDeviation;

        // Keep the signed extreme: its sign is what tells us whether pressing
        // raises or lowers the reading, i.e. the switch polarity.
        if (absDeviation > absMax) channel.maxDeviation = deviation;

        int32_t movedThreshold = (int32_t)channel.idleStdDev * HETRIGGER_CAL_MOVED_SIGMA;
        if (movedThreshold < HETRIGGER_CAL_MOVED_FLOOR) movedThreshold = HETRIGGER_CAL_MOVED_FLOOR;
        if (absDeviation > movedThreshold) channel.moved = true;
    }
}

void HETriggerAddon::applyCalibration(uint8_t actuationPercent, uint8_t pressPercent,
                                      uint8_t releasePercent, bool continuousRT) {
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;

    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        HECalChannel & channel = calData[he];

        // Only write channels that actually moved; an untouched channel would
        // otherwise get a zero span and become permanently unusable.
        if (!channel.moved) continue;

        HETriggerInfo & trigger = options.triggers[he];

        trigger.idle = channel.idleMean;
        trigger.pressed = (int32_t)channel.idleMean + channel.maxDeviation;
        trigger.is_polarized = (channel.maxDeviation < 0);
        trigger.has_idle = true;
        trigger.has_pressed = true;
        trigger.has_is_polarized = true;

        // 4 sigma covers essentially all of the idle noise distribution.
        const int32_t noise = (int32_t)channel.idleStdDev * 4;
        trigger.noise = noise;
        trigger.has_noise = true;

        const int32_t span = (channel.maxDeviation < 0) ? -channel.maxDeviation : channel.maxDeviation;
        int32_t deadzone = (span > 0) ? ((noise * 100) / span) : 3;
        if (deadzone < 2) deadzone = 2;
        if (deadzone > 10) deadzone = 10;
        trigger.travelDeadzone = (uint32_t)deadzone;
        trigger.has_travelDeadzone = true;

        trigger.actuationPoint = actuationPercent;
        trigger.rtPressSensitivity = pressPercent;
        trigger.rtReleaseSensitivity = releasePercent;
        trigger.continuousRapidTrigger = continuousRT;
        trigger.has_actuationPoint = true;
        trigger.has_rtPressSensitivity = true;
        trigger.has_rtReleaseSensitivity = true;
        trigger.has_continuousRapidTrigger = true;

    }

    options.triggers_count = HETRIGGER_COUNT;

    calMode = HECalMode::OFF;
    rebuildGeometry();
    for(uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        triggerActive[he] = false;
        rtArmed[he] = false;
    }

    EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
}

// Sweep every assigned channel once and fold the readings into the calibration
// accumulators. This is deliberately separate from preprocess(): calibration is
// driven from the web config, and the config-mode main loop skips core0 add-ons
// entirely (see GP2040::run), so preprocess() never runs while the wizard is
// open. GP2040::run calls this directly in that mode instead.
void HETriggerAddon::runCalibrationSweep() {
    if (calMode == HECalMode::OFF) return;

    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;

    if (time_reached(calTimeout)) {
        abortCalibration();
        return;
    }

    if (calMode == HECalMode::IDLE_BASELINE &&
            getCalibrationElapsedMs() >= HETRIGGER_CAL_IDLE_MS) {
        advanceCalibration();
    }

    if (calMode == HECalMode::DONE) return;

    // Monitoring runs the real actuation logic so the test view shows exactly
    // what the gamepad would do, including rapid trigger, without emitting input.
    if (calMode == HECalMode::MONITOR) {
        HETriggerOptions & monitorOptions =
            Storage::getInstance().getAddonOptions().heTriggerOptions;
        for (uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
            if (!isChannelAssigned(he)) continue;

            const uint32_t channel  = (monitorOptions.muxChannels <= 1) ? 0 : (he % monitorOptions.muxChannels);
            const uint32_t adcIndex = (monitorOptions.muxChannels <= 1) ? he : (he / monitorOptions.muxChannels);
            if (adcIndex >= 4) continue;
            if (muxPinArray[adcIndex] < 26 || muxPinArray[adcIndex] > 29) continue;

            if (monitorOptions.muxChannels > 1) selectChannel(channel);
            if (lastADCSelected != muxPinArray[adcIndex]) {
                adc_select_input(muxPinArray[adcIndex] - 26);
                lastADCSelected = muxPinArray[adcIndex];
            }
            busy_wait_us_32(HETRIGGER_SETTLE_US);

            const uint16_t rawValue = adc_read();
            uint16_t value = rawValue;
            if (monitorOptions.emaSmoothing == 1) {
                value = emaSmoothing(value, emaSmoothingReads[he]);
                emaSmoothingReads[he] = value;
            }

            const int16_t travel = toTravel(he, value);
            lastTravel[he] = travel;
            lastRawTravel[he] = toTravel(he, rawValue);
            updateTrigger(he, travel);
        }
        return;
    }

    for (uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        if (!isChannelAssigned(he)) continue;

        const uint32_t channel  = (options.muxChannels <= 1) ? 0 : (he % options.muxChannels);
        const uint32_t adcIndex = (options.muxChannels <= 1) ? he : (he / options.muxChannels);
        if (adcIndex >= 4) continue;
        if (muxPinArray[adcIndex] < 26 || muxPinArray[adcIndex] > 29) continue;

        if (options.muxChannels > 1) selectChannel(channel);
        if (lastADCSelected != muxPinArray[adcIndex]) {
            adc_select_input(muxPinArray[adcIndex] - 26);
            lastADCSelected = muxPinArray[adcIndex];
        }
        busy_wait_us_32(HETRIGGER_SETTLE_US);

        // Deliberately unsmoothed: the EMA filter would attenuate exactly the
        // brief peak of a fast press that the capture phase is looking for.
        accumulateCalibration(he, adc_read());
    }
}

void HETriggerAddon::preprocess() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;

    // Calibration takes over the sampling loop entirely: it accumulates statistics
    // instead of producing input, so mashing buttons during the press phase cannot
    // leak into a game.
    if (calMode != HECalMode::OFF) {
        runCalibrationSweep();
        return;
    }

    for (uint8_t he = 0; he < HETRIGGER_COUNT; he++) {
        const int32_t action = actionFor(he);

        // Profile controls are resolved from the *base* bindings rather than the
        // active profile, so that a cycle button assigned in the base profile
        // keeps working even in a profile that leaves that channel unbound --
        // otherwise switching into such a profile would strand the user there.
        const int32_t baseAction = options.triggers[he].action;
        const bool isProfileControl =
            (baseAction >= HE_ACTION_PROFILE_CYCLE && baseAction <= HE_ACTION_PROFILE_4);

        // Ignore triggers with no actions
        if (action == GpioAction::NONE && !isProfileControl)
            continue;

        // In direct mode each trigger is its own ADC pin and there is no mux to
        // address; otherwise the flat index splits into mux board and channel.
        const uint32_t channel  = (options.muxChannels <= 1) ? 0 : (he % options.muxChannels);
        const uint32_t adcIndex = (options.muxChannels <= 1) ? he : (he / options.muxChannels);

        // muxPinArray only has four entries. Direct mode indexes it by `he`, which
        // runs to 31, so this guard is load-bearing and not just defensive.
        if (adcIndex >= 4) continue;
        if (muxPinArray[adcIndex] < 26 || muxPinArray[adcIndex] > 29) continue;

        if (options.muxChannels > 1) selectChannel(channel);

        // Only Switch ADC if we are not currently on the mux ADC
        if ( lastADCSelected != muxPinArray[adcIndex]) {
            adc_select_input(muxPinArray[adcIndex]-26);
            lastADCSelected = muxPinArray[adcIndex];
        }

        busy_wait_us_32(HETRIGGER_SETTLE_US);

        const uint16_t rawValue = adc_read();
        uint16_t value = rawValue;

        // Smoothing runs in raw ADC space, before the travel conversion, so that
        // the filter state and the reading it filters are always in the same units.
        if ( options.emaSmoothing == 1 ) {
            value = emaSmoothing(value, emaSmoothingReads[he]);
            emaSmoothingReads[he] = value;
        }

        const int16_t travel = toTravel(he, value);
        lastTravel[he] = travel;
        lastRawTravel[he] = toTravel(he, rawValue);

        updateTrigger(he, travel);

        if (isProfileControl) {
            if (triggerActive[he]) {
                if (!profileActionHeld[he]) {
                    profileActionHeld[he] = true;
                    if (baseAction == HE_ACTION_PROFILE_CYCLE) {
                        cycleHEProfile();
                    } else {
                        setHEProfile((uint8_t)(baseAction - HE_ACTION_PROFILE_1));
                    }
                }
            } else {
                profileActionHeld[he] = false;
            }
            continue;
        }

        if (triggerActive[he]) {
            applyAction(gamepad, he, action);
        } else {
            menuActionHeld[he] = false;
        }
    }

    updateProfilePersistence();
}

// Scales travel into a joystick axis deflection. Travel below the actuation
// point produces no deflection at all, so the stick stays centred until the
// switch would have registered as pressed; past that it ramps to full over the
// remaining travel.
uint16_t HETriggerAddon::analogDeflection(uint8_t he, bool positive) {
    const int16_t travel = lastRawTravel[he];
    const int16_t actuation = actuationTravel[he];

    // Full deflection is reached slightly before the bottom of travel. `pressed`
    // was recorded as the hardest press seen during calibration, so demanding
    // exactly that much again would mean the stick only truly maxes out when the
    // user presses at least as hard as they did that once -- in practice a normal
    // press lands around 85-95% and the diagonal corners stay unreachable. This
    // mirrors the deadzone at the top of travel.
    int32_t saturation = TRAVEL_MAX - deadzoneTravel[he];
    if (saturation <= actuation) saturation = TRAVEL_MAX;

    int32_t range = saturation - actuation;
    if (range <= 0) range = 1;

    int32_t scaled = ((int32_t)travel - actuation) * GAMEPAD_JOYSTICK_MID / range;
    if (scaled < 0) scaled = 0;
    if (scaled > GAMEPAD_JOYSTICK_MID) scaled = GAMEPAD_JOYSTICK_MID;

    return positive ? (uint16_t)(GAMEPAD_JOYSTICK_MID + scaled)
                    : (uint16_t)(GAMEPAD_JOYSTICK_MID - scaled);
}

void HETriggerAddon::applyAction(Gamepad * gamepad, uint8_t he, int32_t action) {
    // Proportional analog output: drive the axis by how far the switch is
    // pressed instead of slamming it to the rail. Flagged by an offset on the
    // action itself, so it is chosen per binding and therefore per profile.
    if (action >= HE_ANALOG_PROPORTIONAL_OFFSET) {
        switch (action - HE_ANALOG_PROPORTIONAL_OFFSET) {
            case GpioAction::ANALOG_DIRECTION_LS_X_NEG: gamepad->state.lx = analogDeflection(he, false); return;
            case GpioAction::ANALOG_DIRECTION_LS_X_POS: gamepad->state.lx = analogDeflection(he, true);  return;
            case GpioAction::ANALOG_DIRECTION_LS_Y_NEG: gamepad->state.ly = analogDeflection(he, false); return;
            case GpioAction::ANALOG_DIRECTION_LS_Y_POS: gamepad->state.ly = analogDeflection(he, true);  return;
            case GpioAction::ANALOG_DIRECTION_RS_X_NEG: gamepad->state.rx = analogDeflection(he, false); return;
            case GpioAction::ANALOG_DIRECTION_RS_X_POS: gamepad->state.rx = analogDeflection(he, true);  return;
            case GpioAction::ANALOG_DIRECTION_RS_Y_NEG: gamepad->state.ry = analogDeflection(he, false); return;
            case GpioAction::ANALOG_DIRECTION_RS_Y_POS: gamepad->state.ry = analogDeflection(he, true);  return;
            default: break;
        }
    }

    switch (action) {
        case GpioAction::BUTTON_PRESS_UP: gamepad->state.dpad |= GAMEPAD_MASK_UP; break;
        case GpioAction::BUTTON_PRESS_DOWN: gamepad->state.dpad |= GAMEPAD_MASK_DOWN; break;
        case GpioAction::BUTTON_PRESS_LEFT: gamepad->state.dpad |= GAMEPAD_MASK_LEFT; break;
        case GpioAction::BUTTON_PRESS_RIGHT: gamepad->state.dpad |= GAMEPAD_MASK_RIGHT; break;
        case GpioAction::BUTTON_PRESS_B1: gamepad->state.buttons |= GAMEPAD_MASK_B1; break;
        case GpioAction::BUTTON_PRESS_B2: gamepad->state.buttons |= GAMEPAD_MASK_B2; break;
        case GpioAction::BUTTON_PRESS_B3: gamepad->state.buttons |= GAMEPAD_MASK_B3; break;
        case GpioAction::BUTTON_PRESS_B4: gamepad->state.buttons |= GAMEPAD_MASK_B4; break;
        case GpioAction::BUTTON_PRESS_L1: gamepad->state.buttons |= GAMEPAD_MASK_L1; break;
        case GpioAction::BUTTON_PRESS_R1: gamepad->state.buttons |= GAMEPAD_MASK_R1; break;
        case GpioAction::BUTTON_PRESS_L2: gamepad->state.buttons |= GAMEPAD_MASK_L2; break;
        case GpioAction::BUTTON_PRESS_R2: gamepad->state.buttons |= GAMEPAD_MASK_R2; break;
        case GpioAction::BUTTON_PRESS_S1: gamepad->state.buttons |= GAMEPAD_MASK_S1; break;
        case GpioAction::BUTTON_PRESS_S2: gamepad->state.buttons |= GAMEPAD_MASK_S2; break;
        case GpioAction::BUTTON_PRESS_L3: gamepad->state.buttons |= GAMEPAD_MASK_L3; break;
        case GpioAction::BUTTON_PRESS_R3: gamepad->state.buttons |= GAMEPAD_MASK_R3; break;
        case GpioAction::BUTTON_PRESS_A1: gamepad->state.buttons |= GAMEPAD_MASK_A1; break;
        case GpioAction::BUTTON_PRESS_A2: gamepad->state.buttons |= GAMEPAD_MASK_A2; break;
        case GpioAction::BUTTON_PRESS_A3: gamepad->state.buttons |= GAMEPAD_MASK_A3; break;
        case GpioAction::BUTTON_PRESS_A4: gamepad->state.buttons |= GAMEPAD_MASK_A4; break;
        case GpioAction::BUTTON_PRESS_E1: gamepad->state.buttons |= GAMEPAD_MASK_E1; break;
        case GpioAction::BUTTON_PRESS_E2: gamepad->state.buttons |= GAMEPAD_MASK_E2; break;
        case GpioAction::BUTTON_PRESS_E3: gamepad->state.buttons |= GAMEPAD_MASK_E3; break;
        case GpioAction::BUTTON_PRESS_E4: gamepad->state.buttons |= GAMEPAD_MASK_E4; break;
        case GpioAction::BUTTON_PRESS_E5: gamepad->state.buttons |= GAMEPAD_MASK_E5; break;
        case GpioAction::BUTTON_PRESS_E6: gamepad->state.buttons |= GAMEPAD_MASK_E6; break;
        case GpioAction::BUTTON_PRESS_E7: gamepad->state.buttons |= GAMEPAD_MASK_E7; break;
        case GpioAction::BUTTON_PRESS_E8: gamepad->state.buttons |= GAMEPAD_MASK_E8; break;
        case GpioAction::BUTTON_PRESS_E9: gamepad->state.buttons |= GAMEPAD_MASK_E9; break;
        case GpioAction::BUTTON_PRESS_E10: gamepad->state.buttons |= GAMEPAD_MASK_E10; break;
        case GpioAction::BUTTON_PRESS_E11: gamepad->state.buttons |= GAMEPAD_MASK_E11; break;
        case GpioAction::BUTTON_PRESS_E12: gamepad->state.buttons |= GAMEPAD_MASK_E12; break;
        case GpioAction::ANALOG_DIRECTION_LS_X_NEG:	gamepad->state.lx = GAMEPAD_JOYSTICK_MIN; break;
	    case GpioAction::ANALOG_DIRECTION_LS_X_POS:	gamepad->state.lx = GAMEPAD_JOYSTICK_MAX; break;
	    case GpioAction::ANALOG_DIRECTION_LS_Y_NEG:	gamepad->state.ly = GAMEPAD_JOYSTICK_MIN; break;
	    case GpioAction::ANALOG_DIRECTION_LS_Y_POS:	gamepad->state.ly = GAMEPAD_JOYSTICK_MAX; break;
	    case GpioAction::ANALOG_DIRECTION_RS_X_NEG:	gamepad->state.rx = GAMEPAD_JOYSTICK_MIN; break;
	    case GpioAction::ANALOG_DIRECTION_RS_X_POS:	gamepad->state.rx = GAMEPAD_JOYSTICK_MAX; break;
        case GpioAction::ANALOG_DIRECTION_RS_Y_NEG:	gamepad->state.ry = GAMEPAD_JOYSTICK_MIN; break;
        case GpioAction::ANALOG_DIRECTION_RS_Y_POS:	gamepad->state.ry = GAMEPAD_JOYSTICK_MAX; break;
        case GpioAction::BUTTON_PRESS_FN:	gamepad->state.aux |= AUX_MASK_FUNCTION; break;
        // Menu navigation is edge triggered: these post an event rather than
        // setting a state bit, so firing every frame while held would flood the
        // menu with repeats.
        case GpioAction::MENU_NAVIGATION_UP:
        case GpioAction::MENU_NAVIGATION_DOWN:
        case GpioAction::MENU_NAVIGATION_LEFT:
        case GpioAction::MENU_NAVIGATION_RIGHT:
        case GpioAction::MENU_NAVIGATION_SELECT:
        case GpioAction::MENU_NAVIGATION_BACK:
        case GpioAction::MENU_NAVIGATION_TOGGLE:
            if (!menuActionHeld[he]) {
                menuActionHeld[he] = true;
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent((GpioAction)action));
            }
            break;
        default: break;
    }
}
