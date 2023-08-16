/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

// GP2040 Libraries
#include "gamepad.h"
#include "enums.pb.h"
#include "storagemanager.h"

#include "FlashPROM.h"
#include "CRC32.h"

#include "storagemanager.h"
#include "system.h"

// PS5 compatibility
#include "ps4_driver.h"

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
	.keycode = { 0 },
	.multimedia = 0
};

Gamepad::Gamepad(int debounceMS) :
	debounceMS(debounceMS)
	, debouncer(debounceMS)
	, options(Storage::getInstance().getGamepadOptions())
	, hotkeyOptions(Storage::getInstance().getHotkeyOptions())
{}

void Gamepad::setup()
{
	// Configure pin mapping
	const PinMappings& pinMappings = Storage::getInstance().getProfilePinMappings();
	const GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();

	const auto convertPin = [](int32_t pin) -> uint8_t { return isValidPin(pin) ? pin : 0xff; };
	mapDpadUp    = new GamepadButtonMapping(convertPin(pinMappings.pinDpadUp),		GAMEPAD_MASK_UP);
	mapDpadDown  = new GamepadButtonMapping(convertPin(pinMappings.pinDpadDown),	GAMEPAD_MASK_DOWN);
	mapDpadLeft  = new GamepadButtonMapping(convertPin(pinMappings.pinDpadLeft),	GAMEPAD_MASK_LEFT);
	mapDpadRight = new GamepadButtonMapping(convertPin(pinMappings.pinDpadRight),	GAMEPAD_MASK_RIGHT);
	mapButtonB1  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonB1),	GAMEPAD_MASK_B1);
	mapButtonB2  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonB2),	GAMEPAD_MASK_B2);
	mapButtonB3  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonB3),	GAMEPAD_MASK_B3);
	mapButtonB4  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonB4),	GAMEPAD_MASK_B4);
	mapButtonL1  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonL1),	GAMEPAD_MASK_L1);
	mapButtonR1  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonR1),	GAMEPAD_MASK_R1);
	mapButtonL2  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonL2),	GAMEPAD_MASK_L2);
	mapButtonR2  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonR2),	GAMEPAD_MASK_R2);
	mapButtonS1  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonS1),	GAMEPAD_MASK_S1);
	mapButtonS2  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonS2),	GAMEPAD_MASK_S2);
	mapButtonL3  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonL3),	GAMEPAD_MASK_L3);
	mapButtonR3  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonR3),	GAMEPAD_MASK_R3);
	mapButtonA1  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonA1),	GAMEPAD_MASK_A1);
	mapButtonA2  = new GamepadButtonMapping(convertPin(pinMappings.pinButtonA2),	GAMEPAD_MASK_A2);

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

	// initialize the Function pin button/switch if it is configured
	if (isValidPin(pinMappings.pinButtonFn)) {
		gpio_init(pinMappings.pinButtonFn);             // Initialize pin
		gpio_set_dir(pinMappings.pinButtonFn, GPIO_IN); // Set as INPUT
		gpio_pull_up(pinMappings.pinButtonFn);          // Set as PULLUP
	}

	// setup PS5 compatibility
	PS4Data::getInstance().ps4ControllerType = gamepadOptions.ps4ControllerType;
}

/**
 * @brief Undo setup().
 */
void Gamepad::teardown_and_reinit(const uint32_t profileNum)
{
	const PinMappings& pinMappings = Storage::getInstance().getProfilePinMappings();
	// deinitialize the GPIO pins so we don't have orphans
	for (int i = 0; i < GAMEPAD_DIGITAL_INPUT_COUNT; i++)
	{
		if (gamepadMappings[i]->isAssigned())
		{
			gpio_deinit(gamepadMappings[i]->pin);
		}
	}
	if (isValidPin(pinMappings.pinButtonFn)) {
		gpio_deinit(pinMappings.pinButtonFn);
	}

	// set to new profile
	Storage::getInstance().setProfile(profileNum);

	// reinitialize pin mappings
	this->setup();
}

