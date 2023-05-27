#include "addons/keyboard_host.h"
#include "storagemanager.h"

#include "pio_usb.h"

GamepadState _keyboard_host_state;

KeyboardButtonMapping *_keyboard_host_mapDpadUp;
KeyboardButtonMapping *_keyboard_host_mapDpadDown;
KeyboardButtonMapping *_keyboard_host_mapDpadLeft;
KeyboardButtonMapping *_keyboard_host_mapDpadRight;
KeyboardButtonMapping *_keyboard_host_mapButtonB1;
KeyboardButtonMapping *_keyboard_host_mapButtonB2;
KeyboardButtonMapping *_keyboard_host_mapButtonB3;
KeyboardButtonMapping *_keyboard_host_mapButtonB4;
KeyboardButtonMapping *_keyboard_host_mapButtonL1;
KeyboardButtonMapping *_keyboard_host_mapButtonR1;
KeyboardButtonMapping *_keyboard_host_mapButtonL2;
KeyboardButtonMapping *_keyboard_host_mapButtonR2;
KeyboardButtonMapping *_keyboard_host_mapButtonS1;
KeyboardButtonMapping *_keyboard_host_mapButtonS2;
KeyboardButtonMapping *_keyboard_host_mapButtonL3;
KeyboardButtonMapping *_keyboard_host_mapButtonR3;
KeyboardButtonMapping *_keyboard_host_mapButtonA1;
KeyboardButtonMapping *_keyboard_host_mapButtonA2;

bool KeyboardHostAddon::available() {
	return Storage::getInstance().getAddonOptions().keyboardHostOptions.enabled;
}

void KeyboardHostAddon::setup() {
  KeyboardMapping& keyboardMapping = Storage::getInstance().getAddonOptions().keyboardHostOptions.mapping;
	// board_init();
  // board_init() should be doing what the two lines below are doing but doesn't work
  // needs tinyusb_board library linked
	pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
	tuh_init(BOARD_TUH_RHPORT);

  _keyboard_host_mapDpadUp    = new KeyboardButtonMapping(keyboardMapping.keyDpadUp  ,  GAMEPAD_MASK_UP);
  _keyboard_host_mapDpadDown  = new KeyboardButtonMapping(keyboardMapping.keyDpadDown,  GAMEPAD_MASK_DOWN);
  _keyboard_host_mapDpadLeft  = new KeyboardButtonMapping(keyboardMapping.keyDpadLeft,  GAMEPAD_MASK_LEFT);
  _keyboard_host_mapDpadRight = new KeyboardButtonMapping(keyboardMapping.keyDpadRight,  GAMEPAD_MASK_RIGHT);
  _keyboard_host_mapButtonB1  = new KeyboardButtonMapping(keyboardMapping.keyButtonB1,  GAMEPAD_MASK_B1);
  _keyboard_host_mapButtonB2  = new KeyboardButtonMapping(keyboardMapping.keyButtonB2,  GAMEPAD_MASK_B2);
  _keyboard_host_mapButtonR2  = new KeyboardButtonMapping(keyboardMapping.keyButtonR2,  GAMEPAD_MASK_R2);
  _keyboard_host_mapButtonL2  = new KeyboardButtonMapping(keyboardMapping.keyButtonL2,  GAMEPAD_MASK_L2);
  _keyboard_host_mapButtonB3  = new KeyboardButtonMapping(keyboardMapping.keyButtonB3,  GAMEPAD_MASK_B3);
  _keyboard_host_mapButtonB4  = new KeyboardButtonMapping(keyboardMapping.keyButtonB4,  GAMEPAD_MASK_B4);
  _keyboard_host_mapButtonR1  = new KeyboardButtonMapping(keyboardMapping.keyButtonR1,  GAMEPAD_MASK_R1);
  _keyboard_host_mapButtonL1  = new KeyboardButtonMapping(keyboardMapping.keyButtonL1,  GAMEPAD_MASK_L1);
  _keyboard_host_mapButtonS1  = new KeyboardButtonMapping(keyboardMapping.keyButtonS1,  GAMEPAD_MASK_S1);
  _keyboard_host_mapButtonS2  = new KeyboardButtonMapping(keyboardMapping.keyButtonS2,  GAMEPAD_MASK_S2);
  _keyboard_host_mapButtonL3  = new KeyboardButtonMapping(keyboardMapping.keyButtonL3,  GAMEPAD_MASK_L3);
  _keyboard_host_mapButtonR3  = new KeyboardButtonMapping(keyboardMapping.keyButtonR3,  GAMEPAD_MASK_R3);
  _keyboard_host_mapButtonA1  = new KeyboardButtonMapping(keyboardMapping.keyButtonA1,  GAMEPAD_MASK_A1);
  _keyboard_host_mapButtonA2  = new KeyboardButtonMapping(keyboardMapping.keyButtonA2,  GAMEPAD_MASK_A2);
}

