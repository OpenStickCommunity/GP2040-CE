#include "addons/keyboard_host.h"
#include "addons/keyboard_host_listener.h"
#include "storagemanager.h"
#include "drivermanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"
#include "class/hid/hid_host.h"

bool KeyboardHostAddon::available() {
  const KeyboardHostOptions& keyboardHostOptions = Storage::getInstance().getAddonOptions().keyboardHostOptions;
	return keyboardHostOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0);
}

void KeyboardHostAddon::setup() {
  listener = new KeyboardHostListener();
}

void KeyboardHostAddon::preprocess() {
  ((KeyboardHostListener*)listener)->process();
}

// convert hid keycode to ascii and print via usb device CDC (ignore non-printable)
void KeyboardHostAddon::process_kbd_report(uint8_t dev_addr, hid_keyboard_report_t const *report)
{
  uint16_t joystickMid = GAMEPAD_JOYSTICK_MID;
	if ( DriverManager::getInstance().getDriver() != nullptr ) {
		joystickMid = DriverManager::getInstance().getDriver()->GetJoystickMidValue();
	}

  _keyboard_host_state.dpad = 0;
  _keyboard_host_state.buttons = 0;
  _keyboard_host_state.lx = joystickMid;
  _keyboard_host_state.ly = joystickMid;
  _keyboard_host_state.rx = joystickMid;
  _keyboard_host_state.ry = joystickMid;
  _keyboard_host_state.lt = 0;
  _keyboard_host_state.rt = 0;

  for(uint8_t i=0; i<7; i++)
  {
    uint8_t keycode = (i < 6) ? report->keycode[i] : getKeycodeFromModifier(report->modifier);
    if ( keycode )
    {
      _keyboard_host_state.dpad |=
            ((keycode == _keyboard_host_mapDpadUp.key)    ? _keyboard_host_mapDpadUp.buttonMask : _keyboard_host_state.dpad)
          | ((keycode == _keyboard_host_mapDpadDown.key)  ? _keyboard_host_mapDpadDown.buttonMask : _keyboard_host_state.dpad)
          | ((keycode == _keyboard_host_mapDpadLeft.key)  ? _keyboard_host_mapDpadLeft.buttonMask  : _keyboard_host_state.dpad)
          | ((keycode == _keyboard_host_mapDpadRight.key) ? _keyboard_host_mapDpadRight.buttonMask : _keyboard_host_state.dpad)
        ;

        _keyboard_host_state.buttons |=
            ((keycode == _keyboard_host_mapButtonB1.key)  ? _keyboard_host_mapButtonB1.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonB2.key)  ? _keyboard_host_mapButtonB2.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonB3.key)  ? _keyboard_host_mapButtonB3.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonB4.key)  ? _keyboard_host_mapButtonB4.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonL1.key)  ? _keyboard_host_mapButtonL1.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonR1.key)  ? _keyboard_host_mapButtonR1.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonL2.key)  ? _keyboard_host_mapButtonL2.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonR2.key)  ? _keyboard_host_mapButtonR2.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonS1.key)  ? _keyboard_host_mapButtonS1.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonS2.key)  ? _keyboard_host_mapButtonS2.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonL3.key)  ? _keyboard_host_mapButtonL3.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonR3.key)  ? _keyboard_host_mapButtonR3.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonA1.key)  ? _keyboard_host_mapButtonA1.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonA2.key)  ? _keyboard_host_mapButtonA2.buttonMask  : _keyboard_host_state.buttons)
        ;
    }
  }
}
