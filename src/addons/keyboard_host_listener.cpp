#include "addons/keyboard_host_listener.h"
#include "drivermanager.h"
#include "storagemanager.h"
#include "class/hid/hid_host.h"

#define DEV_ADDR_NONE 0xFF

void KeyboardHostListener::setup() {
  const KeyboardHostOptions& keyboardHostOptions = Storage::getInstance().getAddonOptions().keyboardHostOptions;
  const KeyboardMapping& keyboardMapping = keyboardHostOptions.mapping;

  _keyboard_host_mapDpadUp.setMask(GAMEPAD_MASK_UP);
  _keyboard_host_mapDpadDown.setMask(GAMEPAD_MASK_DOWN);
  _keyboard_host_mapDpadLeft.setMask(GAMEPAD_MASK_LEFT);
  _keyboard_host_mapDpadRight.setMask(GAMEPAD_MASK_RIGHT);
  _keyboard_host_mapButtonB1.setMask(GAMEPAD_MASK_B1);
  _keyboard_host_mapButtonB2.setMask(GAMEPAD_MASK_B2);
  _keyboard_host_mapButtonB3.setMask(GAMEPAD_MASK_B3);
  _keyboard_host_mapButtonB4.setMask(GAMEPAD_MASK_B4);
  _keyboard_host_mapButtonL1.setMask(GAMEPAD_MASK_L1);
  _keyboard_host_mapButtonR1.setMask(GAMEPAD_MASK_R1);
  _keyboard_host_mapButtonL2.setMask(GAMEPAD_MASK_L2);
  _keyboard_host_mapButtonR2.setMask(GAMEPAD_MASK_R2);
  _keyboard_host_mapButtonS1.setMask(GAMEPAD_MASK_S1);
  _keyboard_host_mapButtonS2.setMask(GAMEPAD_MASK_S2);
  _keyboard_host_mapButtonL3.setMask(GAMEPAD_MASK_L3);
  _keyboard_host_mapButtonR3.setMask(GAMEPAD_MASK_R3);
  _keyboard_host_mapButtonA1.setMask(GAMEPAD_MASK_A1);
  _keyboard_host_mapButtonA2.setMask(GAMEPAD_MASK_A2);
  _keyboard_host_mapDpadUp.setKey(keyboardMapping.keyDpadUp);
  _keyboard_host_mapDpadDown.setKey(keyboardMapping.keyDpadDown);
  _keyboard_host_mapDpadLeft.setKey(keyboardMapping.keyDpadLeft);
  _keyboard_host_mapDpadRight.setKey(keyboardMapping.keyDpadRight);
  _keyboard_host_mapButtonB1.setKey(keyboardMapping.keyButtonB1);
  _keyboard_host_mapButtonB2.setKey(keyboardMapping.keyButtonB2);
  _keyboard_host_mapButtonR2.setKey(keyboardMapping.keyButtonR2);
  _keyboard_host_mapButtonL2.setKey(keyboardMapping.keyButtonL2);
  _keyboard_host_mapButtonB3.setKey(keyboardMapping.keyButtonB3);
  _keyboard_host_mapButtonB4.setKey(keyboardMapping.keyButtonB4);
  _keyboard_host_mapButtonR1.setKey(keyboardMapping.keyButtonR1);
  _keyboard_host_mapButtonL1.setKey(keyboardMapping.keyButtonL1);
  _keyboard_host_mapButtonS1.setKey(keyboardMapping.keyButtonS1);
  _keyboard_host_mapButtonS2.setKey(keyboardMapping.keyButtonS2);
  _keyboard_host_mapButtonL3.setKey(keyboardMapping.keyButtonL3);
  _keyboard_host_mapButtonR3.setKey(keyboardMapping.keyButtonR3);
  _keyboard_host_mapButtonA1.setKey(keyboardMapping.keyButtonA1);
  _keyboard_host_mapButtonA2.setKey(keyboardMapping.keyButtonA2);
  _keyboard_host_mapButtonA3.setKey(keyboardMapping.keyButtonA3);
  _keyboard_host_mapButtonA4.setKey(keyboardMapping.keyButtonA4);

  mouseLeftMapping = keyboardHostOptions.mouseLeft;
  mouseMiddleMapping = keyboardHostOptions.mouseMiddle;
  mouseRightMapping = keyboardHostOptions.mouseRight;

  _keyboard_host_mounted = false;
  _keyboard_dev_addr = DEV_ADDR_NONE;
  _keyboard_instance = 0;
  
  _mouse_host_mounted = false;
  _mouse_dev_addr = DEV_ADDR_NONE;
  _mouse_instance = 0;

  mouseX = 0;
  mouseY = 0;
  mouseZ = 0;
  mouseActive = false;
}

void KeyboardHostListener::process() {
  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  if (_keyboard_host_mounted == true || _mouse_host_mounted == true) {
    gamepad->state.dpad     |= _keyboard_host_state.dpad;
    gamepad->state.buttons  |= _keyboard_host_state.buttons;
    gamepad->state.lx       |= _keyboard_host_state.lx;
    gamepad->state.ly       |= _keyboard_host_state.ly;
    gamepad->state.rx       |= _keyboard_host_state.rx;
    gamepad->state.ry       |= _keyboard_host_state.ry;
    if (!gamepad->hasAnalogTriggers) {
        gamepad->state.lt       |= _keyboard_host_state.lt;
        gamepad->state.rt       |= _keyboard_host_state.rt;
    }
  }

  if ( _mouse_host_mounted == true ) {
    gamepad->auxState.sensors.mouse.active = mouseActive;
    if ( mouseActive == true ) {
        gamepad->auxState.sensors.mouse.active = true;
        gamepad->auxState.sensors.mouse.x = mouseX;
        gamepad->auxState.sensors.mouse.y = mouseY;
        gamepad->auxState.sensors.mouse.z = mouseZ;
        mouseActive = false;
    }
  }
}

void KeyboardHostListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    // Interface protocol (hid_interface_protocol_enum_t)
    uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

    // tuh_hid_report_received_cb() will be invoked when report is available
    if (_keyboard_host_mounted == false && itf_protocol == HID_ITF_PROTOCOL_KEYBOARD) {
        _keyboard_host_mounted = true;
        _keyboard_dev_addr = dev_addr;
        _keyboard_instance = instance;
    } else if (_mouse_host_mounted == false && itf_protocol == HID_ITF_PROTOCOL_MOUSE) {
        Gamepad *gamepad = Storage::getInstance().GetGamepad();
        gamepad->auxState.sensors.mouse.enabled = true;
        _mouse_host_mounted = true;
        _mouse_dev_addr = dev_addr;
        _mouse_instance = instance;
    }
}

void KeyboardHostListener::unmount(uint8_t dev_addr) {
    if ( _keyboard_host_mounted == true && _keyboard_dev_addr == dev_addr ) {
        _keyboard_host_mounted = false;
        _keyboard_dev_addr = DEV_ADDR_NONE;
        _keyboard_instance = 0;
    } else if ( _mouse_host_mounted == true && _mouse_dev_addr == dev_addr ) {
        Gamepad *gamepad = Storage::getInstance().GetGamepad();
        gamepad->auxState.sensors.mouse.enabled = false;
        _mouse_host_mounted = false;
        _mouse_dev_addr = DEV_ADDR_NONE;
        _mouse_instance = 0;
    }
}

void KeyboardHostListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len){
  // do nothing if we haven't mounted
if ( _keyboard_host_mounted == false && _mouse_host_mounted == false )
    return;

  // tuh_hid_report_received_cb() will be invoked when report is available
  if ( _keyboard_host_mounted == true && _keyboard_dev_addr == dev_addr && _keyboard_instance == instance ) {
    process_kbd_report(dev_addr, (hid_keyboard_report_t const*) report );
  } else if ( _mouse_host_mounted == true && _mouse_dev_addr == dev_addr && _mouse_instance == instance) {
    process_mouse_report(dev_addr, (hid_mouse_report_t const*) report );
  }
}

uint8_t KeyboardHostListener::getKeycodeFromModifier(uint8_t modifier) {
	switch (modifier) {
	  case KEYBOARD_MODIFIER_LEFTCTRL   : return HID_KEY_CONTROL_LEFT ;
	  case KEYBOARD_MODIFIER_LEFTSHIFT  : return HID_KEY_SHIFT_LEFT   ;
	  case KEYBOARD_MODIFIER_LEFTALT    : return HID_KEY_ALT_LEFT     ;
	  case KEYBOARD_MODIFIER_LEFTGUI    : return HID_KEY_GUI_LEFT     ;
	  case KEYBOARD_MODIFIER_RIGHTCTRL  : return HID_KEY_CONTROL_RIGHT;
	  case KEYBOARD_MODIFIER_RIGHTSHIFT : return HID_KEY_SHIFT_RIGHT  ;
	  case KEYBOARD_MODIFIER_RIGHTALT   : return HID_KEY_ALT_RIGHT    ;
	  case KEYBOARD_MODIFIER_RIGHTGUI   : return HID_KEY_GUI_RIGHT    ;
	}

	return 0;
}

void KeyboardHostListener::preprocess_report()
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
}

// convert hid keycode to ascii and print via usb device CDC (ignore non-printable)
void KeyboardHostListener::process_kbd_report(uint8_t dev_addr, hid_keyboard_report_t const *report)
{
  preprocess_report();

  // make this 13 instead of 7 to include modifier bitfields from hid_keyboard_modifier_bm_t
  for(uint8_t i=0; i<13; i++)
  {
    uint8_t keycode = 0;
    if (i < 6) {
        // process keycodes normally
        keycode = report->keycode[i];
    } else {
        // keycode modifiers are bitfields, so the old getKeycodeFromModifier switch approach doesn't work
        // keycode = getKeycodeFromModifier(report->modifier);
        // new approach masks the modifier bit to determine which keys are pressed
        keycode = getKeycodeFromModifier(report->modifier & (1 << (i - 6)));
    }
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
          | ((keycode == _keyboard_host_mapButtonA3.key)  ? _keyboard_host_mapButtonA3.buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonA4.key)  ? _keyboard_host_mapButtonA4.buttonMask  : _keyboard_host_state.buttons)
        ;
    }
  }
}

void KeyboardHostListener::process_mouse_report(uint8_t dev_addr, hid_mouse_report_t const * report)
{
  preprocess_report();

  //------------- button state  -------------//
  _keyboard_host_state.buttons |=
      (report->buttons & MOUSE_BUTTON_LEFT   ?   mouseLeftMapping : _keyboard_host_state.buttons)
    | (report->buttons & MOUSE_BUTTON_MIDDLE ? mouseMiddleMapping : _keyboard_host_state.buttons)
    | (report->buttons & MOUSE_BUTTON_RIGHT  ?  mouseRightMapping : _keyboard_host_state.buttons)
  ;

  //------------- cursor movement -------------//
  mouseX = report->x;
  mouseY = report->y;
  mouseZ = report->wheel;
  mouseActive = true;
}
