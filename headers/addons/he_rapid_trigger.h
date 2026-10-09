#ifndef _HE_RAPID_TRIGGER_H
#define _HE_RAPID_TRIGGER_H

#include <stdint.h>

// Rapid Trigger for analog switches. Keep in sync with www/src/Addons/HERapidTrigger.ts;
// both are checked against tests/he_rapid_trigger/vectors.txt.
// Positions are depths in ADC counts: 0 at idle, increasing as the switch is pressed.

struct HERapidTriggerConfig {
    int32_t actuation;          // depth where the key first activates
    int32_t pressSensitivity;   // downstroke distance to re-press inside the Rapid Trigger zone
    int32_t releaseSensitivity; // upstroke distance to release inside the Rapid Trigger zone
    int32_t noise;              // hysteresis around actuation and minimum sensitivity
    int32_t travel;             // calibrated idle-to-pressed distance
    bool rapidTrigger;
    bool continuous;            // stay in the Rapid Trigger zone until fully released
};

struct HERapidTriggerState {
    bool active;
    bool engaged;    // inside the Rapid Trigger zone
    int32_t extreme; // deepest point while active, shallowest while released
};

// The bottom 5% of travel (or the noise band, if larger) counts as fully released.
#define HE_RAPID_TRIGGER_RELEASED_ZONE_DIVISOR 20

static inline int32_t heRapidTriggerMax(int32_t a, int32_t b) { return a > b ? a : b; }
static inline int32_t heRapidTriggerMin(int32_t a, int32_t b) { return a < b ? a : b; }

static inline void heRapidTriggerReset(HERapidTriggerState& state) {
    state.active = false;
    state.engaged = false;
    state.extreme = 0;
}

static inline bool heRapidTriggerUpdate(HERapidTriggerState& state, const HERapidTriggerConfig& config, int32_t depth) {
    const int32_t noise = heRapidTriggerMax(config.noise, 0);
    const int32_t actuation = heRapidTriggerMax(config.actuation, 1);
    // Releasing must always be possible from idle, so hysteresis stays below the actuation point.
    const int32_t releaseBelow = actuation - heRapidTriggerMin(noise, actuation - 1);

    if (!config.rapidTrigger) {
        state.engaged = false;
        state.extreme = depth;
        state.active = state.active ? depth >= releaseBelow : depth >= actuation;
        return state.active;
    }

    if (!state.engaged) {
        state.engaged = state.active = depth >= actuation;
        state.extreme = depth;
        return state.active;
    }

    int32_t exitBelow = releaseBelow;
    if (config.continuous) {
        const int32_t releasedZone = heRapidTriggerMax(noise, heRapidTriggerMax(config.travel, 0) / HE_RAPID_TRIGGER_RELEASED_ZONE_DIVISOR);
        exitBelow = heRapidTriggerMin(releasedZone + 1, releaseBelow);
    }
    if (depth < exitBelow) {
        state.engaged = state.active = false;
        state.extreme = depth;
        return state.active;
    }

    const int32_t minSensitivity = heRapidTriggerMax(noise, 1);
    if (state.active) {
        if (depth > state.extreme) {
            state.extreme = depth;
        } else if (state.extreme - depth >= heRapidTriggerMax(config.releaseSensitivity, minSensitivity)) {
            state.active = false;
            state.extreme = depth;
        }
    } else {
        if (depth < state.extreme) {
            state.extreme = depth;
        } else if (depth - state.extreme >= heRapidTriggerMax(config.pressSensitivity, minSensitivity)) {
            state.active = true;
            state.extreme = depth;
        }
    }
    return state.active;
}

#endif // _HE_RAPID_TRIGGER_H
