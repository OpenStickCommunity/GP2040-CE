#include "addons/togglejoystick.h"

#include "storagemanager.h"
#include "types.h"

#include "GamepadEnums.h"
#include "config.pb.h"
#include "helper.h"

bool ToggleJoystickAddon::available() {
  const ToggleJoystickOptions &options =
      Storage::getInstance().getAddonOptions().toggleJoystickOptions;
  primaryTogglePin = options.primaryTogglePin;
  secondaryTogglePin = options.secondaryTogglePin;
  primaryToggle = options.primaryToggle;
  secondaryToggle = options.secondaryToggle;
  // return options.enabled && isValidPin(options.primaryTogglePin);
  return options.enabled;
}

void ToggleJoystickAddon::setup() {
  gpio_init(primaryTogglePin);             // Initialize pin
  gpio_set_dir(primaryTogglePin, GPIO_IN); // Set as INPUT
  gpio_pull_up(primaryTogglePin);          // Set as PULLUP
  gpio_init(secondaryTogglePin);             // Initialize pin
  gpio_set_dir(secondaryTogglePin, GPIO_IN); // Set as INPUT
  gpio_pull_up(secondaryTogglePin);          // Set as PULLUP

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
  primaryState = !gpio_get(primaryTogglePin);
  secondaryState = !gpio_get(secondaryTogglePin);
}

/**
 * Reinitialize masks.
 */
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
