/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

// GP2040 Libraries
#include "gamepad.h"
#include "storagemanager.h"

#include "FlashPROM.h"
#include "CRC32.h"

// MUST BE DEFINED for mpgs
uint32_t getMillis() {
	return to_ms_since_boot(get_absolute_time());
}

uint64_t getMicro() {
	return to_us_since_boot(get_absolute_time());
}


static HIDReport hidReport
{
	.square_btn = 0, .cross_btn = 0, .circle_btn = 0, .triangle_btn = 0,
	.l1_btn = 0, .r1_btn = 0, .l2_btn = 0, .r2_btn = 0,
	.select_btn = 0, .start_btn = 0, .l3_btn = 0, .r3_btn = 0, .ps_btn = 0, .tp_btn = 0,
	.direction = 0x08,
	.l_x_axis = 0x80, .l_y_axis = 0x80, .r_x_axis = 0x80, .r_y_axis = 0x80,
	.right_axis = 0x00, .left_axis = 0x00, .up_axis = 0x00, .down_axis = 0x00,
	.triangle_axis = 0x00, .circle_axis = 0x00, .cross_axis = 0x00, .square_axis = 0x00,
	.l1_axis = 0x00, .r1_axis = 0x00, .l2_axis = 0x00, .r2_axis = 0x00
};

static PS4Report ps4Report
{
	.report_id = 0x01,
	.left_stick_x = 0x80, .left_stick_y = 0x80, .right_stick_x = 0x80, .right_stick_y = 0x80,
	.dpad = 0x08,
	.button_west = 0, .button_south = 0, .button_east = 0, .button_north = 0,
	.button_l1 = 0, .button_r1 = 0, .button_l2 = 0, .button_r2 = 0,
	.button_select = 0, .button_start = 0, .button_l3 = 0, .button_r3 = 0, .button_home = 0,	
	.padding = 0,
	.mystery = { },
	.touchpad_data = TouchpadData(),
	.mystery_2 = { }
};

static SwitchReport switchReport
{
	.buttons = 0,
	.hat = SWITCH_HAT_NOTHING,
	.lx = SWITCH_JOYSTICK_MID,
	.ly = SWITCH_JOYSTICK_MID,
	.rx = SWITCH_JOYSTICK_MID,
	.ry = SWITCH_JOYSTICK_MID,
	.vendor = 0,
};

static XInputReport xinputReport
{
	.report_id = 0,
	.report_size = XINPUT_ENDPOINT_SIZE,
	.buttons1 = 0,
	.buttons2 = 0,
	.lt = 0,
	.rt = 0,
	.lx = GAMEPAD_JOYSTICK_MID,
	.ly = GAMEPAD_JOYSTICK_MID,
	.rx = GAMEPAD_JOYSTICK_MID,
	.ry = GAMEPAD_JOYSTICK_MID,
	._reserved = { },
};

static TouchpadData touchpadData;
static uint8_t last_report_counter = 0;


static KeyboardReport keyboardReport
{
	.keycode = { 0 }
};

