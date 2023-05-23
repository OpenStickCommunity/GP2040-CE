#include "addons/button_lock.h"
#include "storagemanager.h"
#include "hardware/gpio.h"

bool ButtonLockAddon::available() {
	const AddonOptions& options = Storage::getInstance().getAddonOptions();
	buttonLockMap = options.buttonLockMap;
	buttonLockPin = options.buttonLockPin;
	return options.ButtonLockAddonEnabled &&
		buttonLockMap != 0 &&
		buttonLockPin != (uint8_t)-1;
}

void ButtonLockAddon::setup() {
	gpio_init(buttonLockPin);             // Initialize pin
	gpio_set_dir(buttonLockPin, GPIO_IN); // Set as INPUT
	gpio_pull_up(buttonLockPin);          // Set as PULLUP
}

void ButtonLockAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	if (!gpio_get(buttonLockPin)) {
			if (buttonLockMap & GAMEPAD_MASK_DU) {
				gamepad->state.dpad &= ~GAMEPAD_MASK_UP;
			}
			if (buttonLockMap & GAMEPAD_MASK_DD) {
				gamepad->state.dpad &= ~GAMEPAD_MASK_DOWN;
			}
			if (buttonLockMap & GAMEPAD_MASK_DL) {
				gamepad->state.dpad &= ~GAMEPAD_MASK_LEFT;
			}
			if (buttonLockMap & GAMEPAD_MASK_DR) {
				gamepad->state.dpad &= ~GAMEPAD_MASK_RIGHT;
			}
			gamepad->state.buttons &= ~buttonLockMap;
	}
}