#include "addons/togglejoystick.h"

#include "storagemanager.h"
#include "types.h"

#include "GamepadEnums.h"
#include "config.pb.h"
#include "gamepad/GamepadState.h"
#include "helper.h"

bool ToggleJoystickAddon::available() {
  const ToggleJoystickOptions &options =
      Storage::getInstance().getAddonOptions().toggleJoystickOptions;
  GpioAction *pinMappings = Storage::getInstance().getProfilePinMappings();
  bool pinSet = false;
  for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
    switch (pinMappings[pin]) {
    case GpioAction::BUTTON_PRESS_TJ_PRIMARY:
    case GpioAction::BUTTON_PRESS_TJ_SECONDARY:
    case GpioAction::BUTTON_PRESS_TJ_TILT:
      pinSet = true;
    default:
      break;
    }
  }
  return options.enabled && pinSet;
}

void ToggleJoystickAddon::setup() {
  GpioAction *pinMappings = Storage::getInstance().getProfilePinMappings();
  primaryToggleMask = 0;
  secondaryToggleMask = 0;
  tiltFactorMask = 0;
  for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
    switch (pinMappings[pin]) {
    case GpioAction::BUTTON_PRESS_TJ_PRIMARY:
      primaryToggleMask = 1 << pin;
      break;
    case GpioAction::BUTTON_PRESS_TJ_SECONDARY:
      secondaryToggleMask = 1 << pin;
      break;
    case GpioAction::BUTTON_PRESS_TJ_TILT:
      tiltFactorMask = 1 << pin;
      break;
    default:
      break;
    }
  }
  const ToggleJoystickOptions &options =
      Storage::getInstance().getAddonOptions().toggleJoystickOptions;
  primaryToggle = options.primaryToggle;
  secondaryToggle = options.secondaryToggle;
	tiltFactor = options.tiltFactor;

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
	tiltState = allPins & tiltFactorMask;
}

void ToggleJoystickAddon::reinit() { this->setup(); }

void ToggleJoystickAddon::toggleJoystick(Gamepad *gamepad,
                                         DpadMode currentDpadMode,
                                         DpadMode toggle, bool toggleSwapped) {
  // On pin press
  if (!onStateChange) {
    onStateChange = true;
    prevDpadMode = currentDpadMode;
    defaultDpadMode = currentDpadMode;
    joystickChanged = false;
  }
  // Only toggle joystick once the joystick has changed direction at least once
  if (!joystickChanged &&
      ((prevDpadMode == DPAD_MODE_LEFT_ANALOG &&
        (gamepad->state.lx != prevLX || gamepad->state.ly != prevLY)) ||
       (prevDpadMode == DPAD_MODE_RIGHT_ANALOG &&
        (gamepad->state.rx != prevRX || gamepad->state.ry != prevRY)) ||
       (prevDpadMode == DPAD_MODE_DIGITAL &&
        (gamepad->state.dpad != prevDpad)))) {
    joystickChanged = true;
    gamepad->setDpadMode(toggle);
  }
  // If the joystick has changed direction at least once, make sure gamepad
  // dpadmode is set to toggle
  if (joystickChanged && currentDpadMode != toggle) {
    // When swapping between toggles, prevent the previous toggle from
    // overwriting defaultDpadMode
    if (!toggleSwapped) {
      defaultDpadMode = currentDpadMode;
    }
    gamepad->setDpadMode(toggle);
  }
  // If the joystick has not changed direction at least once and the dpad mode
  // has changed, change default dpadmode to the new one and set gamepad
  // dpadmode to prevDpadMode
  if (!joystickChanged && currentDpadMode != prevDpadMode) {
    defaultDpadMode = currentDpadMode;
    gamepad->setDpadMode(prevDpadMode);
  }
}

void ToggleJoystickAddon::tiltJoystick(Gamepad *gamepad) {
  if (tiltState) {
    gamepad->state.lx = (gamepad->state.lx * tiltFactor +
                        GAMEPAD_JOYSTICK_MID * (100 - tiltFactor)) / 100;
    gamepad->state.ly = (gamepad->state.ly * tiltFactor +
                        GAMEPAD_JOYSTICK_MID * (100 - tiltFactor)) / 100;
    gamepad->state.rx = (gamepad->state.rx * tiltFactor +
                        GAMEPAD_JOYSTICK_MID * (100 - tiltFactor)) / 100;
    gamepad->state.ry = (gamepad->state.ry * tiltFactor +
                        GAMEPAD_JOYSTICK_MID * (100 - tiltFactor)) / 100;
		// If the state is greater than GAMEPAD_JOYSTICK_MID, round up to make it symmetrical
		gamepad->state.lx += gamepad->state.lx > GAMEPAD_JOYSTICK_MID;
		gamepad->state.ly += gamepad->state.ly > GAMEPAD_JOYSTICK_MID;
		gamepad->state.rx += gamepad->state.rx > GAMEPAD_JOYSTICK_MID;
		gamepad->state.ry += gamepad->state.ry > GAMEPAD_JOYSTICK_MID;

  }
}

void ToggleJoystickAddon::setPrevJoystick(Gamepad *gamepad, DpadMode toggle) {
  // Set gamepad to frozen inputs
  if (prevDpadMode == DPAD_MODE_LEFT_ANALOG &&
      toggle != DPAD_MODE_LEFT_ANALOG) {
    gamepad->state.lx = prevLX;
    gamepad->state.ly = prevLY;
  } else if (prevDpadMode == DPAD_MODE_RIGHT_ANALOG &&
             toggle != DPAD_MODE_RIGHT_ANALOG) {
    gamepad->state.rx = prevRX;
    gamepad->state.ry = prevRY;
  } else if (prevDpadMode == DPAD_MODE_DIGITAL && toggle != DPAD_MODE_DIGITAL) {
    gamepad->state.dpad = prevDpad;
  }
}

void ToggleJoystickAddon::process() {
  update();

  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  DpadMode currentDpadMode = gamepad->getOptions().dpadMode;

  if (primaryState) {
    toggleJoystick(gamepad, currentDpadMode, primaryToggle, prevState == 1);
    tiltJoystick(gamepad);
    setPrevJoystick(gamepad, primaryToggle);
  } else if (secondaryState) {
    toggleJoystick(gamepad, currentDpadMode, secondaryToggle, prevState >= 2);
    tiltJoystick(gamepad);
    setPrevJoystick(gamepad, secondaryToggle);
  } else {
    // On pin release
    if (onStateChange) {
      onStateChange = false;
      gamepad->setDpadMode(defaultDpadMode);
    }
		tiltJoystick(gamepad);
    // Save previous inputs
    prevLX = gamepad->state.lx;
    prevLY = gamepad->state.ly;
    prevRX = gamepad->state.rx;
    prevRY = gamepad->state.ry;
    prevDpad = gamepad->state.dpad;
  }
  prevState = (primaryState << 1) + secondaryState;
}