void Gamepad::setup()
{
	//load(); // MPGS loads
	options = mpgStorage->getGamepadOptions();

	// Configure pin mapping
	f2Mask = (GAMEPAD_MASK_A1 | GAMEPAD_MASK_S2);
	const BoardOptions& boardOptions = Storage::getInstance().getBoardOptions();

	mapDpadUp    = new GamepadButtonMapping(boardOptions.pinDpadUp,    GAMEPAD_MASK_UP);
	mapDpadDown  = new GamepadButtonMapping(boardOptions.pinDpadDown,  GAMEPAD_MASK_DOWN);
	mapDpadLeft  = new GamepadButtonMapping(boardOptions.pinDpadLeft,  GAMEPAD_MASK_LEFT);
	mapDpadRight = new GamepadButtonMapping(boardOptions.pinDpadRight, GAMEPAD_MASK_RIGHT);
	mapButtonB1  = new GamepadButtonMapping(boardOptions.pinButtonB1,  GAMEPAD_MASK_B1);
	mapButtonB2  = new GamepadButtonMapping(boardOptions.pinButtonB2,  GAMEPAD_MASK_B2);
	mapButtonB3  = new GamepadButtonMapping(boardOptions.pinButtonB3,  GAMEPAD_MASK_B3);
	mapButtonB4  = new GamepadButtonMapping(boardOptions.pinButtonB4,  GAMEPAD_MASK_B4);
	mapButtonL1  = new GamepadButtonMapping(boardOptions.pinButtonL1,  GAMEPAD_MASK_L1);
	mapButtonR1  = new GamepadButtonMapping(boardOptions.pinButtonR1,  GAMEPAD_MASK_R1);
	mapButtonL2  = new GamepadButtonMapping(boardOptions.pinButtonL2,  GAMEPAD_MASK_L2);
	mapButtonR2  = new GamepadButtonMapping(boardOptions.pinButtonR2,  GAMEPAD_MASK_R2);
	mapButtonS1  = new GamepadButtonMapping(boardOptions.pinButtonS1,  GAMEPAD_MASK_S1);
	mapButtonS2  = new GamepadButtonMapping(boardOptions.pinButtonS2,  GAMEPAD_MASK_S2);
	mapButtonL3  = new GamepadButtonMapping(boardOptions.pinButtonL3,  GAMEPAD_MASK_L3);
	mapButtonR3  = new GamepadButtonMapping(boardOptions.pinButtonR3,  GAMEPAD_MASK_R3);
	mapButtonA1  = new GamepadButtonMapping(boardOptions.pinButtonA1,  GAMEPAD_MASK_A1);
	mapButtonA2  = new GamepadButtonMapping(boardOptions.pinButtonA2,  GAMEPAD_MASK_A2);

	gamepadMappings = new GamepadButtonMapping *[GAMEPAD_DIGITAL_INPUT_COUNT]
	{
		mapDpadUp,   mapDpadDown, mapDpadLeft, mapDpadRight,
		mapButtonB1, mapButtonB2, mapButtonB3, mapButtonB4,
		mapButtonL1, mapButtonR1, mapButtonL2, mapButtonR2,
		mapButtonS1, mapButtonS2, mapButtonL3, mapButtonR3,
		mapButtonA1, mapButtonA2
	};

	for (int i = 0; i < GAMEPAD_DIGITAL_INPUT_COUNT; i++)
	{
		if (gamepadMappings[i]->isAssigned())
		{
			gpio_init(gamepadMappings[i]->pin);             // Initialize pin
			gpio_set_dir(gamepadMappings[i]->pin, GPIO_IN); // Set as INPUT
			gpio_pull_up(gamepadMappings[i]->pin);          // Set as PULLUP
		}
	}

	#ifdef PIN_SETTINGS
		gpio_init(PIN_SETTINGS);             // Initialize pin
		gpio_set_dir(PIN_SETTINGS, GPIO_IN); // Set as INPUT
		gpio_pull_up(PIN_SETTINGS);          // Set as PULLUP
	#endif

	hotkeyF1Up    =	options.hotkeyF1Up;
	hotkeyF1Down  =	options.hotkeyF1Down;
	hotkeyF1Left  =	options.hotkeyF1Left;
	hotkeyF1Right =	options.hotkeyF1Right;
	hotkeyF2Up 	  =	options.hotkeyF2Up;
	hotkeyF2Down  =	options.hotkeyF2Down;
	hotkeyF2Left  =	options.hotkeyF2Left;
	hotkeyF2Right =	options.hotkeyF2Right;
}

void Gamepad::process()
{
	memcpy(&rawState, &state, sizeof(GamepadState));

	state.dpad = runSOCDCleaner(resolveSOCDMode(options), state.dpad);

	switch (options.dpadMode)
	{
		case DpadMode::DPAD_MODE_LEFT_ANALOG:
			if (!hasRightAnalogStick) {
				state.rx = GAMEPAD_JOYSTICK_MID;
				state.ry = GAMEPAD_JOYSTICK_MID;
			}
			state.lx = dpadToAnalogX(state.dpad);
			state.ly = dpadToAnalogY(state.dpad);
			state.dpad = 0;
			break;

		case DpadMode::DPAD_MODE_RIGHT_ANALOG:
			if (!hasLeftAnalogStick) {
				state.lx = GAMEPAD_JOYSTICK_MID;
				state.ly = GAMEPAD_JOYSTICK_MID;
			}
			state.rx = dpadToAnalogX(state.dpad);
			state.ry = dpadToAnalogY(state.dpad);
			state.dpad = 0;
			break;

		default:
			if (!hasLeftAnalogStick) {
				state.lx = GAMEPAD_JOYSTICK_MID;
				state.ly = GAMEPAD_JOYSTICK_MID;
			}
			if (!hasRightAnalogStick) {
				state.rx = GAMEPAD_JOYSTICK_MID;
				state.ry = GAMEPAD_JOYSTICK_MID;
			}
			break;
	}
}

