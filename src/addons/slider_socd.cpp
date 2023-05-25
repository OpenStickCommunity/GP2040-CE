#include "addons/slider_socd.h"

#include "storagemanager.h"

#include "GamepadEnums.h"

#define SLIDERSOCD_DEBOUNCE_MILLIS 5

#define SOCD_MODE_MASK (SOCD_MODE_UP_PRIORITY & SOCD_MODE_SECOND_INPUT_PRIORITY & SOCD_MODE_FIRST_INPUT_PRIORITY & SOCD_MODE_NEUTRAL)

bool SliderSOCDInput::available() {
    const SOCDSliderOptions& options = Storage::getInstance().getAddonOptions().socdSliderOptions;
	return ( options.enabled && isValidPin(options.pinOne) && isValidPin(options.pinTwo) );
}

void SliderSOCDInput::setup()
{
    const SOCDSliderOptions& options = Storage::getInstance().getAddonOptions().socdSliderOptions;
    sliderSOCDModeOne     = static_cast<SOCDMode>(options.modeOne);
    sliderSOCDModeTwo     = static_cast<SOCDMode>(options.modeTwo);
    sliderSOCDModeDefault = static_cast<SOCDMode>(options.modeDefault);
    pinSliderSOCDOne = options.pinOne;
    pinSliderSOCDTwo = options.pinTwo;

    gpio_init(pinSliderSOCDOne);             // Initialize pin
    gpio_set_dir(pinSliderSOCDOne, GPIO_IN); // Set as INPUT
    gpio_pull_up(pinSliderSOCDOne);          // Set as PULLUP
    gpio_init(pinSliderSOCDTwo);
    gpio_set_dir(pinSliderSOCDTwo, GPIO_IN); // Set as INPUT
    gpio_pull_up(pinSliderSOCDTwo);          // Set as PULLUP
}

SOCDMode SliderSOCDInput::read() {
    if ( pinSliderSOCDOne != (uint8_t)-1 && pinSliderSOCDTwo != (uint8_t)-1) {
        if ( !gpio_get(pinSliderSOCDOne)) {
            return sliderSOCDModeOne;
        } else if ( !gpio_get(pinSliderSOCDTwo)) {
            return sliderSOCDModeTwo;
        }
    }
    return  sliderSOCDModeDefault;
}

void SliderSOCDInput::debounce()
{
    uint32_t uNowTime = getMillis();
    if ((dDebState != socdState) && ((uNowTime - uDebTime) > SLIDERSOCD_DEBOUNCE_MILLIS)) {
        if ( (socdState ^ dDebState) == sliderSOCDModeTwo )
            dDebState = (SOCDMode)(dDebState ^ sliderSOCDModeTwo); // Bounce Second Priority
        else if ( (socdState ^ dDebState) & sliderSOCDModeOne )
            dDebState = (SOCDMode)(dDebState ^ sliderSOCDModeOne); // Bounce Up Priority
        else
            dDebState = (SOCDMode)(dDebState ^ sliderSOCDModeDefault); // Bounce Neutral Priority
        uDebTime = uNowTime;
    }
    socdState = dDebState;
}

void SliderSOCDInput::process()
{
    // Get Slider State
    socdState = read();
#if SLIDERSOCD_DEBOUNCE_MILLIS > 0
    debounce();
#endif

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    if ( gamepad->getOptions().socdMode != socdState) {
        gamepad->setSOCDMode(socdState);
        gamepad->save();
    }
}