void KeyboardHostAddon::preprocess() {
  Gamepad *gamepad = Storage::getInstance().GetGamepad();
  gamepad->state.dpad     |= _keyboard_host_state.dpad;
  gamepad->state.buttons  |= _keyboard_host_state.buttons;
  gamepad->state.lx       |= _keyboard_host_state.lx;
  gamepad->state.ly       |= _keyboard_host_state.ly;
  gamepad->state.rx       |= _keyboard_host_state.rx;
  gamepad->state.ry       |= _keyboard_host_state.ry;
  gamepad->state.lt       |= _keyboard_host_state.lt;
  gamepad->state.rt       |= _keyboard_host_state.rt;

  tuh_task();
}

// Invoked when device with hid interface is mounted
// Report descriptor is also available for use. tuh_hid_parse_report_descriptor()
// can be used to parse common/simple enough descriptor.
// Note: if report descriptor length > CFG_TUH_ENUMERATION_BUFSIZE, it will be skipped
// therefore report_desc = NULL, desc_len = 0
void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len)
{
  (void)desc_report;
  (void)desc_len;

  // Interface protocol (hid_interface_protocol_enum_t)
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  // tuh_hid_report_received_cb() will be invoked when report is available
  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD)
  {
    if ( !tuh_hid_receive_report(dev_addr, instance) )
    {
      // Error: cannot request report
    }
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
  (void) dev_addr;
  (void) instance;
}

uint8_t getKeycodeFromModifier(uint8_t modifier) {
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

// convert hid keycode to ascii and print via usb device CDC (ignore non-printable)
void process_kbd_report(uint8_t dev_addr, hid_keyboard_report_t const *report)
{
  (void) dev_addr;

  _keyboard_host_state.dpad = 0;
  _keyboard_host_state.buttons = 0;
  _keyboard_host_state.lx = GAMEPAD_JOYSTICK_MID;
  _keyboard_host_state.ly = GAMEPAD_JOYSTICK_MID;
  _keyboard_host_state.rx = GAMEPAD_JOYSTICK_MID;
  _keyboard_host_state.ry = GAMEPAD_JOYSTICK_MID; 
  _keyboard_host_state.lt = 0;
  _keyboard_host_state.rt = 0;

  for(uint8_t i=0; i<7; i++)
  {
    uint8_t keycode = (i < 6) ? report->keycode[i] : getKeycodeFromModifier(report->modifier);
    if ( keycode )
    {
      GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();

      _keyboard_host_state.dpad |=
            ((keycode == _keyboard_host_mapDpadUp->key)    ? (gamepadOptions.invertYAxis ? _keyboard_host_mapDpadDown->buttonMask : _keyboard_host_mapDpadUp->buttonMask) : _keyboard_host_state.dpad)
          | ((keycode == _keyboard_host_mapDpadDown->key)  ? (gamepadOptions.invertYAxis ? _keyboard_host_mapDpadUp->buttonMask : _keyboard_host_mapDpadDown->buttonMask) : _keyboard_host_state.dpad)
          | ((keycode == _keyboard_host_mapDpadLeft->key)  ? _keyboard_host_mapDpadLeft->buttonMask  : _keyboard_host_state.dpad)
          | ((keycode == _keyboard_host_mapDpadRight->key) ? _keyboard_host_mapDpadRight->buttonMask : _keyboard_host_state.dpad)
        ;

        _keyboard_host_state.buttons |=
            ((keycode == _keyboard_host_mapButtonB1->key)  ? _keyboard_host_mapButtonB1->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonB2->key)  ? _keyboard_host_mapButtonB2->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonB3->key)  ? _keyboard_host_mapButtonB3->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonB4->key)  ? _keyboard_host_mapButtonB4->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonL1->key)  ? _keyboard_host_mapButtonL1->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonR1->key)  ? _keyboard_host_mapButtonR1->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonL2->key)  ? _keyboard_host_mapButtonL2->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonR2->key)  ? _keyboard_host_mapButtonR2->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonS1->key)  ? _keyboard_host_mapButtonS1->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonS2->key)  ? _keyboard_host_mapButtonS2->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonL3->key)  ? _keyboard_host_mapButtonL3->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonR3->key)  ? _keyboard_host_mapButtonR3->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonA1->key)  ? _keyboard_host_mapButtonA1->buttonMask  : _keyboard_host_state.buttons)
          | ((keycode == _keyboard_host_mapButtonA2->key)  ? _keyboard_host_mapButtonA2->buttonMask  : _keyboard_host_state.buttons)
        ;

        _keyboard_host_state.lx = GAMEPAD_JOYSTICK_MID;
        _keyboard_host_state.ly = GAMEPAD_JOYSTICK_MID;
        _keyboard_host_state.rx = GAMEPAD_JOYSTICK_MID;
        _keyboard_host_state.ry = GAMEPAD_JOYSTICK_MID;
        _keyboard_host_state.lt = 0;
        _keyboard_host_state.rt = 0;
    }
  }
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) len;
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  switch(itf_protocol)
  {
    case HID_ITF_PROTOCOL_KEYBOARD:
      process_kbd_report(dev_addr, (hid_keyboard_report_t const*) report );
    break;

    default: break;
  }

  // continue to request to receive report
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
    //Error: cannot request report
  }
}