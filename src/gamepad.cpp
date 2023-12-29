/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

// GP2040 Libraries
#include "gamepad.h"
#include "enums.pb.h"
#include "storagemanager.h"
#include "types.h"

#include "FlashPROM.h"
#include "CRC32.h"

#include "storagemanager.h"
#include "system.h"

// PS5 compatibility
#include "ps4_driver.h"

// Xbox One compatibility
#include "xbone_driver.h"

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
	.l_x_axis = HID_JOYSTICK_MID,
	.l_y_axis = HID_JOYSTICK_MID,
	.r_x_axis = HID_JOYSTICK_MID,
	.r_y_axis = HID_JOYSTICK_MID,
	.right_axis = 0x00, .left_axis = 0x00, .up_axis = 0x00, .down_axis = 0x00,
	.triangle_axis = 0x00, .circle_axis = 0x00, .cross_axis = 0x00, .square_axis = 0x00,
	.l1_axis = 0x00, .r1_axis = 0x00, .l2_axis = 0x00, .r2_axis = 0x00
};

static PS4Report ps4Report
{
	.report_id = 0x01,
	.left_stick_x = PS4_JOYSTICK_MID,
	.left_stick_y = PS4_JOYSTICK_MID,
	.right_stick_x = PS4_JOYSTICK_MID,
	.right_stick_y = PS4_JOYSTICK_MID,
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

static NeogeoReport neogeoReport
{
	.buttons = 0,
	.hat = 0xf,
	.const0 = 0x80,
	.const1 = 0x80,
	.const2 = 0x80,
	.const3 = 0x80,
	.const4 = 0,	
	.const5 = 0,	
	.const6 = 0,	
	.const7 = 0,	
	.const8 = 0,	
	.const9 = 0,	
	.const10 = 0,	
	.const11 = 0,	
	.const12 = 0,	
	.const13 = 0,	
	.const14 = 0,	
	.const15 = 0,	
	.const16 = 0,	
	.const17 = 0,	
};

static MDMiniReport mdminiReport
{
	.id = 0x01,
	.notuse1 = 0x7f,
	.notuse2 = 0x7f,
	.lx = 0x7f,
	.ly = 0x7f,
	.buttons = 0x0f,
	.notuse3 = 0x00,
};

static PCEngineReport pcengineReport
{
	.buttons = 0,
	.hat = 0xf,
	.const0 = 0x80,
	.const1 = 0x80,
	.const2 = 0x80,
	.const3 = 0x80,
	.const4 = 0,	
};

static AstroReport astroReport
{
	.id = 1,
	.notuse1 = 0x7f,
	.notuse2 = 0x7f,
	.lx = 0x7f,
	.ly = 0x7f,
	.buttons = 0xf,
	.notuse3 = 0,	
};

static EgretReport egretReport
{
	.buttons = 0,
	.lx = EGRET_JOYSTICK_MID,
	.ly = EGRET_JOYSTICK_MID,
};

static PSClassicReport psClassicReport
{
	.buttons = 0x0014
};

static XboxOriginalReport xboxOriginalReport
{
    .dButtons = 0,
    .A = 0,
    .B = 0,
    .X = 0,
    .Y = 0,
    .BLACK = 0,
    .WHITE = 0,
    .L = 0,
    .R = 0,
    .leftStickX = 0,
    .leftStickY = 0,
    .rightStickX = 0,
    .rightStickY = 0,
};

#define XBONE_KEEPALIVE_TIMER 15000

static XboxOneGamepad_Data_t xboneReport
{
	.sync = 0,
	.guide = 0,
	.start = 0,
	.back = 0,
	.a = 0,
	.b = 0,
	.x = 0,
	.y = 0,
	.dpadUp = 0,
	.dpadDown = 0,
	.dpadLeft = 0,
	.dpadRight = 0,
	.leftShoulder = 0,
	.rightShoulder = 0,
	.leftThumbClick = 0,
	.rightThumbClick = 0,
	.leftTrigger = 0,
	.rightTrigger = 0,
	.leftStickX = GAMEPAD_JOYSTICK_MID,
	.leftStickY = GAMEPAD_JOYSTICK_MID,
	.rightStickX = GAMEPAD_JOYSTICK_MID,
	.rightStickY = GAMEPAD_JOYSTICK_MID,
	.reserved = {}
};

static uint16_t xboneReportSize;

// Xbox One & PS4/PS5 Timing
static TouchpadData touchpadData;

static KeyboardReport keyboardReport
{
	.keycode = { 0 },
	.multimedia = 0
};

Gamepad::Gamepad() :
	debouncer()
	, options(Storage::getInstance().getGamepadOptions())
	, hotkeyOptions(Storage::getInstance().getHotkeyOptions())
{}

void Gamepad::setup()
{
	// Configure pin mapping
	GpioAction* pinMappings = Storage::getInstance().getProfilePinMappings();
	const GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();

	mapDpadUp    = new GamepadButtonMapping(GAMEPAD_MASK_UP);
	mapDpadDown  = new GamepadButtonMapping(GAMEPAD_MASK_DOWN);
	mapDpadLeft  = new GamepadButtonMapping(GAMEPAD_MASK_LEFT);
	mapDpadRight = new GamepadButtonMapping(GAMEPAD_MASK_RIGHT);
	mapButtonB1  = new GamepadButtonMapping(GAMEPAD_MASK_B1);
	mapButtonB2  = new GamepadButtonMapping(GAMEPAD_MASK_B2);
	mapButtonB3  = new GamepadButtonMapping(GAMEPAD_MASK_B3);
	mapButtonB4  = new GamepadButtonMapping(GAMEPAD_MASK_B4);
	mapButtonL1  = new GamepadButtonMapping(GAMEPAD_MASK_L1);
	mapButtonR1  = new GamepadButtonMapping(GAMEPAD_MASK_R1);
	mapButtonL2  = new GamepadButtonMapping(GAMEPAD_MASK_L2);
	mapButtonR2  = new GamepadButtonMapping(GAMEPAD_MASK_R2);
	mapButtonS1  = new GamepadButtonMapping(GAMEPAD_MASK_S1);
	mapButtonS2  = new GamepadButtonMapping(GAMEPAD_MASK_S2);
	mapButtonL3  = new GamepadButtonMapping(GAMEPAD_MASK_L3);
	mapButtonR3  = new GamepadButtonMapping(GAMEPAD_MASK_R3);
	mapButtonA1  = new GamepadButtonMapping(GAMEPAD_MASK_A1);
	mapButtonA2  = new GamepadButtonMapping(GAMEPAD_MASK_A2);
	mapButtonFn  = new GamepadButtonMapping(AUX_MASK_FUNCTION);

	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		if (pinMappings[pin] > 0)
		{
			gpio_init(pin);             // Initialize pin
			gpio_set_dir(pin, GPIO_IN); // Set as INPUT
			gpio_pull_up(pin);          // Set as PULLUP
			switch (pinMappings[pin]) {
				case GpioAction::BUTTON_PRESS_UP:	mapDpadUp->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_DOWN:	mapDpadDown->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_LEFT:	mapDpadLeft->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_RIGHT:	mapDpadRight->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_B1:	mapButtonB1->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_B2:	mapButtonB2->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_B3:	mapButtonB3->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_B4:	mapButtonB4->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_L1:	mapButtonL1->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_R1:	mapButtonR1->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_L2:	mapButtonL2->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_R2:	mapButtonR2->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_S1:	mapButtonS1->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_S2:	mapButtonS2->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_L3:	mapButtonL3->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_R3:	mapButtonR3->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_A1:	mapButtonA1->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_A2:	mapButtonA2->pinMask |= 1 << pin; break;
				case GpioAction::BUTTON_PRESS_FN:	mapButtonFn->pinMask |= 1 << pin; break;
				default:				break;
			}
		}
	}

	// setup PS5 compatibility
	PS4Data::getInstance().ps4ControllerType = gamepadOptions.ps4ControllerType;
	last_report_counter = 0;
	last_axis_counter = 0;

	// Xbox One Keep-Alive
	keep_alive_timer = to_ms_since_boot(get_absolute_time());
	keep_alive_sequence = 1;
	virtual_keycode_sequence = 0;
	xb1_guide_pressed = false;
	xboneReportSize = sizeof(XboxOneGamepad_Data_t);
}