void Gamepad::read()
{
	// Need to invert since we're using pullups
	uint32_t values = ~gpio_get_all();

	#ifdef PIN_SETTINGS
	state.aux = 0
		| ((values & (1 << PIN_SETTINGS)) ? (1 << 0) : 0)
	;
	#endif

	state.dpad = 0
		| ((values & mapDpadUp->pinMask)    ? (options.invertYAxis ? mapDpadDown->buttonMask : mapDpadUp->buttonMask) : 0)
		| ((values & mapDpadDown->pinMask)  ? (options.invertYAxis ? mapDpadUp->buttonMask : mapDpadDown->buttonMask) : 0)
		| ((values & mapDpadLeft->pinMask)  ? mapDpadLeft->buttonMask  : 0)
		| ((values & mapDpadRight->pinMask) ? mapDpadRight->buttonMask : 0)
	;

	state.buttons = 0
		| ((values & mapButtonB1->pinMask)  ? mapButtonB1->buttonMask  : 0)
		| ((values & mapButtonB2->pinMask)  ? mapButtonB2->buttonMask  : 0)
		| ((values & mapButtonB3->pinMask)  ? mapButtonB3->buttonMask  : 0)
		| ((values & mapButtonB4->pinMask)  ? mapButtonB4->buttonMask  : 0)
		| ((values & mapButtonL1->pinMask)  ? mapButtonL1->buttonMask  : 0)
		| ((values & mapButtonR1->pinMask)  ? mapButtonR1->buttonMask  : 0)
		| ((values & mapButtonL2->pinMask)  ? mapButtonL2->buttonMask  : 0)
		| ((values & mapButtonR2->pinMask)  ? mapButtonR2->buttonMask  : 0)
		| ((values & mapButtonS1->pinMask)  ? mapButtonS1->buttonMask  : 0)
		| ((values & mapButtonS2->pinMask)  ? mapButtonS2->buttonMask  : 0)
		| ((values & mapButtonL3->pinMask)  ? mapButtonL3->buttonMask  : 0)
		| ((values & mapButtonR3->pinMask)  ? mapButtonR3->buttonMask  : 0)
		| ((values & mapButtonA1->pinMask)  ? mapButtonA1->buttonMask  : 0)
		| ((values & mapButtonA2->pinMask)  ? mapButtonA2->buttonMask  : 0)
	;

	state.lx = GAMEPAD_JOYSTICK_MID;
	state.ly = GAMEPAD_JOYSTICK_MID;
	state.rx = GAMEPAD_JOYSTICK_MID;
	state.ry = GAMEPAD_JOYSTICK_MID;
	state.lt = 0;
	state.rt = 0;
}

void Gamepad::debounce() {
	debouncer.debounce(&state);
}

void Gamepad::save()
{
	bool dirty = false;
	GamepadOptions savedOptions = mpgStorage->getGamepadOptions();
	if (memcmp(&savedOptions, &options, sizeof(GamepadOptions)))
	{
		mpgStorage->setGamepadOptions(options);
		dirty = true;
	}

	if (dirty)
		mpgStorage->save();
}

