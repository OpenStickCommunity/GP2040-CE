#include "addons/slider_profile.h"

#include "enums.pb.h"
#include "storagemanager.h"
#include "types.h"

bool SliderProfileInput::available() {
    const ProfileSliderOptions& options = Storage::getInstance().getAddonOptions().profileSliderOptions;
    return options.enabled;
}

void SliderProfileInput::setup()
{
    const ProfileSliderOptions& options = Storage::getInstance().getAddonOptions().profileSliderOptions;
    numPositions = options.numPositions;
    if (numPositions > MAX_PROFILE_SLIDER_POSITIONS) {
        numPositions = MAX_PROFILE_SLIDER_POSITIONS;
    }

    // Clear all masks
    anyPositionMask = 0;
    for (int i = 0; i < MAX_PROFILE_SLIDER_POSITIONS; i++) {
        positionMasks[i] = 0;
    }

    // Build masks for each position based on GPIO mappings
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch (pinMappings[pin].action) {
            case SUSTAIN_PROFILE_SLIDER_POSITION_1:     positionMasks[0] |= 1 << pin; break;
            case SUSTAIN_PROFILE_SLIDER_POSITION_2:     positionMasks[1] |= 1 << pin; break;
            case SUSTAIN_PROFILE_SLIDER_POSITION_3:     positionMasks[2] |= 1 << pin; break;
            case SUSTAIN_PROFILE_SLIDER_POSITION_4:     positionMasks[3] |= 1 << pin; break;
            case SUSTAIN_PROFILE_SLIDER_POSITION_5:     positionMasks[4] |= 1 << pin; break;
            case SUSTAIN_PROFILE_SLIDER_POSITION_6:     positionMasks[5] |= 1 << pin; break;
            case SUSTAIN_PROFILE_SLIDER_POSITION_7:     positionMasks[6] |= 1 << pin; break;
            case SUSTAIN_PROFILE_SLIDER_POSITION_8:     positionMasks[7] |= 1 << pin; break;
            default:                                    break;
        }
    }

    for (uint32_t i = 0; i < numPositions; i++) {
        anyPositionMask |= positionMasks[i];
    }
}

uint32_t SliderProfileInput::read() {
    const ProfileSliderOptions& options = Storage::getInstance().getAddonOptions().profileSliderOptions;
    uint32_t currentProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;

    // Masks are rebuilt from the active profile's pin mappings on every
    // profile change; if the new profile doesn't map any slider positions,
    // hold the current profile instead of snapping to the default profile,
    // which would oscillate between the two profiles
    if (anyPositionMask == 0) {
        return currentProfile;
    }

    Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;

    // Check each position mask in order
    for (uint32_t i = 0; i < numPositions; i++) {
        if (values & positionMasks[i]) {
            // Return the profile number for this position (1-indexed); an
            // assignment of 0 means unset, so fall back to position index + 1
            if (i < options.profileAssignments_count && options.profileAssignments[i] > 0) {
                return options.profileAssignments[i];
            }
            return i + 1;
        }
    }

    // No GPIO position matched, return default profile
    if (options.defaultProfile > 0) {
        return options.defaultProfile;
    }
    // Fallback: stay at current profile
    return currentProfile;
}

/**
 * Reinitialize masks.
 */
void SliderProfileInput::reinit()
{
    this->setup();
}

void SliderProfileInput::process()
{
    // Get Slider State
    uint32_t profileNumber = read();

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    if (gamepad->getOptions().profileNumber != profileNumber) {
        Storage::getInstance().setProfile(profileNumber);
    }
}