/**
 * @brief Undo setup().
 */
void Gamepad::reinit()
{
	delete mapDpadUp;
	delete mapDpadDown;
	delete mapDpadLeft;
	delete mapDpadRight;
	delete mapButtonB1;
	delete mapButtonB2;
	delete mapButtonB3;
	delete mapButtonB4;
	delete mapButtonL1;
	delete mapButtonR1;
	delete mapButtonL2;
	delete mapButtonR2;
	delete mapButtonS1;
	delete mapButtonS2;
	delete mapButtonL3;
	delete mapButtonR3;
	delete mapButtonA1;
	delete mapButtonA2;
	delete mapButtonFn;

	// reinitialize pin mappings
	this->setup();
}

void Gamepad::process()
{
	memcpy(&rawState, &state, sizeof(GamepadState));
	// Get the midpoint value for the current mode
	uint16_t joystickMid = GetJoystickMidValue(options.inputMode);

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

	// 4-way before SOCD, might have better history without losing any coherent functionality
	if (options.fourWayMode) {
		state.dpad = filterToFourWayMode(state.dpad);
	}

	state.dpad = runSOCDCleaner(resolveSOCDMode(options), state.dpad);

	switch (options.dpadMode)
	{
		case DpadMode::DPAD_MODE_LEFT_ANALOG:
			if (!hasRightAnalogStick) {
				state.rx = joystickMid;
				state.ry = joystickMid;
			}
			state.lx = dpadToAnalogX(state.dpad, options.inputMode);
			state.ly = dpadToAnalogY(state.dpad, options.inputMode);
			state.dpad = 0;
			break;

		case DpadMode::DPAD_MODE_RIGHT_ANALOG:
			if (!hasLeftAnalogStick) {
				state.lx = joystickMid;
				state.ly = joystickMid;
			}
			state.rx = dpadToAnalogX(state.dpad, options.inputMode);
			state.ry = dpadToAnalogY(state.dpad, options.inputMode);
			state.dpad = 0;
			break;

		default:
			if (!hasLeftAnalogStick) {
				state.lx = joystickMid;
				state.ly = joystickMid;
			}
			if (!hasRightAnalogStick) {
				state.rx = joystickMid;
				state.ry = joystickMid;
			}
			break;
	}
}