GamepadHotkey Gamepad::hotkey()
{
	static GamepadHotkey lastAction = HOTKEY_NONE;
	GamepadHotkey action = HOTKEY_NONE;
	if (pressedF1())
	{
		if (state.dpad == hotkeyF1Up   .dpadMask) action = hotkeyF1Up   .action;
		if (state.dpad == hotkeyF1Down .dpadMask) action = hotkeyF1Down .action;
		if (state.dpad == hotkeyF1Left .dpadMask) action = hotkeyF1Left .action;
		if (state.dpad == hotkeyF1Right.dpadMask) action = hotkeyF1Right.action;
		if (action != HOTKEY_NONE) {
			state.dpad = 0;
			state.buttons &= ~(f1Mask);
		}
	} else if (pressedF2()) {
		if (state.dpad == hotkeyF2Up   .dpadMask) action = hotkeyF2Up   .action;
		if (state.dpad == hotkeyF2Down .dpadMask) action = hotkeyF2Down .action;
		if (state.dpad == hotkeyF2Left .dpadMask) action = hotkeyF2Left .action;
		if (state.dpad == hotkeyF2Right.dpadMask) action = hotkeyF2Right.action;
		if (action != HOTKEY_NONE) {
			state.dpad = 0;
			state.buttons &= ~(f2Mask);
		}
	}

	switch (action) {
		case HOTKEY_NONE              : return action;
		case HOTKEY_DPAD_DIGITAL      : options.dpadMode = DPAD_MODE_DIGITAL; break;
		case HOTKEY_DPAD_LEFT_ANALOG  : options.dpadMode = DPAD_MODE_LEFT_ANALOG; break;
		case HOTKEY_DPAD_RIGHT_ANALOG : options.dpadMode = DPAD_MODE_RIGHT_ANALOG; break;
		case HOTKEY_HOME_BUTTON       : state.buttons |= GAMEPAD_MASK_A1; break; // Press the Home button
		case HOTKEY_CAPTURE_BUTTON    :
			break;
		case HOTKEY_SOCD_UP_PRIORITY  : options.socdMode = SOCD_MODE_UP_PRIORITY; break;
		case HOTKEY_SOCD_NEUTRAL      : options.socdMode = SOCD_MODE_NEUTRAL; break;
		case HOTKEY_SOCD_LAST_INPUT   : options.socdMode = SOCD_MODE_SECOND_INPUT_PRIORITY; break;
		case HOTKEY_SOCD_FIRST_INPUT  : options.socdMode = SOCD_MODE_FIRST_INPUT_PRIORITY; break;
		case HOTKEY_SOCD_BYPASS       : options.socdMode = SOCD_MODE_BYPASS; break;
		case HOTKEY_INVERT_X_AXIS     : break;
		case HOTKEY_INVERT_Y_AXIS     :
			if (lastAction != HOTKEY_INVERT_Y_AXIS)
				options.invertYAxis = !options.invertYAxis;
			break;
	}

	GamepadHotkey hotkey = action;
	if (hotkey != GamepadHotkey::HOTKEY_NONE)
		save();

	return hotkey;
}


void * Gamepad::getReport()
{
	switch (options.inputMode)
	{
		case INPUT_MODE_XINPUT:
			return getXInputReport();

		case INPUT_MODE_SWITCH:
			return getSwitchReport();

		case INPUT_MODE_PS4:
			return getPS4Report();

		case INPUT_MODE_KEYBOARD:
			return getKeyboardReport();

		default:
			return getHIDReport();
	}
}


uint16_t Gamepad::getReportSize()
{
	switch (options.inputMode)
	{
		case INPUT_MODE_XINPUT:
			return sizeof(XInputReport);

		case INPUT_MODE_SWITCH:
			return sizeof(SwitchReport);

		case INPUT_MODE_PS4:
			return sizeof(PS4Report);

		case INPUT_MODE_KEYBOARD:
			return sizeof(KeyboardReport);

		default:
			return sizeof(HIDReport);
	}
}


HIDReport *Gamepad::getHIDReport()
{
	switch (state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        hidReport.direction = HID_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   hidReport.direction = HID_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     hidReport.direction = HID_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: hidReport.direction = HID_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      hidReport.direction = HID_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  hidReport.direction = HID_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      hidReport.direction = HID_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    hidReport.direction = HID_HAT_UPLEFT;    break;
		default:                                     hidReport.direction = HID_HAT_NOTHING;   break;
	}

	hidReport.cross_btn    = pressedB1();
	hidReport.circle_btn   = pressedB2();
	hidReport.square_btn   = pressedB3();
	hidReport.triangle_btn = pressedB4();
	hidReport.l1_btn       = pressedL1();
	hidReport.r1_btn       = pressedR1();
	hidReport.l2_btn       = pressedL2();
	hidReport.r2_btn       = pressedR2();
	hidReport.select_btn   = pressedS1();
	hidReport.start_btn    = pressedS2();
	hidReport.l3_btn       = pressedL3();
	hidReport.r3_btn       = pressedR3();
	hidReport.ps_btn       = pressedA1();
	hidReport.tp_btn       = pressedA2();

	hidReport.l_x_axis = static_cast<uint8_t>(state.lx >> 8);
	hidReport.l_y_axis = static_cast<uint8_t>(state.ly >> 8);
	hidReport.r_x_axis = static_cast<uint8_t>(state.rx >> 8);
	hidReport.r_y_axis = static_cast<uint8_t>(state.ry >> 8);

	return &hidReport;
}


