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

#include "drivermanager.h"
#include "storagemanager.h"
#include "system.h"

// MUST BE DEFINED for mpgs
uint32_t getMillis() {
	return to_ms_since_boot(get_absolute_time());
}

uint64_t getMicro() {
	return to_us_since_boot(get_absolute_time());
}

Gamepad::Gamepad() :
	options(Storage::getInstance().getGamepadOptions())
	, hotkeyOptions(Storage::getInstance().getHotkeyOptions())
{}

void Gamepad::setup()
{
	// Configure pin mapping
	GpioAction* pinMappings = Storage::getInstance().getProfilePinMappings();

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

	lastAction = HOTKEY_NONE;
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
	uint16_t joystickMid = GAMEPAD_JOYSTICK_MID;
	if ( DriverManager::getInstance().getDriver() != nullptr ) {
		joystickMid = DriverManager::getInstance().getDriver()->GetJoystickMidValue();
	}

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
			state.lx = dpadToAnalogX(state.dpad);
			state.ly = dpadToAnalogY(state.dpad);
			state.dpad = 0;
			break;

		case DpadMode::DPAD_MODE_RIGHT_ANALOG:
			if (!hasLeftAnalogStick) {
				state.lx = joystickMid;
				state.ly = joystickMid;
			}
			state.rx = dpadToAnalogX(state.dpad);
			state.ry = dpadToAnalogY(state.dpad);
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
	Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
	
	// Get the midpoint value for the current mode
	uint16_t joystickMid = GAMEPAD_JOYSTICK_MID;
	if ( DriverManager::getInstance().getDriver() != nullptr ) {
		joystickMid = DriverManager::getInstance().getDriver()->GetJoystickMidValue();
	}

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

void Gamepad::save()
{
	Storage::getInstance().save();
}

void Gamepad::hotkey()
{
	if (options.lockHotkeys)
		return;
	
	GamepadHotkey action = HOTKEY_NONE;
	if (pressedHotkey(hotkeyOptions.hotkey01))	    action = selectHotkey(hotkeyOptions.hotkey01);
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
	if ( action != HOTKEY_NONE ) {
		// processHotkeyAction checks lastAction to determine if the action is repeatable or not
		processHotkeyAction(action);
	}
	lastAction = action;
}

void Gamepad::clearState() {
	state.dpad = 0;
	state.buttons = 0;
	state.aux = 0;
	state.lx = GAMEPAD_JOYSTICK_MID;
	state.ly = GAMEPAD_JOYSTICK_MID;
	state.rx = GAMEPAD_JOYSTICK_MID;
	state.ry = GAMEPAD_JOYSTICK_MID;
	state.lt = 0;
	state.rt = 0;
}

/**
 * @brief Take a hotkey action if it hasn't already been taken, modifying state/options appropriately.
 */
void Gamepad::processHotkeyAction(GamepadHotkey action) {
	bool reqSave = false;
	switch (action) {
		case HOTKEY_DPAD_DIGITAL:
			if (action != lastAction) {
				options.dpadMode = DPAD_MODE_DIGITAL;
				reqSave = true;
			}
			break;
		case HOTKEY_DPAD_LEFT_ANALOG:
			if (action != lastAction) {
				options.dpadMode = DPAD_MODE_LEFT_ANALOG;
				reqSave = true;
			}
			break;
		case HOTKEY_DPAD_RIGHT_ANALOG:
			if (action != lastAction) {
				options.dpadMode = DPAD_MODE_RIGHT_ANALOG;
				reqSave = true;
			}
			break;
		case HOTKEY_HOME_BUTTON:
			state.buttons |= GAMEPAD_MASK_A1;
			break;
		case HOTKEY_L3_BUTTON:
			state.buttons |= GAMEPAD_MASK_L3;
			break;
		case HOTKEY_R3_BUTTON:
			state.buttons |= GAMEPAD_MASK_R3;
			break;
		case HOTKEY_B1_BUTTON:
			state.buttons |= GAMEPAD_MASK_B1;
			break;
		case HOTKEY_B2_BUTTON:
			state.buttons |= GAMEPAD_MASK_B2;
			break;
		case HOTKEY_B3_BUTTON:
			state.buttons |= GAMEPAD_MASK_B3;
			break;
		case HOTKEY_B4_BUTTON:
			state.buttons |= GAMEPAD_MASK_B4;
			break;
		case HOTKEY_L1_BUTTON:
			state.buttons |= GAMEPAD_MASK_L1;
			break;
		case HOTKEY_R1_BUTTON:
			state.buttons |= GAMEPAD_MASK_R1;
			break;
		case HOTKEY_L2_BUTTON:
			state.buttons |= GAMEPAD_MASK_L2;
			break;
		case HOTKEY_R2_BUTTON:
			state.buttons |= GAMEPAD_MASK_R2;
			break;
		case HOTKEY_S1_BUTTON:
			state.buttons |= GAMEPAD_MASK_S1;
			break;
		case HOTKEY_S2_BUTTON:
			state.buttons |= GAMEPAD_MASK_S2;
			break;
		case HOTKEY_A1_BUTTON:
			state.buttons |= GAMEPAD_MASK_A1;
			break;
		case HOTKEY_A2_BUTTON:
			state.buttons |= GAMEPAD_MASK_A2;
			break;
		case HOTKEY_SOCD_UP_PRIORITY:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_UP_PRIORITY;
				reqSave = true;
			}
			break;
		case HOTKEY_SOCD_NEUTRAL:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_NEUTRAL;
				reqSave = true;
			}
			break;
		case HOTKEY_SOCD_LAST_INPUT:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_SECOND_INPUT_PRIORITY;
				reqSave = true;
			}
			break;
		case HOTKEY_SOCD_FIRST_INPUT:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_FIRST_INPUT_PRIORITY;
				reqSave = true;
			}
			break;
		case HOTKEY_SOCD_BYPASS:
			if (action != lastAction) {
				options.socdMode = SOCD_MODE_BYPASS;
				reqSave = true;
			}
			break;
		case HOTKEY_REBOOT_DEFAULT:
			System::reboot(System::BootMode::DEFAULT);
			break;
		case HOTKEY_CAPTURE_BUTTON:
			state.buttons |= GAMEPAD_MASK_A2;
			break;
		case HOTKEY_TOUCHPAD_BUTTON:
			state.buttons |= GAMEPAD_MASK_A2;
			break;				
		case HOTKEY_INVERT_X_AXIS:
			if (action != lastAction) {
				options.invertXAxis = !options.invertXAxis;
				reqSave = true;
			}
			break;
		case HOTKEY_INVERT_Y_AXIS:
			if (action != lastAction) {
				options.invertYAxis = !options.invertYAxis;
				reqSave = true;
			}
			break;
		case HOTKEY_TOGGLE_4_WAY_MODE:
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
		default: // Unknown action
			return;
	}

	// only save if requested
	if (reqSave) {
		save();
	}
}