void Gamepad::read()
{
	// Need to invert since we're using pullups
	Mask_t values = ~gpio_get_all();
	// Get the midpoint value for the current mode
	uint16_t joystickMid = GetJoystickMidValue(options.inputMode);

	state.aux = 0
		| (values & mapButtonFn->pinMask)   ? mapButtonFn->buttonMask : 0;

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

	state.lx = joystickMid;
	state.ly = joystickMid;
	state.rx = joystickMid;
	state.ry = joystickMid;
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
	else if (pressedHotkey(hotkeyOptions.hotkey13))	action = selectHotkey(hotkeyOptions.hotkey13);
	else if (pressedHotkey(hotkeyOptions.hotkey14))	action = selectHotkey(hotkeyOptions.hotkey14);
	else if (pressedHotkey(hotkeyOptions.hotkey15))	action = selectHotkey(hotkeyOptions.hotkey15);
	else if (pressedHotkey(hotkeyOptions.hotkey16))	action = selectHotkey(hotkeyOptions.hotkey16);
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
		case HOTKEY_HOME_BUTTON       : state.buttons |= GAMEPAD_MASK_A1; break;
		case HOTKEY_L3_BUTTON         : state.buttons |= GAMEPAD_MASK_L3; break;
		case HOTKEY_R3_BUTTON         : state.buttons |= GAMEPAD_MASK_R3; break;
		case HOTKEY_B1_BUTTON         : state.buttons |= GAMEPAD_MASK_B1; break;
		case HOTKEY_B2_BUTTON         : state.buttons |= GAMEPAD_MASK_B2; break;
		case HOTKEY_B3_BUTTON         : state.buttons |= GAMEPAD_MASK_B3; break;
		case HOTKEY_B4_BUTTON         : state.buttons |= GAMEPAD_MASK_B4; break;
		case HOTKEY_L1_BUTTON         : state.buttons |= GAMEPAD_MASK_L1; break;
		case HOTKEY_R1_BUTTON         : state.buttons |= GAMEPAD_MASK_R1; break;
		case HOTKEY_L2_BUTTON         : state.buttons |= GAMEPAD_MASK_L2; break;
		case HOTKEY_R2_BUTTON         : state.buttons |= GAMEPAD_MASK_R2; break;
		case HOTKEY_S1_BUTTON         : state.buttons |= GAMEPAD_MASK_S1; break;
		case HOTKEY_S2_BUTTON         : state.buttons |= GAMEPAD_MASK_S2; break;
		case HOTKEY_A1_BUTTON         : state.buttons |= GAMEPAD_MASK_A1; break;
		case HOTKEY_A2_BUTTON         : state.buttons |= GAMEPAD_MASK_A2; break;
		case HOTKEY_SOCD_UP_PRIORITY  : options.socdMode = SOCD_MODE_UP_PRIORITY; reqSave = true; break;
		case HOTKEY_SOCD_NEUTRAL      : options.socdMode = SOCD_MODE_NEUTRAL; reqSave = true; break;
		case HOTKEY_SOCD_LAST_INPUT   : options.socdMode = SOCD_MODE_SECOND_INPUT_PRIORITY; reqSave = true; break;
		case HOTKEY_SOCD_FIRST_INPUT  : options.socdMode = SOCD_MODE_FIRST_INPUT_PRIORITY;  reqSave = true;break;
		case HOTKEY_SOCD_BYPASS       : options.socdMode = SOCD_MODE_BYPASS; reqSave = true; break;
		case HOTKEY_REBOOT_DEFAULT    : System::reboot(System::BootMode::DEFAULT); break;
		case HOTKEY_CAPTURE_BUTTON    : state.buttons |= GAMEPAD_MASK_A2; break;
		case HOTKEY_TOUCHPAD_BUTTON   : state.buttons |= GAMEPAD_MASK_A2; break;				
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
				Storage::getInstance().setProfile(1);
				userRequestedReinit = true;
				reqSave = true;
			}
			break;
		case HOTKEY_LOAD_PROFILE_2:
			if (action != lastAction) {
				Storage::getInstance().setProfile(2);
				userRequestedReinit = true;
				reqSave = true;
			}
			break;
		case HOTKEY_LOAD_PROFILE_3:
			if (action != lastAction) {
				Storage::getInstance().setProfile(3);
				userRequestedReinit = true;
				reqSave = true;
			}
			break;
		case HOTKEY_LOAD_PROFILE_4:
			if (action != lastAction) {
				Storage::getInstance().setProfile(4);
				userRequestedReinit = true;
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

		case INPUT_MODE_XBONE:
			return getXBOneReport();

		case INPUT_MODE_KEYBOARD:
			return getKeyboardReport();

		case INPUT_MODE_NEOGEO:
			return getNeogeoReport();

		case INPUT_MODE_MDMINI:
			return getMDMiniReport();

		case INPUT_MODE_PCEMINI:
			return getPCEngineReport();

		case INPUT_MODE_EGRET:
			return getEgretReport();

		case INPUT_MODE_ASTRO:
			return getAstroReport();

		case INPUT_MODE_PSCLASSIC:
			return getPSClassicReport();

		case INPUT_MODE_XBOXORIGINAL:
			return getXboxOriginalReport();

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

		case INPUT_MODE_XBONE:
			return xboneReportSize;

		case INPUT_MODE_KEYBOARD:
			return sizeof(KeyboardReport);

		case INPUT_MODE_NEOGEO:
			return sizeof(NeogeoReport);

		case INPUT_MODE_MDMINI:
			return sizeof(MDMiniReport);

		case INPUT_MODE_PCEMINI:
			return sizeof(PCEngineReport);

		case INPUT_MODE_EGRET:
			return sizeof(EgretReport);

		case INPUT_MODE_ASTRO:
			return sizeof(AstroReport);

		case INPUT_MODE_PSCLASSIC:
			return sizeof(PSClassicReport);

		case INPUT_MODE_XBOXORIGINAL:
			return sizeof(XboxOriginalReport);

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
		xinputReport.lt = pressedL2() ? 0xFF : state.lt;
		xinputReport.rt = pressedR2() ? 0xFF : state.rt;
	}
	else
	{
		xinputReport.lt = pressedL2() ? 0xFF : 0;
		xinputReport.rt = pressedR2() ? 0xFF : 0;
	}

	return &xinputReport;
}

