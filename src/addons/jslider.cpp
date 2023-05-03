#include "addons/jslider.h"

#include "storagemanager.h"

#include "GamepadEnums.h"

#define JSLIDER_DEBOUNCE_MILLIS 5

#define DPAD_MODE_MASK (DPAD_MODE_LEFT_ANALOG & DPAD_MODE_RIGHT_ANALOG & DPAD_MODE_DIGITAL)

bool JSliderInput::available() {
    const AddonOptions& options = Storage::getInstance().getAddonOptions();
    pinSliderLS = options.pinSliderLS;
    pinSliderRS = options.pinSliderRS;
	return ( options.JSliderInputEnabled &&
        pinSliderLS != (uint8_t)-1 &&
        pinSliderRS != (uint8_t)-1);
}

void JSliderInput::setup()
{
    gpio_init(pinSliderLS);             // Initialize pin
    gpio_set_dir(pinSliderLS, GPIO_IN); // Set as INPUT
    gpio_pull_up(pinSliderLS);          // Set as PULLUP
    gpio_init(pinSliderRS);
    gpio_set_dir(pinSliderRS, GPIO_IN); // Set as INPUT
    gpio_pull_up(pinSliderRS);          // Set as PULLUP
}

DpadMode JSliderInput::read() {
    if ( pinSliderLS != (uint8_t)-1 && pinSliderRS != (uint8_t)-1) {
        if ( !gpio_get(pinSliderLS)) {
            return DPAD_MODE_LEFT_ANALOG;
        } else if ( !gpio_get(pinSliderRS)) {
            return DPAD_MODE_RIGHT_ANALOG;
        }
    }
    return  DPAD_MODE_DIGITAL;
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
    if ( gamepad->options.dpadMode != dpadState) {
        gamepad->options.dpadMode = dpadState;
        gamepad->save();
    }
}
