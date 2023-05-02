#include "addons/extra_button.h"
#include "storagemanager.h"
#include "hardware/gpio.h"

bool ExtraButtonAddon::available() {
	const AddonOptions& options = Storage::getInstance().getAddonOptions();
	extraButtonMap = options.extraButtonMap;
	extraButtonPin = options.extraButtonPin;
	return options.ExtraButtonAddonEnabled &&
		extraButtonMap != 0 &&
		extraButtonPin != (uint8_t)-1;
}

void ExtraButtonAddon::setup() {
	gpio_init(extraButtonPin);             // Initialize pin
	gpio_set_dir(extraButtonPin, GPIO_IN); // Set as INPUT
	gpio_pull_up(extraButtonPin);          // Set as PULLUP
}

void ExtraButtonAddon::preprocess() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	if (!gpio_get(extraButtonPin)) {
		if (extraButtonMap > (GAMEPAD_MASK_A2)) {
			switch (extraButtonMap) {
				case (GAMEPAD_MASK_DU):
					gamepad->state.dpad |= GAMEPAD_MASK_UP;
					break;
				case (GAMEPAD_MASK_DD):
					gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
					break;
				case (GAMEPAD_MASK_DL):
					gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
					break;
				case (GAMEPAD_MASK_DR):
					gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
					break;
			}
		}
		else gamepad->state.buttons |= extraButtonMap;
	}
}