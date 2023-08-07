#include "addons/slider_input_mode.h"

#include "storagemanager.h"

#include "GamepadEnums.h"

#include "system.h"

#define SLIDER_INPUT_MODE_DEBOUNCE_MILLIS 50

bool SliderInputMode::available() {
    const SliderInputModeOptions& options = Storage::getInstance().getAddonOptions().sliderInputModeOptions;
		return ( options.enabled && ( isValidPin(options.pinOne) || isValidPin(options.pinTwo) ) );
}

void SliderInputMode::setup()
{
    const SliderInputModeOptions& options = Storage::getInstance().getAddonOptions().sliderInputModeOptions;
    sliderInputModeOne     = static_cast<InputMode>(options.modeOne);
    sliderInputModeTwo     = static_cast<InputMode>(options.modeTwo);
    sliderInputModeDefault = static_cast<InputMode>(options.modeDefault);
    pinSliderInputModeOne = options.pinOne;
    pinSliderInputModeTwo = options.pinTwo;

		if ( isValidPin(pinSliderInputModeOne) ) {
				gpio_init(pinSliderInputModeOne);
    		gpio_set_dir(pinSliderInputModeOne, GPIO_IN);
    		gpio_pull_up(pinSliderInputModeOne);
		}

		if ( isValidPin(pinSliderInputModeTwo) ) {
				gpio_init(pinSliderInputModeTwo);
				gpio_set_dir(pinSliderInputModeTwo, GPIO_IN);
				gpio_pull_up(pinSliderInputModeTwo);
		}
}

InputMode SliderInputMode::read() {
    if ( pinSliderInputModeOne != (uint8_t)-1 && !gpio_get(pinSliderInputModeOne) ) {
        return sliderInputModeOne;
		} else if ( pinSliderInputModeTwo != (uint8_t)-1 && !gpio_get(pinSliderInputModeTwo) ) {
			  return sliderInputModeTwo;
    } else {
				return sliderInputModeDefault;
		}
}

void SliderInputMode::debounce()
{
    uint32_t uNowTime = getMillis();
    if ((dDebState != simState) && ((uNowTime - uDebTime) > SLIDER_INPUT_MODE_DEBOUNCE_MILLIS)) {
        if ( simState == sliderInputModeTwo )
            dDebState = (InputMode)(sliderInputModeTwo); 				// Slot 2
        else if ( simState == sliderInputModeOne )
            dDebState = (InputMode)(sliderInputModeOne); 				// Slot 1
        else
            dDebState = (InputMode)(sliderInputModeDefault); 		// Default Slot
        uDebTime = uNowTime;
    }
    simState = dDebState;
}

void SliderInputMode::process()
{
    // Get Slider State
    simState = read();
#if SLIDER_INPUT_MODE_DEBOUNCE_MILLIS > 0
    debounce();
#endif
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
		if ( gamepad->getOptions().inputMode != simState ) {
				gamepad->setInputMode(simState);
				gamepad->save();

				tud_disconnect();
        sleep_ms(500);
				System::reboot(System::BootMode::GAMEPAD);
		}
}
