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
    case GpioAction::BUTTON_PRESS_TJ_TOGGLE_1:
    case GpioAction::BUTTON_PRESS_TJ_TOGGLE_2:
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
  for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
    switch (pinMappings[pin]) {
    case GpioAction::BUTTON_PRESS_TJ_TOGGLE_1:
      primaryToggleMask = 1 << pin;
      break;
    case GpioAction::BUTTON_PRESS_TJ_TOGGLE_2:
      secondaryToggleMask = 1 << pin;
      break;
    default:
      break;
    }
  }
  const ToggleJoystickOptions &options =
      Storage::getInstance().getAddonOptions().toggleJoystickOptions;
  primaryToggle = options.primaryToggle;
  secondaryToggle = options.secondaryToggle;

  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  prevLX = gamepad->state.lx;
  prevLY = gamepad->state.ly;
  prevRX = gamepad->state.rx;
  prevRY = gamepad->state.ry;
  prevDpad = gamepad->state.dpad;
  prev2LX = gamepad->state.lx;
  prev2LY = gamepad->state.ly;
  prev2RX = gamepad->state.rx;
  prev2RY = gamepad->state.ry;
  prev2Dpad = gamepad->state.dpad;

  defaultDpadMode = gamepad->getOptions().dpadMode;
  prevDpadMode = gamepad->getOptions().dpadMode;

  primaryToggleState = false;
  secondaryToggleState = false;
  onStateChange = false;
  joystickChanged = false;
  holdSecondary = false;
  freezeInputs = true;

  prevToggleState = 0;
}

void ToggleJoystickAddon::update() {
  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  Mask_t allPins = gamepad->debouncedGpio;
  primaryToggleState = allPins & primaryToggleMask;
  secondaryToggleState = allPins & secondaryToggleMask;
}

void ToggleJoystickAddon::reinit() { this->setup(); }

void ToggleJoystickAddon::toggleJoystick(Gamepad *gamepad,
                                         DpadMode currentDpadMode,
                                         DpadMode toggle, bool toggleSwapped,
                                         bool bothStatesHeld) {
  // On the first change from default state to primary or secondary state
  if (!onStateChange) {
    onStateChange = true;
    prevDpadMode = currentDpadMode;
    defaultDpadMode = currentDpadMode;
    joystickChanged = false;
    // Save previous inputs
    prevLX = gamepad->state.lx;
    prevLY = gamepad->state.ly;
    prevRX = gamepad->state.rx;
    prevRY = gamepad->state.ry;
    prevDpad = gamepad->state.dpad;
  }
  // On change from secondary state to primary and secondary state
  else if (toggleSwapped && bothStatesHeld) {
    prev2LX = gamepad->state.lx;
    prev2LY = gamepad->state.ly;
    prev2RX = gamepad->state.rx;
    prev2RY = gamepad->state.ry;
    prev2Dpad = gamepad->state.dpad;
    // hold secondary input if any
    holdSecondary =
        (secondaryToggle == DPAD_MODE_LEFT_ANALOG &&
         (prevLX != prev2LX || prevLY != prev2LY)) ||
        (secondaryToggle == DPAD_MODE_RIGHT_ANALOG &&
         (prevRX != prev2RX || prevRY != prev2RY)) ||
        (secondaryToggle == DPAD_MODE_DIGITAL && (prevDpad != prev2Dpad));
    joystickChanged = holdSecondary ? false : joystickChanged;
  }
  // Release secondary frozen inputs when secondaryToggleState is false
  holdSecondary &= bothStatesHeld;
  // Only toggle joystick once the joystick has changed direction at least once
  if (!joystickChanged &&
      // joystick changes from default state
      (holdSecondary ||
       ((prevDpadMode == DPAD_MODE_LEFT_ANALOG &&
         (gamepad->state.lx != prevLX || gamepad->state.ly != prevLY)) ||
        (prevDpadMode == DPAD_MODE_RIGHT_ANALOG &&
         (gamepad->state.rx != prevRX || gamepad->state.ry != prevRY)) ||
        (prevDpadMode == DPAD_MODE_DIGITAL &&
         (gamepad->state.dpad != prevDpad)))) &&
      // joystick changes from secondary state
      (!holdSecondary ||
       ((secondaryToggle == DPAD_MODE_LEFT_ANALOG &&
         (gamepad->state.lx != prev2LX || gamepad->state.ly != prev2LY)) ||
        (secondaryToggle == DPAD_MODE_RIGHT_ANALOG &&
         (gamepad->state.rx != prev2RX || gamepad->state.ry != prev2RY)) ||
        (secondaryToggle == DPAD_MODE_DIGITAL &&
         (gamepad->state.dpad != prev2Dpad))))) {
    joystickChanged = true;
    gamepad->setDpadMode(toggle);
  }
  // If the joystick has changed direction at least once, make sure gamepad
  // dpadmode is set to toggle and set defaultDpadMode to currentDpadMode
  else if (joystickChanged && currentDpadMode != toggle) {
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
  if (!joystickChanged && !holdSecondary && currentDpadMode != prevDpadMode) {
    defaultDpadMode = currentDpadMode;
    gamepad->setDpadMode(prevDpadMode);
  } else if (!joystickChanged && holdSecondary &&
             currentDpadMode != secondaryToggle) {
    defaultDpadMode = currentDpadMode;
    gamepad->setDpadMode(secondaryToggle);
  }
}

void ToggleJoystickAddon::setPrevJoystick(Gamepad *gamepad, DpadMode toggle) {
  if (!freezeInputs)
    return;
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
  if (!holdSecondary)
    return;
  if (secondaryToggle == DPAD_MODE_LEFT_ANALOG &&
      toggle != DPAD_MODE_LEFT_ANALOG) {
    gamepad->state.lx = prev2LX;
    gamepad->state.ly = prev2LY;
  } else if (secondaryToggle == DPAD_MODE_RIGHT_ANALOG &&
             toggle != DPAD_MODE_RIGHT_ANALOG) {
    gamepad->state.rx = prev2RX;
    gamepad->state.ry = prev2RY;
  } else if (secondaryToggle == DPAD_MODE_DIGITAL &&
             toggle != DPAD_MODE_DIGITAL) {
    gamepad->state.dpad = prev2Dpad;
  }
}

void ToggleJoystickAddon::process() {
  update();

  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  DpadMode currentDpadMode = gamepad->getOptions().dpadMode;

  if (primaryToggleState) {
    toggleJoystick(gamepad, currentDpadMode, primaryToggle,
                   prevToggleState == 1, secondaryToggleState);
    setPrevJoystick(gamepad, primaryToggle);
  } else if (secondaryToggleState) {
    toggleJoystick(gamepad, currentDpadMode, secondaryToggle,
                   prevToggleState >= 2, false);
    setPrevJoystick(gamepad, secondaryToggle);
    // On pin release
  } else if (onStateChange) {
    onStateChange = false;
    gamepad->setDpadMode(defaultDpadMode);
  }
  prevToggleState = (primaryToggleState << 1) + secondaryToggleState;
}