void Gamepad::process()
{
	memcpy(&rawState, &state, sizeof(GamepadState));

	// NOTE: Inverted X/Y-axis must run before SOCD and Dpad processing
	if (options.invertXAxis) {
		bool left = (state.dpad & mapDpadLeft->buttonMask) != 0;
		bool right = (state.dpad & mapDpadRight->buttonMask) != 0;
		state.dpad &= ~(mapDpadLeft->buttonMask | mapDpadRight->buttonMask);
		if (left)
			state.dpad |= mapDpadRight->buttonMask;
		if (right)
			state.dpad |= mapDpadLeft->buttonMask;
	}

	if (options.invertYAxis) {
		bool up = (state.dpad & mapDpadUp->buttonMask) != 0;
		bool down = (state.dpad & mapDpadDown->buttonMask) != 0;
		state.dpad &= ~(mapDpadUp->buttonMask | mapDpadDown->buttonMask);
		if (up)
			state.dpad |= mapDpadDown->buttonMask;
		if (down)
			state.dpad |= mapDpadUp->buttonMask;
	}

	state.dpad = runSOCDCleaner(resolveSOCDMode(options), state.dpad);

	// SOCD cleaning first, allows for control over which diagonal to take/filter
	if (options.fourWayMode) {
		state.dpad = filterToFourWayMode(state.dpad);
	}

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
	const PinMappings& pinMappings = Storage::getInstance().getProfilePinMappings();
	// Need to invert since we're using pullups
	uint32_t values = ~gpio_get_all();

	state.aux = 0
		| (values & (1 << pinMappings.pinButtonFn)) ? AUX_MASK_FUNCTION : 0;

	state.dpad = 0
		| ((values & mapDpadUp->pinMask)    ? mapDpadUp->buttonMask : 0)
		| ((values & mapDpadDown->pinMask)  ? mapDpadDown->buttonMask : 0)
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
	Storage::getInstance().save();
}

void Gamepad::hotkey()
{
	if (options.lockHotkeys) return;

	GamepadHotkey action = HOTKEY_NONE;

	if (pressedHotkey(hotkeyOptions.hotkey01))	action = selectHotkey(hotkeyOptions.hotkey01);
	else if (pressedHotkey(hotkeyOptions.hotkey02))	action = selectHotkey(hotkeyOptions.hotkey02);
	else if (pressedHotkey(hotkeyOptions.hotkey03))	action = selectHotkey(hotkeyOptions.hotkey03);
	else if (pressedHotkey(hotkeyOptions.hotkey04))	action = selectHotkey(hotkeyOptions.hotkey04);
	else if (pressedHotkey(hotkeyOptions.hotkey05))	action = selectHotkey(hotkeyOptions.hotkey05);
	else if (pressedHotkey(hotkeyOptions.hotkey06))	action = selectHotkey(hotkeyOptions.hotkey06);
	else if (pressedHotkey(hotkeyOptions.hotkey07))	action = selectHotkey(hotkeyOptions.hotkey07);
	else if (pressedHotkey(hotkeyOptions.hotkey08))	action = selectHotkey(hotkeyOptions.hotkey08);
	else if (pressedHotkey(hotkeyOptions.hotkey09))	action = selectHotkey(hotkeyOptions.hotkey09);
	else if (pressedHotkey(hotkeyOptions.hotkey10))	action = selectHotkey(hotkeyOptions.hotkey10);
	else if (pressedHotkey(hotkeyOptions.hotkey11))	action = selectHotkey(hotkeyOptions.hotkey11);
	else if (pressedHotkey(hotkeyOptions.hotkey12))	action = selectHotkey(hotkeyOptions.hotkey12);
	else                                        lastAction = HOTKEY_NONE;
	processHotkeyIfNewAction(action);
}

/**
 * @brief Take a hotkey action if it hasn't already been taken, modifying state/options appropriately.
 */
