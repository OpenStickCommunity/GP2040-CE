#include "addons/focus_mode.h"
#include "storagemanager.h"
#include "hardware/gpio.h"

bool FocusModeAddon::available() {
	const FocusModeOptions& options = Storage::getInstance().getAddonOptions().focusModeOptions;
	return options.enabled &&
		options.buttonLockMask != 0 &&
		isValidPin(options.pin);
}

void FocusModeAddon::setup() {
	const FocusModeOptions& options = Storage::getInstance().getAddonOptions().focusModeOptions;
	buttonLockMask = options.buttonLockMask;
	focusModePin = options.pin;
	gpio_init(focusModePin);             // Initialize pin
	gpio_set_dir(focusModePin, GPIO_IN); // Set as INPUT
	gpio_pull_up(focusModePin);          // Set as PULLUP
}

void FocusModeAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
	if (!gpio_get(focusModePin)) {
			if (buttonLockMask & GAMEPAD_MASK_DU) {
				gamepad->state.dpad &= ~GAMEPAD_MASK_UP;
			}
			if (buttonLockMask & GAMEPAD_MASK_DD) {
				gamepad->state.dpad &= ~GAMEPAD_MASK_DOWN;
			}
			if (buttonLockMask & GAMEPAD_MASK_DL) {
				gamepad->state.dpad &= ~GAMEPAD_MASK_LEFT;
			}
			if (buttonLockMask & GAMEPAD_MASK_DR) {
				gamepad->state.dpad &= ~GAMEPAD_MASK_RIGHT;
			}
			gamepad->state.buttons &= ~buttonLockMask;
	}
}