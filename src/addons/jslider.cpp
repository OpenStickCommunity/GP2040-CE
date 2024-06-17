#include "addons/jslider.h"

#include "storagemanager.h"
#include "types.h"

#include "GamepadEnums.h"
#include "helper.h"
#include "config.pb.h"

#define DPAD_MODE_MASK (DPAD_MODE_LEFT_ANALOG & DPAD_MODE_RIGHT_ANALOG & DPAD_MODE_DIGITAL)

bool JSliderInput::available() {
    const SliderOptions& options = Storage::getInstance().getAddonOptions().sliderOptions;
    return options.enabled;
}

void JSliderInput::setup()
{
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch (pinMappings[pin].action) {
            case SUSTAIN_DP_MODE_DP:    dpModeMask |= 1 << pin; break;
            case SUSTAIN_DP_MODE_LS:    lsModeMask |= 1 << pin; break;
            case SUSTAIN_DP_MODE_RS:    rsModeMask |= 1 << pin; break;
            default:                    break;
        }
    }
}

DpadMode JSliderInput::read() {
    const SliderOptions& options = Storage::getInstance().getAddonOptions().sliderOptions;
    Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
    if (values & dpModeMask)            return DpadMode::DPAD_MODE_DIGITAL;
    else if (values & lsModeMask)       return DpadMode::DPAD_MODE_LEFT_ANALOG;
    else if (values & rsModeMask)       return DpadMode::DPAD_MODE_RIGHT_ANALOG;
    return options.modeDefault;
}

/**
 * Reinitialize masks.
 */
void JSliderInput::reinit()
{
    dpModeMask = 0;
    lsModeMask = 0;
    rsModeMask = 0;
    this->setup();
}

void JSliderInput::process()
{
    // Get Slider State
    DpadMode dpadState = read();

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    if ( gamepad->getOptions().dpadMode != dpadState) {
        gamepad->setDpadMode(dpadState);
    }
}