SwitchReport *Gamepad::getSwitchReport()
{
	switch (state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        switchReport.hat = SWITCH_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   switchReport.hat = SWITCH_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     switchReport.hat = SWITCH_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: switchReport.hat = SWITCH_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      switchReport.hat = SWITCH_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  switchReport.hat = SWITCH_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      switchReport.hat = SWITCH_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    switchReport.hat = SWITCH_HAT_UPLEFT;    break;
		default:                                     switchReport.hat = SWITCH_HAT_NOTHING;   break;
	}

	switchReport.buttons = 0
		| (pressedB1() ? SWITCH_MASK_B       : 0)
		| (pressedB2() ? SWITCH_MASK_A       : 0)
		| (pressedB3() ? SWITCH_MASK_Y       : 0)
		| (pressedB4() ? SWITCH_MASK_X       : 0)
		| (pressedL1() ? SWITCH_MASK_L       : 0)
		| (pressedR1() ? SWITCH_MASK_R       : 0)
		| (pressedL2() ? SWITCH_MASK_ZL      : 0)
		| (pressedR2() ? SWITCH_MASK_ZR      : 0)
		| (pressedS1() ? SWITCH_MASK_MINUS   : 0)
		| (pressedS2() ? SWITCH_MASK_PLUS    : 0)
		| (pressedL3() ? SWITCH_MASK_L3      : 0)
		| (pressedR3() ? SWITCH_MASK_R3      : 0)
		| (pressedA1() ? SWITCH_MASK_HOME    : 0)
		| (pressedA2() ? SWITCH_MASK_CAPTURE : 0)
	;

	switchReport.lx = static_cast<uint8_t>(state.lx >> 8);
	switchReport.ly = static_cast<uint8_t>(state.ly >> 8);
	switchReport.rx = static_cast<uint8_t>(state.rx >> 8);
	switchReport.ry = static_cast<uint8_t>(state.ry >> 8);

	return &switchReport;
}


XInputReport *Gamepad::getXInputReport()
{
	xinputReport.buttons1 = 0
		| (pressedUp()    ? XBOX_MASK_UP    : 0)
		| (pressedDown()  ? XBOX_MASK_DOWN  : 0)
		| (pressedLeft()  ? XBOX_MASK_LEFT  : 0)
		| (pressedRight() ? XBOX_MASK_RIGHT : 0)
		| (pressedS2()    ? XBOX_MASK_START : 0)
		| (pressedS1()    ? XBOX_MASK_BACK  : 0)
		| (pressedL3()    ? XBOX_MASK_LS    : 0)
		| (pressedR3()    ? XBOX_MASK_RS    : 0)
	;

	xinputReport.buttons2 = 0
		| (pressedL1() ? XBOX_MASK_LB   : 0)
		| (pressedR1() ? XBOX_MASK_RB   : 0)
		| (pressedA1() ? XBOX_MASK_HOME : 0)
		| (pressedB1() ? XBOX_MASK_A    : 0)
		| (pressedB2() ? XBOX_MASK_B    : 0)
		| (pressedB3() ? XBOX_MASK_X    : 0)
		| (pressedB4() ? XBOX_MASK_Y    : 0)
	;

	xinputReport.lx = static_cast<int16_t>(state.lx) + INT16_MIN;
	xinputReport.ly = static_cast<int16_t>(~state.ly) + INT16_MIN;
	xinputReport.rx = static_cast<int16_t>(state.rx) + INT16_MIN;
	xinputReport.ry = static_cast<int16_t>(~state.ry) + INT16_MIN;

	if (hasAnalogTriggers)
	{
		xinputReport.lt = state.lt;
		xinputReport.rt = state.rt;
	}
	else
	{
		xinputReport.lt = pressedL2() ? 0xFF : 0;
		xinputReport.rt = pressedR2() ? 0xFF : 0;
	}

	return &xinputReport;
}


