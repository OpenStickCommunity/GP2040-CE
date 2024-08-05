#include "addons/togglejoystick.h"

#include "storagemanager.h"
#include "types.h"

#include "GamepadEnums.h"
#include "config.pb.h"
#include "helper.h"

bool ToggleJoystickAddon::available() {
  const ToggleJoystickOptions &options =
      Storage::getInstance().getAddonOptions().toggleJoystickOptions;
  primaryToggle = options.primaryToggle;
  secondaryToggle = options.secondaryToggle;
	GpioAction* pinMappings = Storage::getInstance().getProfilePinMappings();
	bool pinSet = false;
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		switch( pinMappings[pin] ) {
			case GpioAction::BUTTON_PRESS_TJ_PRIMARY:
			case GpioAction::BUTTON_PRESS_TJ_SECONDARY:
				pinSet = true;
			default:
				break;
		}
	}
  return options.enabled && pinSet;
}

void ToggleJoystickAddon::setup() {
	GpioAction* pinMappings = Storage::getInstance().getProfilePinMappings();
	primaryToggleMask = 0;
	secondaryToggleMask = 0;
	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		switch( pinMappings[pin] ) {
			case GpioAction::BUTTON_PRESS_TJ_PRIMARY:
				primaryToggleMask = 1 << pin;
				break;
			case GpioAction::BUTTON_PRESS_TJ_SECONDARY:
				secondaryToggleMask = 1 << pin;
				break;
			default:
				break;
		}
	}

  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  prevLX = gamepad->state.lx;
  prevLY = gamepad->state.ly;
  prevRX = gamepad->state.rx;
  prevRY = gamepad->state.ry;
  prevDpad = gamepad->state.dpad;

  defaultDpadMode = gamepad->getOptions().dpadMode;
  prevDpadMode = gamepad->getOptions().dpadMode;

  primaryState = false;
  secondaryState = false;
	onStateChange = false;
	joystickChanged = false;
	prevState = 0;
}

void ToggleJoystickAddon::update() {
  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  Mask_t allPins = gamepad->debouncedGpio;
  primaryState = allPins & primaryToggleMask;
  secondaryState = allPins & secondaryToggleMask;
}

void ToggleJoystickAddon::reinit() {
  this->setup();
}

void ToggleJoystickAddon::toggleJoystick(Gamepad *gamepad, DpadMode currentDpadMode, DpadMode toggle, bool toggleSwapped) {
	// On pin press
	if (!onStateChange) {
		onStateChange = true;
		prevDpadMode = currentDpadMode;
		defaultDpadMode = currentDpadMode;
		joystickChanged = false;
	}
	// Only toggle joystick once the direction changes
	if (!joystickChanged && ((prevDpadMode == DPAD_MODE_LEFT_ANALOG && (gamepad->state.lx != prevLX || gamepad->state.ly != prevLY)) ||
				(prevDpadMode == DPAD_MODE_RIGHT_ANALOG && (gamepad->state.rx != prevRX || gamepad->state.ry != prevRY)) ||
				(prevDpadMode == DPAD_MODE_DIGITAL && (gamepad->state.dpad != prevDpad)))
		) {
		joystickChanged = true;
		gamepad->setDpadMode(toggle);
	}
	if (joystickChanged && currentDpadMode != toggle) {
		// When swapping between toggles, prevent the previous toggle from overwriting defaultDpadMode
		if (!toggleSwapped) {
			defaultDpadMode = currentDpadMode;
		}
		gamepad->setDpadMode(toggle);
	}
	if (!joystickChanged && currentDpadMode != prevDpadMode) {
		defaultDpadMode = currentDpadMode;
		gamepad->setDpadMode(prevDpadMode);
	}

	// Set gamepad to frozen inputs
	if (prevDpadMode == DPAD_MODE_LEFT_ANALOG && toggle != DPAD_MODE_LEFT_ANALOG) {
		gamepad->state.lx = prevLX;
		gamepad->state.ly = prevLY;
	}
	else if (prevDpadMode == DPAD_MODE_RIGHT_ANALOG && toggle != DPAD_MODE_RIGHT_ANALOG) {
		gamepad->state.rx = prevRX;
		gamepad->state.ry = prevRY;
	}
	else if (prevDpadMode == DPAD_MODE_DIGITAL && toggle != DPAD_MODE_DIGITAL) {
		gamepad->state.dpad = prevDpad;
	}
}

void ToggleJoystickAddon::process() {
	update();

  Gamepad *gamepad = Storage::getInstance().GetGamepad();
	DpadMode currentDpadMode = gamepad->getOptions().dpadMode;

  if (primaryState) {
		toggleJoystick(gamepad, currentDpadMode, primaryToggle, prevState == 1);
		gamepad->state.lx = gamepad->state.lx * 0.5 - 0.5;
		gamepad->state.ly = gamepad->state.ly * 0.5 - 0.5;
		gamepad->state.rx = gamepad->state.rx * 0.5 - 0.5;
		gamepad->state.ry = gamepad->state.ry * 0.5 - 0.5;
	}
	else if (secondaryState) {
		toggleJoystick(gamepad, currentDpadMode, secondaryToggle, prevState >= 2);
  }
	else {
		// On pin release
    if (onStateChange) {
			onStateChange = false;
      gamepad->setDpadMode(defaultDpadMode);
    }
		// Save previous inputs
    prevLX = gamepad->state.lx;
    prevLY = gamepad->state.ly;
    prevRX = gamepad->state.rx;
    prevRY = gamepad->state.ry;
    prevDpad = gamepad->state.dpad;
  }
	prevState = (primaryState << 1) + secondaryState;
}
