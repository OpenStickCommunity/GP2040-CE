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
    
    // Clear all masks
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
}

uint32_t SliderProfileInput::read() {
    const ProfileSliderOptions& options = Storage::getInstance().getAddonOptions().profileSliderOptions;
    Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
    
    // Check each position mask in order
    for (uint32_t i = 0; i < numPositions && i < MAX_PROFILE_SLIDER_POSITIONS; i++) {
        if (values & positionMasks[i]) {
            // Return the profile number for this position (1-indexed)
            if (i < options.profileAssignments_count) {
                return options.profileAssignments[i];
            }
            // Default to position index + 1 if no assignment exists
            return i + 1;
        }
    }
    
    // No GPIO position matched, return default profile
    if (options.defaultProfile > 0) {
        return options.defaultProfile;
    }
    // Fallback: stay at current profile
    return Storage::getInstance().GetGamepad()->getOptions().profileNumber;
}

/**
 * Reinitialize masks.
 */
void SliderProfileInput::reinit()
{
    for (int i = 0; i < MAX_PROFILE_SLIDER_POSITIONS; i++) {
        positionMasks[i] = 0;
    }
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
