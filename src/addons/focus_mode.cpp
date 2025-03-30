#include "addons/focus_mode.h"
#include "storagemanager.h"
#include "hardware/gpio.h"

bool FocusModeAddon::available() {
	const FocusModeOptions& options = Storage::getInstance().getAddonOptions().focusModeOptions;
	return options.enabled && options.buttonLockMask != 0;
}

void FocusModeAddon::setup() {
	const FocusModeOptions& options = Storage::getInstance().getAddonOptions().focusModeOptions;
	buttonLockMask = options.buttonLockMask;

	// Setup Focus Mode
	mapFocusMode = new GamepadButtonMapping(0);

	GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		switch (pinMappings[pin].action) {
			case GpioAction::SUSTAIN_FOCUS_MODE: mapFocusMode->pinMask |= 1 << pin; break;
			default:    break;
		}
	}

}

void FocusModeAddon::process() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
    Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;

    if (!Storage::getInstance().getAddonOptions().focusModeOptions.enabled) return;

	if (values & mapFocusMode->pinMask) {
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