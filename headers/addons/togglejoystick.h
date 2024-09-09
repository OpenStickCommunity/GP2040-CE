#ifndef _ToggleJoystick_H
#define _ToggleJoystick_H

#include "gpaddon.h"

#include "GamepadEnums.h"
#include "types.h"

#ifndef TOGGLE_JOYSTICK_ENABLED
#define TOGGLE_JOYSTICK_ENABLED 0
#endif

#ifndef TOGGLE_JOYSTICK_PRIMARY_TOGGLE
#define TOGGLE_JOYSTICK_PRIMARY_TOGGLE DPAD_MODE_RIGHT_ANALOG
#endif

#ifndef TOGGLE_JOYSTICK_SECONDARY_TOGGLE
#define TOGGLE_JOYSTICK_SECONDARY_TOGGLE DPAD_MODE_DIGITAL
#endif

// ToggleJoystick Module Name
#define ToggleJoystickName "ToggleJoystick"

class ToggleJoystickAddon : public GPAddon {
public:
  virtual bool available();
  virtual void setup();  // ToggleJoystick Setup
  virtual void update(); // ToggleJoystick Setup
  virtual void reinit();
  virtual void preprocess() {}
  virtual void process(); // ToggleJoystick process
  virtual std::string name() { return ToggleJoystickName; }

private:
  void toggleJoystick(Gamepad *gamepad, DpadMode currentDpadMode,
                      DpadMode toggle, bool toggleSwapped, bool bothStatesHeld);
  void setPrevJoystick(Gamepad *gamepad, DpadMode toggle);

  DpadMode defaultDpadMode;
  DpadMode prevDpadMode;
  DpadMode primaryToggle;
  DpadMode secondaryToggle;

  uint32_t primaryToggleMask;
  uint32_t secondaryToggleMask;
  uint32_t prevDpad;
  uint32_t prev2Dpad;

  uint16_t prevLX;
  uint16_t prevLY;
  uint16_t prevRX;
  uint16_t prevRY;
  uint16_t prev2LX;
  uint16_t prev2LY;
  uint16_t prev2RX;
  uint16_t prev2RY;

  uint8_t prevToggleState;

  bool primaryToggleState;
  bool secondaryToggleState;
  bool onStateChange;
  bool joystickChanged;
  bool holdSecondary;
  bool freezeInputs;
};

#endif // _ToggleJoystick_H_