PS4Report *Gamepad::getPS4Report()
{
	switch (state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        ps4Report.dpad = HID_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   ps4Report.dpad = HID_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     ps4Report.dpad = HID_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: ps4Report.dpad = HID_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      ps4Report.dpad = HID_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  ps4Report.dpad = HID_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      ps4Report.dpad = HID_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    ps4Report.dpad = HID_HAT_UPLEFT;    break;
		default:                                     ps4Report.dpad = PS4_HAT_NOTHING;   break;
	}

	ps4Report.button_south    = pressedB1();
	ps4Report.button_east     = pressedB2();
	ps4Report.button_west     = pressedB3();
	ps4Report.button_north    = pressedB4();
	ps4Report.button_l1       = pressedL1();
	ps4Report.button_r1       = pressedR1();
	ps4Report.button_l2       = pressedL2();
	ps4Report.button_r2       = pressedR2();
	ps4Report.button_select   = pressedS1();
	ps4Report.button_start    = pressedS2();
	ps4Report.button_l3       = pressedL3();
	ps4Report.button_r3       = pressedR3();
	ps4Report.button_home     = pressedA1();
	ps4Report.button_touchpad = pressedA2();

	// report counter is 6 bits, but we circle 0-255
	ps4Report.report_counter = last_report_counter++;

	ps4Report.left_stick_x = static_cast<uint8_t>(state.lx >> 8);
	ps4Report.left_stick_y = static_cast<uint8_t>(state.ly >> 8);
	ps4Report.right_stick_x = static_cast<uint8_t>(state.rx >> 8);
	ps4Report.right_stick_y = static_cast<uint8_t>(state.ry >> 8);

	ps4Report.left_trigger = 0;
	ps4Report.right_trigger = 0;

	// set touchpad to nothing
	touchpadData.p1.unpressed = 1;
	touchpadData.p2.unpressed = 1;
	ps4Report.touchpad_data = touchpadData;

	return &ps4Report;
}

uint8_t Gamepad::getModifier(uint8_t code) {
	switch (code) {
		case HID_KEY_CONTROL_LEFT : return KEYBOARD_MODIFIER_LEFTCTRL  ;
		case HID_KEY_SHIFT_LEFT   : return KEYBOARD_MODIFIER_LEFTSHIFT ;
		case HID_KEY_ALT_LEFT     : return KEYBOARD_MODIFIER_LEFTALT   ;
		case HID_KEY_GUI_LEFT     : return KEYBOARD_MODIFIER_LEFTGUI   ;
		case HID_KEY_CONTROL_RIGHT: return KEYBOARD_MODIFIER_RIGHTCTRL ;
		case HID_KEY_SHIFT_RIGHT  : return KEYBOARD_MODIFIER_RIGHTSHIFT;
		case HID_KEY_ALT_RIGHT    : return KEYBOARD_MODIFIER_RIGHTALT  ;
		case HID_KEY_GUI_RIGHT    : return KEYBOARD_MODIFIER_RIGHTGUI  ;
	}

	return 0;
}

void Gamepad::pressKey(uint8_t code) {
	if (code >= HID_KEY_CONTROL_LEFT) {
		keyboardReport.keycode[0] |= getModifier(code);
	} else if ((code >> 3) < KEY_COUNT - 2) {
		keyboardReport.keycode[(code >> 3) + 1] |= 1 << (code & 7);
	}
}

void Gamepad::releaseAllKeys(void) {
	for (uint8_t i = 0; i < (sizeof(keyboardReport.keycode) / sizeof(keyboardReport.keycode[0])); i++) {
		keyboardReport.keycode[i] = 0;
	}
}

KeyboardReport *Gamepad::getKeyboardReport()
{
	releaseAllKeys();
	if(pressedUp())     { pressKey(options.keyDpadUp); }
	if(pressedDown())   { pressKey(options.keyDpadDown); }
	if(pressedLeft())	{ pressKey(options.keyDpadLeft); }
	if(pressedRight()) 	{ pressKey(options.keyDpadRight); }
	if(pressedB1()) 	{ pressKey(options.keyButtonB1); }
	if(pressedB2()) 	{ pressKey(options.keyButtonB2); }
	if(pressedB3()) 	{ pressKey(options.keyButtonB3); }
	if(pressedB4()) 	{ pressKey(options.keyButtonB4); }
	if(pressedL1()) 	{ pressKey(options.keyButtonL1); }
	if(pressedR1()) 	{ pressKey(options.keyButtonR1); }
	if(pressedL2()) 	{ pressKey(options.keyButtonL2); }
	if(pressedR2()) 	{ pressKey(options.keyButtonR2); }
	if(pressedS1()) 	{ pressKey(options.keyButtonS1); }
	if(pressedS2()) 	{ pressKey(options.keyButtonS2); }
	if(pressedL3()) 	{ pressKey(options.keyButtonL3); }
	if(pressedR3()) 	{ pressKey(options.keyButtonR3); }
	if(pressedA1()) 	{ pressKey(options.keyButtonA1); }
	if(pressedA2()) 	{ pressKey(options.keyButtonA2); }
	return &keyboardReport;
}


