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

#ifndef TOGGLE_JOYSTICK_TILT_FACTOR
#define TOGGLE_JOYSTICK_TILT_FACTOR 50
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
                      DpadMode toggle, bool toggleSwapped);
  void tiltJoystick(Gamepad *gamepad);
  void setPrevJoystick(Gamepad *gamepad, DpadMode toggle);

  DpadMode defaultDpadMode;
  DpadMode prevDpadMode;
  DpadMode primaryToggle;
  DpadMode secondaryToggle;

  uint32_t primaryToggleMask;
  uint32_t secondaryToggleMask;
  uint32_t tiltFactorMask;
  uint32_t prevDpad;

  uint16_t prevLX;
  uint16_t prevLY;
  uint16_t prevRX;
  uint16_t prevRY;

  uint8_t prevState;
  uint8_t tiltFactor;

  bool primaryState;
  bool secondaryState;
	bool tiltState;
  bool onStateChange;
  bool joystickChanged;
};

#endif // _ToggleJoystick_H_
