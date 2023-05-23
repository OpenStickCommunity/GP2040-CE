#include "addons/keyboard_host.h"
#include "storagemanager.h"
#include "hardware/gpio.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "bsp/board.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "pio_usb.h"

#include "tusb.h"

static hid_keyboard_report_t prev_report = { 0, 0, {0} }; // previous report to check key released
GamepadState _keyboard_host_state;
GamepadState _keyboard_host_state_temp;

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
	return true;
}

void KeyboardHostAddon::setup() {
  Gamepad *gamepad = Storage::getInstance().GetGamepad();
	// sleep_ms(10);
	// // board_init();
  // board_init() should be doing what the two lines below are doing but doesn't work
  // needs tinyusb_board library linked
	pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
  tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
	bool boardInit = tuh_init(BOARD_TUH_RHPORT);
	printf("Board init %d\n", boardInit);

  _keyboard_host_mapDpadUp    = new KeyboardButtonMapping(gamepad->options.keyDpadUp  ,  GAMEPAD_MASK_UP);
  _keyboard_host_mapDpadDown  = new KeyboardButtonMapping(gamepad->options.keyDpadDown,  GAMEPAD_MASK_DOWN);
  _keyboard_host_mapDpadLeft  = new KeyboardButtonMapping(gamepad->options.keyDpadLeft,  GAMEPAD_MASK_LEFT);
  _keyboard_host_mapDpadRight = new KeyboardButtonMapping(gamepad->options.keyDpadRight,  GAMEPAD_MASK_RIGHT);
  _keyboard_host_mapButtonB1  = new KeyboardButtonMapping(gamepad->options.keyButtonB1,  GAMEPAD_MASK_B1);
  _keyboard_host_mapButtonB2  = new KeyboardButtonMapping(gamepad->options.keyButtonB2,  GAMEPAD_MASK_B2);
  _keyboard_host_mapButtonR2  = new KeyboardButtonMapping(gamepad->options.keyButtonR2,  GAMEPAD_MASK_R2);
  _keyboard_host_mapButtonL2  = new KeyboardButtonMapping(gamepad->options.keyButtonL2,  GAMEPAD_MASK_L2);
  _keyboard_host_mapButtonB3  = new KeyboardButtonMapping(gamepad->options.keyButtonB3,  GAMEPAD_MASK_B3);
  _keyboard_host_mapButtonB4  = new KeyboardButtonMapping(gamepad->options.keyButtonB4,  GAMEPAD_MASK_B4);
  _keyboard_host_mapButtonR1  = new KeyboardButtonMapping(gamepad->options.keyButtonR1,  GAMEPAD_MASK_R1);
  _keyboard_host_mapButtonL1  = new KeyboardButtonMapping(gamepad->options.keyButtonL1,  GAMEPAD_MASK_L1);
  _keyboard_host_mapButtonS1  = new KeyboardButtonMapping(gamepad->options.keyButtonS1,  GAMEPAD_MASK_S1);
  _keyboard_host_mapButtonS2  = new KeyboardButtonMapping(gamepad->options.keyButtonS2,  GAMEPAD_MASK_S2);
  _keyboard_host_mapButtonL3  = new KeyboardButtonMapping(gamepad->options.keyButtonL3,  GAMEPAD_MASK_L3);
  _keyboard_host_mapButtonR3  = new KeyboardButtonMapping(gamepad->options.keyButtonR3,  GAMEPAD_MASK_R3);
  _keyboard_host_mapButtonA1  = new KeyboardButtonMapping(gamepad->options.keyButtonA1,  GAMEPAD_MASK_A1);
  _keyboard_host_mapButtonA2  = new KeyboardButtonMapping(gamepad->options.keyButtonA2,  GAMEPAD_MASK_A2);
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
  const char* protocol_str[] = { "None", "Keyboard", "Mouse" };
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  uint16_t vid, pid;
  tuh_vid_pid_get(dev_addr, &vid, &pid);

  char tempbuf[256];
  int count = sprintf(tempbuf, "[%04x:%04x][%u] HID Interface%u, Protocol = %s\r\n", vid, pid, dev_addr, instance, protocol_str[itf_protocol]);

  // tud_cdc_write(tempbuf, (uint32_t) count);
  // tud_cdc_write_flush();

  // Receive report from boot keyboard & mouse only
  // tuh_hid_report_received_cb() will be invoked when report is available
  if (itf_protocol == HID_ITF_PROTOCOL_KEYBOARD || itf_protocol == HID_ITF_PROTOCOL_MOUSE)
  {
    if ( !tuh_hid_receive_report(dev_addr, instance) )
    {
      // tud_cdc_write_str("Error: cannot request report\r\n");
    }
  }
}