void Gamepad::processHotkeyIfNewAction(GamepadHotkey action)
{
	bool reqSave = false;
	switch (action) {
		case HOTKEY_NONE              : return;
		case HOTKEY_DPAD_DIGITAL      : options.dpadMode = DPAD_MODE_DIGITAL; reqSave = true; break;
		case HOTKEY_DPAD_LEFT_ANALOG  : options.dpadMode = DPAD_MODE_LEFT_ANALOG; reqSave = true; break;
		case HOTKEY_DPAD_RIGHT_ANALOG : options.dpadMode = DPAD_MODE_RIGHT_ANALOG; reqSave = true; break;
		case HOTKEY_HOME_BUTTON       : state.buttons |= GAMEPAD_MASK_A1; break; // Press the Home button
		case HOTKEY_L3_BUTTON         : state.buttons |= GAMEPAD_MASK_L3; break; // Press the L3 button
		case HOTKEY_R3_BUTTON         : state.buttons |= GAMEPAD_MASK_R3; break; // Press the R3 button
		case HOTKEY_SOCD_UP_PRIORITY  : options.socdMode = SOCD_MODE_UP_PRIORITY; reqSave = true; break;
		case HOTKEY_SOCD_NEUTRAL      : options.socdMode = SOCD_MODE_NEUTRAL; reqSave = true; break;
		case HOTKEY_SOCD_LAST_INPUT   : options.socdMode = SOCD_MODE_SECOND_INPUT_PRIORITY; reqSave = true; break;
		case HOTKEY_SOCD_FIRST_INPUT  : options.socdMode = SOCD_MODE_FIRST_INPUT_PRIORITY;  reqSave = true;break;
		case HOTKEY_SOCD_BYPASS       : options.socdMode = SOCD_MODE_BYPASS; reqSave = true; break;
		case HOTKEY_REBOOT_DEFAULT    : System::reboot(System::BootMode::DEFAULT); break;
		case HOTKEY_CAPTURE_BUTTON    :
			if (options.inputMode == INPUT_MODE_PS4 && options.switchTpShareForDs4) {
				state.buttons |= GAMEPAD_MASK_A2;
			} else {
				state.buttons |= GAMEPAD_MASK_S1;
			}
			break;
		case HOTKEY_TOUCHPAD_BUTTON    :
			if (options.inputMode == INPUT_MODE_PS4) {
				state.buttons |= GAMEPAD_MASK_A2;
			} else {
				state.buttons |= GAMEPAD_MASK_S1;
			}
			break;				
		case HOTKEY_INVERT_X_AXIS     :
			if (action != lastAction) {
				options.invertXAxis = !options.invertXAxis;
				reqSave = true;
			}
			break;
		case HOTKEY_INVERT_Y_AXIS     :
			if (action != lastAction) {
				options.invertYAxis = !options.invertYAxis;
				reqSave = true;
			}
			break;
		case HOTKEY_TOGGLE_4_WAY_MODE :
			if (action != lastAction) {
				options.fourWayMode = !options.fourWayMode;
				reqSave = true;
			}
			break;
		case HOTKEY_TOGGLE_DDI_4_WAY_MODE:
			if (action != lastAction) {
				DualDirectionalOptions& ddiOpt = Storage::getInstance().getAddonOptions().dualDirectionalOptions;
				ddiOpt.fourWayMode = !ddiOpt.fourWayMode;
				reqSave = true;
			}
			break;
		case HOTKEY_LOAD_PROFILE_1:
			if (action != lastAction) {
				this->teardown_and_reinit(1);
				reqSave = true;
			}
			break;
		case HOTKEY_LOAD_PROFILE_2:
			if (action != lastAction) {
				this->teardown_and_reinit(2);
				reqSave = true;
			}
			break;
		case HOTKEY_LOAD_PROFILE_3:
			if (action != lastAction) {
				this->teardown_and_reinit(3);
				reqSave = true;
			}
			break;
		case HOTKEY_LOAD_PROFILE_4:
			if (action != lastAction) {
				this->teardown_and_reinit(4);
				reqSave = true;
			}
			break;
	}

	// only save if we did something different (except NONE because NONE doesn't get here)
	if (action != lastAction && reqSave) {
		save();
	}

	lastAction = action;
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
	ps4Report.button_select   = options.switchTpShareForDs4 ? pressedA2() : pressedS1();
	ps4Report.button_start    = pressedS2();
	ps4Report.button_l3       = pressedL3();
	ps4Report.button_r3       = pressedR3();
	ps4Report.button_home     = pressedA1();
	ps4Report.button_touchpad = options.switchTpShareForDs4 ? pressedS1() : pressedA2();

	// report counter is 6 bits, but we circle 0-255
	ps4Report.report_counter = last_report_counter++;

	ps4Report.left_stick_x = static_cast<uint8_t>(state.lx >> 8);
	ps4Report.left_stick_y = static_cast<uint8_t>(state.ly >> 8);
	ps4Report.right_stick_x = static_cast<uint8_t>(state.rx >> 8);
	ps4Report.right_stick_y = static_cast<uint8_t>(state.ry >> 8);

	if (hasAnalogTriggers)
	{
		ps4Report.left_trigger = state.lt;
		ps4Report.right_trigger = state.rt;
	}
	else
	{
		ps4Report.left_trigger = pressedL2() ? 0xFF : 0;
		ps4Report.right_trigger = pressedR2() ? 0xFF : 0;
	}

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

uint8_t Gamepad::getMultimedia(uint8_t code) {
	switch (code) {
		case KEYBOARD_MULTIMEDIA_NEXT_TRACK : return 0x01;
		case KEYBOARD_MULTIMEDIA_PREV_TRACK : return 0x02;
		case KEYBOARD_MULTIMEDIA_STOP 	    : return 0x04;
		case KEYBOARD_MULTIMEDIA_PLAY_PAUSE : return 0x08;
		case KEYBOARD_MULTIMEDIA_MUTE 	    : return 0x10;
		case KEYBOARD_MULTIMEDIA_VOLUME_UP  : return 0x20;
		case KEYBOARD_MULTIMEDIA_VOLUME_DOWN: return 0x40;
	}
	return 0;
}

void Gamepad::pressKey(uint8_t code) {
	if (code > HID_KEY_GUI_RIGHT) {
		keyboardReport.reportId = KEYBOARD_MULTIMEDIA_REPORT_ID;
		keyboardReport.multimedia = getMultimedia(code);
	}  else {
		keyboardReport.reportId = KEYBOARD_KEY_REPORT_ID;
		keyboardReport.keycode[code / 8] |= 1 << (code % 8);
	}
}

void Gamepad::releaseAllKeys(void) {
	for (uint8_t i = 0; i < (sizeof(keyboardReport.keycode) / sizeof(keyboardReport.keycode[0])); i++) {
		keyboardReport.keycode[i] = 0;
	}
	keyboardReport.multimedia = 0;
}

KeyboardReport *Gamepad::getKeyboardReport()
{
	const KeyboardMapping& keyboardMapping = Storage::getInstance().getKeyboardMapping();
	releaseAllKeys();
	if(pressedUp())     { pressKey(keyboardMapping.keyDpadUp); }
	if(pressedDown())   { pressKey(keyboardMapping.keyDpadDown); }
	if(pressedLeft())	{ pressKey(keyboardMapping.keyDpadLeft); }
	if(pressedRight()) 	{ pressKey(keyboardMapping.keyDpadRight); }
	if(pressedB1()) 	{ pressKey(keyboardMapping.keyButtonB1); }
	if(pressedB2()) 	{ pressKey(keyboardMapping.keyButtonB2); }
	if(pressedB3()) 	{ pressKey(keyboardMapping.keyButtonB3); }
	if(pressedB4()) 	{ pressKey(keyboardMapping.keyButtonB4); }
	if(pressedL1()) 	{ pressKey(keyboardMapping.keyButtonL1); }
	if(pressedR1()) 	{ pressKey(keyboardMapping.keyButtonR1); }
	if(pressedL2()) 	{ pressKey(keyboardMapping.keyButtonL2); }
	if(pressedR2()) 	{ pressKey(keyboardMapping.keyButtonR2); }
	if(pressedS1()) 	{ pressKey(keyboardMapping.keyButtonS1); }
	if(pressedS2()) 	{ pressKey(keyboardMapping.keyButtonS2); }
	if(pressedL3()) 	{ pressKey(keyboardMapping.keyButtonL3); }
	if(pressedR3()) 	{ pressKey(keyboardMapping.keyButtonR3); }
	if(pressedA1()) 	{ pressKey(keyboardMapping.keyButtonA1); }
	if(pressedA2()) 	{ pressKey(keyboardMapping.keyButtonA2); }
	return &keyboardReport;
}
