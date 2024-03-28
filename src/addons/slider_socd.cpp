#include "addons/slider_socd.h"

#include "enums.pb.h"
#include "storagemanager.h"
#include "types.h"

#include "GamepadEnums.h"

#define SOCD_MODE_MASK (SOCD_MODE_UP_PRIORITY & SOCD_MODE_SECOND_INPUT_PRIORITY & SOCD_MODE_FIRST_INPUT_PRIORITY & SOCD_MODE_NEUTRAL)

bool SliderSOCDInput::available() {
    const SOCDSliderOptions& options = Storage::getInstance().getAddonOptions().socdSliderOptions;
    return options.enabled;
}

void SliderSOCDInput::setup()
{
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
    {
        switch (pinMappings[pin].action) {
            case SUSTAIN_SOCD_MODE_UP_PRIO:     upPrioModeMask |= 1 << pin; break;
            case SUSTAIN_SOCD_MODE_NEUTRAL:     neutralModeMask |= 1 << pin; break;
            case SUSTAIN_SOCD_MODE_SECOND_WIN:  secondInputModeMask |= 1 << pin; break;
            case SUSTAIN_SOCD_MODE_FIRST_WIN:   firstInputModeMask |= 1 << pin; break;
            case SUSTAIN_SOCD_MODE_BYPASS:      bypassModeMask |= 1 << pin; break;
            default:                            break;
        }
    }
}

SOCDMode SliderSOCDInput::read() {
    const SOCDSliderOptions& options = Storage::getInstance().getAddonOptions().socdSliderOptions;
    Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
    if (values & upPrioModeMask)                return SOCDMode::SOCD_MODE_UP_PRIORITY;
    else if (values & neutralModeMask)          return SOCDMode::SOCD_MODE_NEUTRAL;
    else if (values & secondInputModeMask)      return SOCDMode::SOCD_MODE_SECOND_INPUT_PRIORITY;
    else if (values & firstInputModeMask)       return SOCDMode::SOCD_MODE_FIRST_INPUT_PRIORITY;
    else if (values & bypassModeMask)           return SOCDMode::SOCD_MODE_BYPASS;
    return options.modeDefault;
}

/**
 * Reinitialize masks.
 */
void SliderSOCDInput::reinit()
{
    upPrioModeMask = 0;
    neutralModeMask = 0;
    secondInputModeMask = 0;
    firstInputModeMask = 0;
    bypassModeMask = 0;
    this->setup();
}

void SliderSOCDInput::process()
{
    // Get Slider State
    SOCDMode socdState = read();

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    if ( gamepad->getOptions().socdMode != socdState) {
        gamepad->setSOCDMode(socdState);
    }
}
