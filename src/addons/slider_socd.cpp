#include "addons/slider_socd.h"

#include "storagemanager.h"

#include "GamepadEnums.h"

#define SLIDERSOCD_DEBOUNCE_MILLIS 5

#define SOCD_MODE_MASK (SOCD_MODE_UP_PRIORITY & SOCD_MODE_SECOND_INPUT_PRIORITY & SOCD_MODE_NEUTRAL)

bool SliderSOCDInput::available() {
    AddonOptions options = Storage::getInstance().getAddonOptions();
    pinSliderSOCDUp = options.pinSliderSOCDUp;
    pinSliderSOCDSecond = options.pinSliderSOCDSecond;
	return ( options.SliderSOCDInputEnabled &
        pinSliderSOCDUp != (uint8_t)-1 &&
        pinSliderSOCDSecond != (uint8_t)-1);
}

void SliderSOCDInput::setup()
{
    gpio_init(pinSliderSOCDUp);             // Initialize pin
    gpio_set_dir(pinSliderSOCDUp, GPIO_IN); // Set as INPUT
    gpio_pull_up(pinSliderSOCDUp);          // Set as PULLUP
    gpio_init(pinSliderSOCDSecond);
    gpio_set_dir(pinSliderSOCDSecond, GPIO_IN); // Set as INPUT
    gpio_pull_up(pinSliderSOCDSecond);          // Set as PULLUP
}

SOCDMode SliderSOCDInput::read() {
    if ( pinSliderSOCDUp != (uint8_t)-1 && pinSliderSOCDSecond != (uint8_t)-1) {
        if ( !gpio_get(pinSliderSOCDUp)) {
            return SOCD_MODE_UP_PRIORITY;
        } else if ( !gpio_get(pinSliderSOCDSecond)) {
            return SOCD_MODE_SECOND_INPUT_PRIORITY;
        }
    }
    return  SOCD_MODE_NEUTRAL;
}

void SliderSOCDInput::debounce()
{
    uint32_t uNowTime = getMillis();
    if ((dDebState != socdState) && ((uNowTime - uDebTime) > SLIDERSOCD_DEBOUNCE_MILLIS)) {
        if ( (socdState ^ dDebState) == SOCD_MODE_SECOND_INPUT_PRIORITY )
            dDebState = (SOCDMode)(dDebState ^ SOCD_MODE_SECOND_INPUT_PRIORITY); // Bounce Second Priority
        else if ( (socdState ^ dDebState) & SOCD_MODE_UP_PRIORITY )
            dDebState = (SOCDMode)(dDebState ^ SOCD_MODE_UP_PRIORITY); // Bounce Up Priority
        else
            dDebState = (SOCDMode)(dDebState ^ SOCD_MODE_NEUTRAL); // Bounce Neutral Priority
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
    if ( gamepad->options.socdMode != socdState) {
        gamepad->options.socdMode = socdState;
        gamepad->save();
    }
}