static uint8_t xb1_guide_on[] = { 0x01, 0x5b };
static uint8_t xb1_guide_off[] = { 0x00, 0x5b };
static uint8_t xboneIdle[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
					   0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
					   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

// On Send report Success
void Gamepad::sendReportSuccess() {
	switch( (options.inputMode) ) {
		case INPUT_MODE_PS4:
			last_report_counter = (last_report_counter+1) & 0x3F; 	// 6-bit number roll-over
			last_axis_counter = last_axis_counter++; 				// this can roll over on 16-bit
			break;
		case INPUT_MODE_XBONE:
			if ( xboneReport.Header.command == GIP_KEEPALIVE) {
				keep_alive_timer = to_ms_since_boot(get_absolute_time());
				keep_alive_sequence++; // will rollover
				if ( keep_alive_sequence == 0 )
					keep_alive_sequence = 1;
			} else if ( xboneReport.Header.command == GIP_INPUT_REPORT ) {
				// Successfully sent report, actually increment last report counter!
				if ( memcmp(&last_report[4], &((uint8_t*)&xboneReport)[4], xboneReportSize-4) != 0) {
					last_report_counter++;
					if (last_report_counter == 0)
						last_report_counter = 1;
					memcpy(last_report, &xboneReport, xboneReportSize);
				}
			}
			break;
		default:
			break;
	};
}

XboxOneGamepad_Data_t *Gamepad::getXBOneReport()
{
	// No input until auth is ready
	if ( XboxOneData::getInstance().getAuthCompleted() == false ) {
		GIP_HEADER((&xboneReport), GIP_INPUT_REPORT, false, last_report_counter);
		memcpy((void*)&((uint8_t*)&xboneReport)[4], xboneIdle, sizeof(xboneIdle));
		xboneReportSize = sizeof(XboxOneGamepad_Data_t);
		return &xboneReport;
	}

	uint32_t now = to_ms_since_boot(get_absolute_time());
	// Send Keep-Alive every 15 seconds (keep_alive_timer updates if send is successful)
	if ( (now - keep_alive_timer) > XBONE_KEEPALIVE_TIMER) {
		memset(&xboneReport.Header, 0, sizeof(GipHeader_t));
		GIP_HEADER((&xboneReport), GIP_KEEPALIVE, 1, keep_alive_sequence);
		xboneReport.Header.length = 4;
		static uint8_t keepAlive[] = { 0x80, 0x00, 0x00, 0x00 };
		memcpy(&((uint8_t*)&xboneReport)[4], &keepAlive, sizeof(keepAlive));
		xboneReportSize = sizeof(GipHeader_t) + sizeof(keepAlive);
		return &xboneReport;
	}
	
	// Virtual Keycode for Guide Button
	if ( pressedA1() || xb1_guide_pressed == true ) {
		// In a change-state
		if ( (pressedA1() && xb1_guide_pressed == false) ||
			(!pressedA1() && xb1_guide_pressed == true)) {
			virtual_keycode_sequence++; // will rollover
			if ( virtual_keycode_sequence == 0 )
				virtual_keycode_sequence = 1;
			GIP_HEADER((&xboneReport), GIP_VIRTUAL_KEYCODE, 1, virtual_keycode_sequence);
			xboneReport.Header.length = sizeof(xb1_guide_on);
			if ( pressedA1() ) {
				xb1_guide_pressed = true;
				memcpy(&((uint8_t*)&xboneReport)[4], &xb1_guide_on, sizeof(xb1_guide_on));
			} else {
				xb1_guide_pressed = false;
				memcpy(&((uint8_t*)&xboneReport)[4], &xb1_guide_off, sizeof(xb1_guide_off));
			}
		}
		xboneReportSize = sizeof(GipHeader_t) + sizeof(xb1_guide_on);
		return &xboneReport;
	}

	// Only change xbox one input report if we have different inputs!
	XboxOneGamepad_Data_t newInputReport;

	// This is due to our tusb_driver.cpp checking memcmp(last_report, report, size)
	memset(&newInputReport, 0, sizeof(XboxOneGamepad_Data_t));
	GIP_HEADER((&newInputReport), GIP_INPUT_REPORT, false, last_report_counter);

	newInputReport.a = pressedB1();
	newInputReport.b = pressedB2();
	newInputReport.x = pressedB3();
	newInputReport.y = pressedB4();
	newInputReport.leftShoulder = pressedL1();
	newInputReport.rightShoulder = pressedR1();
	newInputReport.leftThumbClick = pressedL3();
	newInputReport.rightThumbClick = pressedR3();
	newInputReport.start = pressedS2();
	newInputReport.back = pressedS1();
	newInputReport.guide = 0; // always 0
	newInputReport.sync = 0; 
	newInputReport.dpadUp = pressedUp();
	newInputReport.dpadDown = pressedDown();
	newInputReport.dpadLeft = pressedLeft();
	newInputReport.dpadRight = pressedRight();

	newInputReport.leftStickX = static_cast<int16_t>(state.lx) + INT16_MIN;
	newInputReport.leftStickY = static_cast<int16_t>(~state.ly) + INT16_MIN;
	newInputReport.rightStickX = static_cast<int16_t>(state.rx) + INT16_MIN;
	newInputReport.rightStickY = static_cast<int16_t>(~state.ry) + INT16_MIN;

	if (hasAnalogTriggers)
	{
		newInputReport.leftTrigger = pressedL2() ? 0x03FF : state.lt;
		newInputReport.rightTrigger = pressedR2() ? 0x03FF : state.rt;
	}
	else
	{
		newInputReport.leftTrigger = pressedL2() ? 0x03FF : 0;
		newInputReport.rightTrigger = pressedR2() ? 0x03FF : 0;
	}

	// We changed inputs since generating our last report, increment last report counter (but don't update until success)
	if ( memcmp(&last_report[4], &((uint8_t*)&newInputReport)[4], sizeof(XboxOneGamepad_Data_t)-4) != 0 ) {
		xboneReportSize = sizeof(XboxOneGamepad_Data_t);
		memcpy(&xboneReport, &newInputReport, xboneReportSize);
		xboneReport.Header.sequence = last_report_counter + 1;
		if ( xboneReport.Header.sequence == 0 )
			xboneReport.Header.sequence = 1;
	}

	return &xboneReport;
}


PS4Report *Gamepad::getPS4Report()
{
	switch (state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        ps4Report.dpad = PS4_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   ps4Report.dpad = PS4_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     ps4Report.dpad = PS4_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: ps4Report.dpad = PS4_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      ps4Report.dpad = PS4_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  ps4Report.dpad = PS4_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      ps4Report.dpad = PS4_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    ps4Report.dpad = PS4_HAT_UPLEFT;    break;
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

	
	if ( !options.ps4ReportHack ) {
		ps4Report.report_counter = last_report_counter; 	// report counter is 6 bits
		ps4Report.axis_timing = last_axis_counter; 			// axis counter is 16 bits
	}

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

PCEngineReport *Gamepad::getPCEngineReport()
{

	switch (state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        pcengineReport.hat = PCENGINE_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   pcengineReport.hat = PCENGINE_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     pcengineReport.hat = PCENGINE_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: pcengineReport.hat = PCENGINE_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      pcengineReport.hat = PCENGINE_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  pcengineReport.hat = PCENGINE_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      pcengineReport.hat = PCENGINE_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    pcengineReport.hat = PCENGINE_HAT_UPLEFT;    break;
		default:                                     pcengineReport.hat = PCENGINE_HAT_NOTHING;   break;
	}

	pcengineReport.buttons = 0x0
		| (pressedB1() ? PCENGINE_MASK_1       : 0)
		| (pressedB2() ? PCENGINE_MASK_2       : 0)
		| (pressedS1() ? PCENGINE_MASK_SELECT  : 0)
		| (pressedS2() ? PCENGINE_MASK_RUN     : 0)
	;

	return &pcengineReport;
}

NeogeoReport *Gamepad::getNeogeoReport()
{

	switch (state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        neogeoReport.hat = NEOGEO_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   neogeoReport.hat = NEOGEO_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     neogeoReport.hat = NEOGEO_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: neogeoReport.hat = NEOGEO_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      neogeoReport.hat = NEOGEO_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  neogeoReport.hat = NEOGEO_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      neogeoReport.hat = NEOGEO_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    neogeoReport.hat = NEOGEO_HAT_UPLEFT;    break;
		default:                                     neogeoReport.hat = NEOGEO_HAT_NOTHING;   break;
	}

	neogeoReport.buttons = 0x0
		| (pressedB3() ? NEOGEO_MASK_A       : 0)
		| (pressedB1() ? NEOGEO_MASK_B       : 0)
		| (pressedB4() ? NEOGEO_MASK_C       : 0)
		| (pressedB2() ? NEOGEO_MASK_D       : 0)
		| (pressedS1() ? NEOGEO_MASK_SELECT  : 0)
		| (pressedS2() ? NEOGEO_MASK_START   : 0)
	;

	return &neogeoReport;
}


MDMiniReport *Gamepad::getMDMiniReport()
{
	mdminiReport.lx = 0x7f;
	mdminiReport.ly = 0x7f;

	if (pressedLeft()) { mdminiReport.lx = MDMINI_MASK_LEFT; }
	if (pressedRight()) { mdminiReport.lx = MDMINI_MASK_RIGHT; }

	if (pressedUp()) { mdminiReport.ly = MDMINI_MASK_UP; }
	if (pressedDown()) { mdminiReport.ly = MDMINI_MASK_DOWN; }

	mdminiReport.buttons = 0x0F
		| (pressedB1()    ? MDMINI_MASK_A     : 0)
		| (pressedB2()    ? MDMINI_MASK_B     : 0)
		| (pressedB3()    ? MDMINI_MASK_X     : 0)
		| (pressedB4()    ? MDMINI_MASK_Y     : 0)
		| (pressedR1()    ? MDMINI_MASK_Z     : 0)
		| (pressedR2()    ? MDMINI_MASK_C     : 0)
		| (pressedS2()    ? MDMINI_MASK_START : 0)
		| (pressedS1()    ? MDMINI_MASK_MODE  : 0)
	;

	return &mdminiReport;
}

AstroReport *Gamepad::getAstroReport()
{
	astroReport.lx = 0x7f;
	astroReport.ly = 0x7f;

	switch (state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        astroReport.lx = ASTRO_JOYSTICK_MID; astroReport.ly = ASTRO_JOYSTICK_MIN; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   astroReport.lx = ASTRO_JOYSTICK_MAX; astroReport.ly = ASTRO_JOYSTICK_MIN; break;
		case GAMEPAD_MASK_RIGHT:                     astroReport.lx = ASTRO_JOYSTICK_MAX; astroReport.ly = ASTRO_JOYSTICK_MID; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: astroReport.lx = ASTRO_JOYSTICK_MAX; astroReport.ly = ASTRO_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_DOWN:                      astroReport.lx = ASTRO_JOYSTICK_MID; astroReport.ly = ASTRO_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  astroReport.lx = ASTRO_JOYSTICK_MIN; astroReport.ly = ASTRO_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_LEFT:                      astroReport.lx = ASTRO_JOYSTICK_MIN; astroReport.ly = ASTRO_JOYSTICK_MID; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    astroReport.lx = ASTRO_JOYSTICK_MIN; astroReport.ly = ASTRO_JOYSTICK_MIN; break;
		default:                                     astroReport.lx = ASTRO_JOYSTICK_MID; astroReport.ly = ASTRO_JOYSTICK_MID; break;
	}


	astroReport.buttons = 0x0F
		| (pressedB1() ? ASTRO_MASK_A       : 0)
		| (pressedB2() ? ASTRO_MASK_B       : 0)
		| (pressedB3() ? ASTRO_MASK_D       : 0)
		| (pressedB4() ? ASTRO_MASK_E       : 0)
		| (pressedR1() ? ASTRO_MASK_F       : 0)
		| (pressedR2() ? ASTRO_MASK_C       : 0)
		| (pressedS1() ? ASTRO_MASK_CREDIT  : 0)
		| (pressedS2() ? ASTRO_MASK_START   : 0)
	;

	return &astroReport;
}

EgretReport *Gamepad::getEgretReport()
{
	switch (state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        egretReport.lx = EGRET_JOYSTICK_MID; egretReport.ly = EGRET_JOYSTICK_MIN; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   egretReport.lx = EGRET_JOYSTICK_MAX; egretReport.ly = EGRET_JOYSTICK_MIN; break;
		case GAMEPAD_MASK_RIGHT:                     egretReport.lx = EGRET_JOYSTICK_MAX; egretReport.ly = EGRET_JOYSTICK_MID; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: egretReport.lx = EGRET_JOYSTICK_MAX; egretReport.ly = EGRET_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_DOWN:                      egretReport.lx = EGRET_JOYSTICK_MID; egretReport.ly = EGRET_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  egretReport.lx = EGRET_JOYSTICK_MIN; egretReport.ly = EGRET_JOYSTICK_MAX; break;
		case GAMEPAD_MASK_LEFT:                      egretReport.lx = EGRET_JOYSTICK_MIN; egretReport.ly = EGRET_JOYSTICK_MID; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    egretReport.lx = EGRET_JOYSTICK_MIN; egretReport.ly = EGRET_JOYSTICK_MIN; break;
		default:                                     egretReport.lx = EGRET_JOYSTICK_MID; egretReport.ly = EGRET_JOYSTICK_MID; break;
	}


	egretReport.buttons = 0
		| (pressedB1() ? EGRET_MASK_A       : 0)
		| (pressedB2() ? EGRET_MASK_B       : 0)
		| (pressedB3() ? EGRET_MASK_D       : 0)
		| (pressedB4() ? EGRET_MASK_E       : 0)
		| (pressedR1() ? EGRET_MASK_F       : 0)
		| (pressedR2() ? EGRET_MASK_C       : 0)
		| (pressedS1() ? EGRET_MASK_CREDIT  : 0)
		| (pressedS2() ? EGRET_MASK_START   : 0)
		| (pressedA1() ? EGRET_MASK_MENU    : 0)
	;

	return &egretReport;
}

PSClassicReport *Gamepad::getPSClassicReport()
{
    psClassicReport.buttons = PSCLASSIC_MASK_CENTER;

	switch (state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        psClassicReport.buttons = PSCLASSIC_MASK_UP; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   psClassicReport.buttons = PSCLASSIC_MASK_UP_RIGHT; break;
		case GAMEPAD_MASK_RIGHT:                     psClassicReport.buttons = PSCLASSIC_MASK_RIGHT; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: psClassicReport.buttons = PSCLASSIC_MASK_DOWN_RIGHT; break;
		case GAMEPAD_MASK_DOWN:                      psClassicReport.buttons = PSCLASSIC_MASK_DOWN; break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  psClassicReport.buttons = PSCLASSIC_MASK_DOWN_LEFT; break;
		case GAMEPAD_MASK_LEFT:                      psClassicReport.buttons = PSCLASSIC_MASK_LEFT; break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    psClassicReport.buttons = PSCLASSIC_MASK_UP_LEFT; break;
		default:                                     psClassicReport.buttons = PSCLASSIC_MASK_CENTER; break;
	}

	psClassicReport.buttons |=
          (pressedS2()    ? PSCLASSIC_MASK_SELECT   : 0)
		| (pressedS1()    ? PSCLASSIC_MASK_START    : 0)
		| (pressedB1()    ? PSCLASSIC_MASK_CROSS    : 0)
		| (pressedB2()    ? PSCLASSIC_MASK_CIRCLE   : 0)
		| (pressedB3()    ? PSCLASSIC_MASK_SQUARE   : 0)
		| (pressedB4()    ? PSCLASSIC_MASK_TRIANGLE : 0)
		| (pressedL1()    ? PSCLASSIC_MASK_L1       : 0)
		| (pressedR1()    ? PSCLASSIC_MASK_R1       : 0)
		| (pressedL2()    ? PSCLASSIC_MASK_L2       : 0)
		| (pressedR2()    ? PSCLASSIC_MASK_R2       : 0)
	;

	return &psClassicReport;
}

XboxOriginalReport *Gamepad::getXboxOriginalReport()
{
    // digital buttons
	xboxOriginalReport.dButtons = 0
		| (pressedUp()    ? XID_DUP    : 0)
		| (pressedDown()  ? XID_DDOWN  : 0)
		| (pressedLeft()  ? XID_DLEFT  : 0)
		| (pressedRight() ? XID_DRIGHT : 0)
		| (pressedS2()    ? XID_START  : 0)
		| (pressedS1()    ? XID_BACK   : 0)
		| (pressedL3()    ? XID_LS     : 0)
		| (pressedR3()    ? XID_RS     : 0)
	;

    // analog buttons - convert to digital
    xboxOriginalReport.A     = (pressedB1() ? 0xFF : 0);
    xboxOriginalReport.B     = (pressedB2() ? 0xFF : 0);
    xboxOriginalReport.X     = (pressedB3() ? 0xFF : 0);
    xboxOriginalReport.Y     = (pressedB4() ? 0xFF : 0);
    xboxOriginalReport.BLACK = (pressedL1() ? 0xFF : 0);
    xboxOriginalReport.WHITE = (pressedR1() ? 0xFF : 0);

    // analog triggers
	if (hasAnalogTriggers)
	{
		xboxOriginalReport.L = pressedL2() ? 0xFF : state.lt;
		xboxOriginalReport.R = pressedR2() ? 0xFF : state.rt;
	}
	else
	{
		xboxOriginalReport.L = pressedL2() ? 0xFF : 0;
		xboxOriginalReport.R = pressedR2() ? 0xFF : 0;
	}

    // analog sticks
	xboxOriginalReport.leftStickX = static_cast<int16_t>(state.lx) + INT16_MIN;
	xboxOriginalReport.leftStickY = static_cast<int16_t>(~state.ly) + INT16_MIN;
	xboxOriginalReport.rightStickX = static_cast<int16_t>(state.rx) + INT16_MIN;
	xboxOriginalReport.rightStickY = static_cast<int16_t>(~state.ry) + INT16_MIN;

    return &xboxOriginalReport;
}