// Invoked when device with hid interface is un-mounted
void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance)
{
//   char tempbuf[256];
//   int count = sprintf(tempbuf, "[%u] HID Interface%u is unmounted\r\n", dev_addr, instance);
//   tud_cdc_write(tempbuf, count);
//   tud_cdc_write_flush();
}

// look up new key in previous keys
static inline bool find_key_in_report(hid_keyboard_report_t const *report, uint8_t keycode)
{
  for(uint8_t i=0; i<6; i++)
  {
    if (report->keycode[i] == keycode)  return true;
  }

  return false;
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
  bool flush = false;

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
      Gamepad * gamepad = Storage::getInstance().GetGamepad();

      _keyboard_host_state.dpad |=
            ((keycode == _keyboard_host_mapDpadUp->key)    ? (gamepad->options.invertYAxis ? _keyboard_host_mapDpadDown->buttonMask : _keyboard_host_mapDpadUp->buttonMask) : _keyboard_host_state.dpad)
          | ((keycode == _keyboard_host_mapDpadDown->key)  ? (gamepad->options.invertYAxis ? _keyboard_host_mapDpadUp->buttonMask : _keyboard_host_mapDpadDown->buttonMask) : _keyboard_host_state.dpad)
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

      // if ( find_key_in_report(&prev_report, keycode) )
      // {
      //   // exist in previous report means the current key is holding
      //   upButtonHeld = false;
      // }else
      // {
      //   upButtonHeld = true;
      //   // not existed in previous report means the current key is pressed

      //   // remap the key code for Colemak layout
      //   #ifdef KEYBOARD_COLEMAK
      //   uint8_t colemak_key_code = colemak[keycode];
      //   if (colemak_key_code != 0) keycode = colemak_key_code;
      //   #endif

      //   bool const is_shift = report->modifier & (KEYBOARD_MODIFIER_LEFTSHIFT | KEYBOARD_MODIFIER_RIGHTSHIFT);
      //   // uint8_t ch = keycode2ascii[keycode][is_shift ? 1 : 0];

      //   // if (ch)
      //   // {
      //   //   if (ch == '\n') tud_cdc_write("\r", 1);
      //   //   tud_cdc_write(&ch, 1);
      //   //   flush = true;
      //   // }
      // }
    }
    // TODO example skips key released
  }

//   if (flush) tud_cdc_write_flush();

  prev_report = *report;
}

// send mouse report to usb device CDC
static void process_mouse_report(uint8_t dev_addr, hid_mouse_report_t const * report)
{
  //------------- button state  -------------//
  //uint8_t button_changed_mask = report->buttons ^ prev_report.buttons;
  char l = report->buttons & MOUSE_BUTTON_LEFT   ? 'L' : '-';
  char m = report->buttons & MOUSE_BUTTON_MIDDLE ? 'M' : '-';
  char r = report->buttons & MOUSE_BUTTON_RIGHT  ? 'R' : '-';

  char tempbuf[32];
  int count = sprintf(tempbuf, "[%u] %c%c%c %d %d %d\r\n", dev_addr, l, m, r, report->x, report->y, report->wheel);

//   tud_cdc_write(tempbuf, count);
//   tud_cdc_write_flush();
}

// Invoked when received report from device via interrupt endpoint
void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len)
{
  printf("Received report!");
  (void) len;
  uint8_t const itf_protocol = tuh_hid_interface_protocol(dev_addr, instance);

  switch(itf_protocol)
  {
    case HID_ITF_PROTOCOL_KEYBOARD:
      process_kbd_report(dev_addr, (hid_keyboard_report_t const*) report );
    break;

    case HID_ITF_PROTOCOL_MOUSE:
      process_mouse_report(dev_addr, (hid_mouse_report_t const*) report );
    break;

    default: break;
  }

  // continue to request to receive report
  if ( !tuh_hid_receive_report(dev_addr, instance) )
  {
    // tud_cdc_write_str("Error: cannot request report\r\n");
  }
}