#include "addons/jslider.h"

#include "storagemanager.h"

#include "GamepadEnums.h"
#include "helper.h"
#include "config.pb.h"

#define JSLIDER_DEBOUNCE_MILLIS 5

#define DPAD_MODE_MASK (DPAD_MODE_LEFT_ANALOG & DPAD_MODE_RIGHT_ANALOG & DPAD_MODE_DIGITAL)

bool JSliderInput::available() {
    const SliderOptions& options = Storage::getInstance().getAddonOptions().sliderOptions;
	return ( options.enabled && (isValidPin(options.pinSliderOne) || isValidPin(options.pinSliderTwo)) );
}

void JSliderInput::setup()
{
    const SliderOptions& options = Storage::getInstance().getAddonOptions().sliderOptions;
    if ( isValidPin(options.pinSliderOne)) {
        gpio_init(options.pinSliderOne);             // Initialize pin
        gpio_set_dir(options.pinSliderOne, GPIO_IN); // Set as INPUT
        gpio_pull_up(options.pinSliderOne);          // Set as PULLUP    
    }
    if ( isValidPin(options.pinSliderTwo)) {
        gpio_init(options.pinSliderTwo);
        gpio_set_dir(options.pinSliderTwo, GPIO_IN); // Set as INPUT
        gpio_pull_up(options.pinSliderTwo);          // Set as PULLUP
    }
}

DpadMode JSliderInput::read() {
    const SliderOptions& options = Storage::getInstance().getAddonOptions().sliderOptions;
    if ( isValidPin(options.pinSliderOne) && !gpio_get(options.pinSliderOne)) {
        return options.modeOne;
    }
    if ( isValidPin(options.pinSliderTwo) && !gpio_get(options.pinSliderTwo)) {
        return options.modeTwo;
    }
    return options.modeZero;
}

void JSliderInput::debounce()
{
    uint32_t uNowTime = getMillis();
    if ((dDebState != dpadState) && ((uNowTime - uDebTime) > JSLIDER_DEBOUNCE_MILLIS)) {
        if ( (dpadState ^ dDebState) == DPAD_MODE_RIGHT_ANALOG )
            dDebState = (DpadMode)(dDebState ^ DPAD_MODE_RIGHT_ANALOG); // Bounce Right Analog
        else if ( (dpadState ^ dDebState) & DPAD_MODE_LEFT_ANALOG )
            dDebState = (DpadMode)(dDebState ^ DPAD_MODE_LEFT_ANALOG); // Bounce Left Analog
        uDebTime = uNowTime;
    }
    dpadState = dDebState;
}

void JSliderInput::process()
{
    // Get Slider State
    dpadState = read();
#if JSLIDER_DEBOUNCE_MILLIS > 0
    debounce();
#endif

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    if ( gamepad->getOptions().dpadMode != dpadState) {
        gamepad->setDpadMode(dpadState);
        gamepad->save();
    }
}