/* Gamepad stuffs */
void GamepadStorage::start()
{
	//EEPROM.start();
}

void GamepadStorage::save()
{
	EEPROM.commit();
}

GamepadOptions GamepadStorage::getGamepadOptions()
{
	GamepadOptions options;
	EEPROM.get(GAMEPAD_STORAGE_INDEX, options);

	uint32_t lastCRC = options.checksum;
	options.checksum = CHECKSUM_MAGIC;
	if (CRC32::calculate(&options) != lastCRC)
	{
		#ifdef DEFAULT_INPUT_MODE
		options.inputMode = DEFAULT_INPUT_MODE;
		#else
		options.inputMode = InputMode::INPUT_MODE_XINPUT; // Default?
		#endif
		#ifdef DEFAULT_DPAD_MODE
		options.dpadMode = DEFAULT_DPAD_MODE;
		#else
		options.dpadMode = DpadMode::DPAD_MODE_DIGITAL; // Default?
		#endif
		#ifdef DEFAULT_SOCD_MODE
		options.socdMode = DEFAULT_SOCD_MODE;
		#else
		options.socdMode = SOCD_MODE_NEUTRAL;
		#endif
		options.invertXAxis = false;
		options.invertYAxis = false;
		options.keyDpadUp    = KEY_DPAD_UP;
		options.keyDpadDown  = KEY_DPAD_DOWN;
		options.keyDpadRight = KEY_DPAD_RIGHT;
		options.keyDpadLeft  = KEY_DPAD_LEFT;
		options.keyButtonB1  = KEY_BUTTON_B1;
		options.keyButtonB2  = KEY_BUTTON_B2;
		options.keyButtonR2  = KEY_BUTTON_R2;
		options.keyButtonL2  = KEY_BUTTON_L2;
		options.keyButtonB3  = KEY_BUTTON_B3;
		options.keyButtonB4  = KEY_BUTTON_B4;
		options.keyButtonR1  = KEY_BUTTON_R1;
		options.keyButtonL1  = KEY_BUTTON_L1;
		options.keyButtonS1  = KEY_BUTTON_S1;
		options.keyButtonS2  = KEY_BUTTON_S2;
		options.keyButtonL3  = KEY_BUTTON_L3;
		options.keyButtonR3  = KEY_BUTTON_R3;
		options.keyButtonA1  = KEY_BUTTON_A1;
		options.keyButtonA2  = KEY_BUTTON_A2;

	    options.hotkeyF1Up = { HOTKEY_F1_UP_MASK, HOTKEY_F1_UP_ACTION };
	    options.hotkeyF1Down = { HOTKEY_F1_DOWN_MASK, HOTKEY_F1_DOWN_ACTION };
	    options.hotkeyF1Left = { HOTKEY_F1_LEFT_MASK, HOTKEY_F1_LEFT_ACTION };
	    options.hotkeyF1Right = { HOTKEY_F1_RIGHT_MASK, HOTKEY_F1_RIGHT_ACTION };
	    options.hotkeyF2Up = { HOTKEY_F2_UP_MASK, HOTKEY_F2_UP_ACTION };
	    options.hotkeyF2Down = { HOTKEY_F2_DOWN_MASK, HOTKEY_F2_DOWN_ACTION };
	    options.hotkeyF2Left = { HOTKEY_F2_LEFT_MASK, HOTKEY_F2_LEFT_ACTION };
	    options.hotkeyF2Right = { HOTKEY_F2_RIGHT_MASK, HOTKEY_F2_RIGHT_ACTION };
		setGamepadOptions(options);
	}

	return options;
}

void GamepadStorage::setGamepadOptions(GamepadOptions options)
{
	options.checksum = 0;
	options.checksum = CRC32::calculate(&options);
	EEPROM.set(GAMEPAD_STORAGE_INDEX, options);
}

