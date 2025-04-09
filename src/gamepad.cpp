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
{
}

void Gamepad::setup()
{
	// Configure pin mapping
	GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();

	mapDpadUp       = new GamepadButtonMapping(GAMEPAD_MASK_UP);
	mapDpadDown     = new GamepadButtonMapping(GAMEPAD_MASK_DOWN);
	mapDpadLeft     = new GamepadButtonMapping(GAMEPAD_MASK_LEFT);
	mapDpadRight    = new GamepadButtonMapping(GAMEPAD_MASK_RIGHT);
	mapButtonB1     = new GamepadButtonMapping(GAMEPAD_MASK_B1);
	mapButtonB2     = new GamepadButtonMapping(GAMEPAD_MASK_B2);
	mapButtonB3     = new GamepadButtonMapping(GAMEPAD_MASK_B3);
	mapButtonB4     = new GamepadButtonMapping(GAMEPAD_MASK_B4);
	mapButtonL1     = new GamepadButtonMapping(GAMEPAD_MASK_L1);
	mapButtonR1     = new GamepadButtonMapping(GAMEPAD_MASK_R1);
	mapButtonL2     = new GamepadButtonMapping(GAMEPAD_MASK_L2);
	mapButtonR2     = new GamepadButtonMapping(GAMEPAD_MASK_R2);
	mapButtonS1     = new GamepadButtonMapping(GAMEPAD_MASK_S1);
	mapButtonS2     = new GamepadButtonMapping(GAMEPAD_MASK_S2);
	mapButtonL3     = new GamepadButtonMapping(GAMEPAD_MASK_L3);
	mapButtonR3     = new GamepadButtonMapping(GAMEPAD_MASK_R3);
	mapButtonA1     = new GamepadButtonMapping(GAMEPAD_MASK_A1);
	mapButtonA2     = new GamepadButtonMapping(GAMEPAD_MASK_A2);
	mapButtonA3     = new GamepadButtonMapping(GAMEPAD_MASK_A3);
	mapButtonA4     = new GamepadButtonMapping(GAMEPAD_MASK_A4);
	mapButtonE1     = new GamepadButtonMapping(GAMEPAD_MASK_E1);
	mapButtonE2     = new GamepadButtonMapping(GAMEPAD_MASK_E2);
	mapButtonE3     = new GamepadButtonMapping(GAMEPAD_MASK_E3);
	mapButtonE4     = new GamepadButtonMapping(GAMEPAD_MASK_E4);
	mapButtonE5     = new GamepadButtonMapping(GAMEPAD_MASK_E5);
	mapButtonE6     = new GamepadButtonMapping(GAMEPAD_MASK_E6);
	mapButtonE7     = new GamepadButtonMapping(GAMEPAD_MASK_E7);
	mapButtonE8     = new GamepadButtonMapping(GAMEPAD_MASK_E8);
	mapButtonE9     = new GamepadButtonMapping(GAMEPAD_MASK_E9);
	mapButtonE10    = new GamepadButtonMapping(GAMEPAD_MASK_E10);
	mapButtonE11    = new GamepadButtonMapping(GAMEPAD_MASK_E11);
	mapButtonE12    = new GamepadButtonMapping(GAMEPAD_MASK_E12);
	mapButtonFn     = new GamepadButtonMapping(AUX_MASK_FUNCTION);
	mapButtonDP     = new GamepadButtonMapping(SUSTAIN_DP_MODE_DP);
	mapButtonLS     = new GamepadButtonMapping(SUSTAIN_DP_MODE_LS);
	mapButtonRS     = new GamepadButtonMapping(SUSTAIN_DP_MODE_RS);
	mapDigitalUp    = new GamepadButtonMapping(GAMEPAD_MASK_UP);
	mapDigitalDown  = new GamepadButtonMapping(GAMEPAD_MASK_DOWN);
	mapDigitalLeft  = new GamepadButtonMapping(GAMEPAD_MASK_LEFT);
	mapDigitalRight = new GamepadButtonMapping(GAMEPAD_MASK_RIGHT);
	mapAnalogLSXNeg = new GamepadButtonMapping(ANALOG_DIRECTION_LS_X_NEG);
	mapAnalogLSXPos = new GamepadButtonMapping(ANALOG_DIRECTION_LS_X_POS);
	mapAnalogLSYNeg = new GamepadButtonMapping(ANALOG_DIRECTION_LS_Y_NEG);
	mapAnalogLSYPos = new GamepadButtonMapping(ANALOG_DIRECTION_LS_Y_POS);
	mapAnalogRSXNeg = new GamepadButtonMapping(ANALOG_DIRECTION_RS_X_NEG);
	mapAnalogRSXPos = new GamepadButtonMapping(ANALOG_DIRECTION_RS_X_POS);
	mapAnalogRSYNeg = new GamepadButtonMapping(ANALOG_DIRECTION_RS_Y_NEG);
	mapAnalogRSYPos = new GamepadButtonMapping(ANALOG_DIRECTION_RS_Y_POS);
	map48WayMode    = new GamepadButtonMapping(SUSTAIN_4_8_WAY_MODE);

	const auto assignCustomMappingToMaps = [&](GpioMappingInfo mapInfo, Pin_t pin) -> void {
		if (mapDpadUp->buttonMask & mapInfo.customDpadMask)	mapDpadUp->pinMask |= 1 << pin;
		if (mapDpadDown->buttonMask & mapInfo.customDpadMask)	mapDpadDown->pinMask |= 1 << pin;
		if (mapDpadLeft->buttonMask & mapInfo.customDpadMask)	mapDpadLeft->pinMask |= 1 << pin;
		if (mapDpadRight->buttonMask & mapInfo.customDpadMask)	mapDpadRight->pinMask |= 1 << pin;
		if (mapButtonB1->buttonMask & mapInfo.customButtonMask)	mapButtonB1->pinMask |= 1 << pin;
		if (mapButtonB2->buttonMask & mapInfo.customButtonMask)	mapButtonB2->pinMask |= 1 << pin;
		if (mapButtonB3->buttonMask & mapInfo.customButtonMask)	mapButtonB3->pinMask |= 1 << pin;
		if (mapButtonB4->buttonMask & mapInfo.customButtonMask)	mapButtonB4->pinMask |= 1 << pin;
		if (mapButtonL1->buttonMask & mapInfo.customButtonMask)	mapButtonL1->pinMask |= 1 << pin;
		if (mapButtonR1->buttonMask & mapInfo.customButtonMask)	mapButtonR1->pinMask |= 1 << pin;
		if (mapButtonL2->buttonMask & mapInfo.customButtonMask)	mapButtonL2->pinMask |= 1 << pin;
		if (mapButtonR2->buttonMask & mapInfo.customButtonMask)	mapButtonR2->pinMask |= 1 << pin;
		if (mapButtonS1->buttonMask & mapInfo.customButtonMask)	mapButtonS1->pinMask |= 1 << pin;
		if (mapButtonS2->buttonMask & mapInfo.customButtonMask)	mapButtonS2->pinMask |= 1 << pin;
		if (mapButtonL3->buttonMask & mapInfo.customButtonMask)	mapButtonL3->pinMask |= 1 << pin;
		if (mapButtonR3->buttonMask & mapInfo.customButtonMask)	mapButtonR3->pinMask |= 1 << pin;
		if (mapButtonA1->buttonMask & mapInfo.customButtonMask)	mapButtonA1->pinMask |= 1 << pin;
		if (mapButtonA2->buttonMask & mapInfo.customButtonMask)	mapButtonA2->pinMask |= 1 << pin;
		if (mapDigitalUp->buttonMask & mapInfo.customDpadMask)	mapDigitalUp->pinMask |= 1 << pin;
		if (mapDigitalDown->buttonMask & mapInfo.customDpadMask)	mapDigitalDown->pinMask |= 1 << pin;
		if (mapDigitalLeft->buttonMask & mapInfo.customDpadMask)	mapDigitalLeft->pinMask |= 1 << pin;
		if (mapDigitalRight->buttonMask & mapInfo.customDpadMask)	mapDigitalRight->pinMask |= 1 << pin;
	};

	for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++)
	{
		switch (pinMappings[pin].action) {
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
			case GpioAction::BUTTON_PRESS_A3:	mapButtonA3->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_A4:	mapButtonA4->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E1:	mapButtonE1->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E2:	mapButtonE2->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E3:	mapButtonE3->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E4:	mapButtonE4->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E5:	mapButtonE5->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E6:	mapButtonE6->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E7:	mapButtonE7->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E8:	mapButtonE8->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E9:	mapButtonE9->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E10:	mapButtonE10->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E11:	mapButtonE11->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_E12:	mapButtonE12->pinMask |= 1 << pin; break;
			case GpioAction::BUTTON_PRESS_FN:	mapButtonFn->pinMask |= 1 << pin; break;
			case GpioAction::SUSTAIN_DP_MODE_DP:	mapButtonDP->pinMask |= 1 << pin; break;
			case GpioAction::SUSTAIN_DP_MODE_LS:	mapButtonLS->pinMask |= 1 << pin; break;
			case GpioAction::SUSTAIN_DP_MODE_RS:	mapButtonRS->pinMask |= 1 << pin; break;
			case GpioAction::CUSTOM_BUTTON_COMBO:	assignCustomMappingToMaps(pinMappings[pin], pin); break;
			case GpioAction::DIGITAL_DIRECTION_UP:	mapDigitalUp->pinMask |= 1 << pin; break;
			case GpioAction::DIGITAL_DIRECTION_DOWN:	mapDigitalDown->pinMask |= 1 << pin; break;
			case GpioAction::DIGITAL_DIRECTION_LEFT:	mapDigitalLeft->pinMask |= 1 << pin; break;
			case GpioAction::DIGITAL_DIRECTION_RIGHT:	mapDigitalRight->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_X_NEG:	mapAnalogLSXNeg->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_X_POS:	mapAnalogLSXPos->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_Y_NEG:	mapAnalogLSYNeg->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_LS_Y_POS:	mapAnalogLSYPos->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_X_NEG:	mapAnalogRSXNeg->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_X_POS:	mapAnalogRSXPos->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_Y_NEG:	mapAnalogRSYNeg->pinMask |= 1 << pin; break;
			case GpioAction::ANALOG_DIRECTION_RS_Y_POS:	mapAnalogRSYPos->pinMask |= 1 << pin; break;
			case GpioAction::SUSTAIN_4_8_WAY_MODE:	map48WayMode->pinMask |= 1 << pin; break;
			default:				break;
		}
	}

	// Define our hotkey array
	hotkeys[0] = hotkeyOptions.hotkey01;
	hotkeys[1] = hotkeyOptions.hotkey02;
	hotkeys[2] = hotkeyOptions.hotkey03;
	hotkeys[3] = hotkeyOptions.hotkey04;
	hotkeys[4] = hotkeyOptions.hotkey05;
	hotkeys[5] = hotkeyOptions.hotkey06;
	hotkeys[6] = hotkeyOptions.hotkey07;
	hotkeys[7] = hotkeyOptions.hotkey08;
	hotkeys[8] = hotkeyOptions.hotkey09;
	hotkeys[9] = hotkeyOptions.hotkey10;
	hotkeys[10] = hotkeyOptions.hotkey11;
	hotkeys[11] = hotkeyOptions.hotkey12;
	hotkeys[12] = hotkeyOptions.hotkey13;
	hotkeys[13] = hotkeyOptions.hotkey14;
	hotkeys[14] = hotkeyOptions.hotkey15;
	hotkeys[15] = hotkeyOptions.hotkey16;
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
	delete mapButtonA3;
	delete mapButtonA4;
	delete mapButtonE1;
	delete mapButtonE2;
	delete mapButtonE3;
	delete mapButtonE4;
	delete mapButtonE5;
	delete mapButtonE6;
	delete mapButtonE7;
	delete mapButtonE8;
	delete mapButtonE9;
	delete mapButtonE10;
	delete mapButtonE11;
	delete mapButtonE12;
	delete mapButtonFn;
	delete mapButtonDP;
	delete mapButtonLS;
	delete mapButtonRS;
	delete mapDigitalUp;
	delete mapDigitalDown;
	delete mapDigitalLeft;
	delete mapDigitalRight;
	delete mapAnalogLSXNeg;
	delete mapAnalogLSXPos;
	delete mapAnalogLSYNeg;
	delete mapAnalogLSYPos;
	delete mapAnalogRSXNeg;
	delete mapAnalogRSXPos;
	delete mapAnalogRSYNeg;
	delete mapAnalogRSYPos;
	delete map48WayMode;

	// reinitialize pin mappings
	this->setup();
}

void Gamepad::process()
{
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
	if (options.fourWayMode ^ map48WayModeToggle) {
		state.dpad = filterToFourWayMode(state.dpad);
	}

	// hold current dpad state regardless of input
	uint8_t dpadOriginal = state.dpad;

	// stash digital-only dpad state for later
	uint8_t dpadOnlyMask = ((dpadOriginal & 0xF0) >> 4);

	// and mask out the mode-specific mask
	uint8_t dpadModeMask = (dpadOriginal & 0x0F);

	// set dpad back to dpad mode-specific state
	state.dpad = dpadModeMask;

	// and join both states before cleanup, but only if the mode is dpad
	// combine dpad mode directions and standalone digital directions
	if (activeDpadMode == DpadMode::DPAD_MODE_DIGITAL) {
		state.dpad |= dpadOnlyMask;
	}

	// clean up after yourself. nobody likes bad inputs.
	state.dpad = runSOCDCleaner(resolveSOCDMode(options), state.dpad);

	// since analog modes only care about the dpad mode inputs, set the dpad state to digital only dpad values
	switch (activeDpadMode)
	{
		case DpadMode::DPAD_MODE_LEFT_ANALOG:
			state.lx = dpadToAnalogX(state.dpad);
			state.ly = dpadToAnalogY(state.dpad);
			state.dpad = dpadOnlyMask;
			break;

		case DpadMode::DPAD_MODE_RIGHT_ANALOG:
			state.rx = dpadToAnalogX(state.dpad);
			state.ry = dpadToAnalogY(state.dpad);
			state.dpad = dpadOnlyMask;
			break;

		default:
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
		| ((values & mapDpadUp->pinMask)       ? mapDpadUp->buttonMask              : 0)
		| ((values & mapDpadDown->pinMask)     ? mapDpadDown->buttonMask            : 0)
		| ((values & mapDpadLeft->pinMask)     ? mapDpadLeft->buttonMask            : 0)
		| ((values & mapDpadRight->pinMask)    ? mapDpadRight->buttonMask           : 0)
		| ((values & mapDigitalUp->pinMask)    ? (mapDigitalUp->buttonMask << 4)    : 0)
		| ((values & mapDigitalDown->pinMask)  ? (mapDigitalDown->buttonMask << 4)  : 0)
		| ((values & mapDigitalLeft->pinMask)  ? (mapDigitalLeft->buttonMask << 4)  : 0)
		| ((values & mapDigitalRight->pinMask) ? (mapDigitalRight->buttonMask << 4) : 0)
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
		| ((values & mapButtonA3->pinMask)  ? mapButtonA3->buttonMask  : 0)
		| ((values & mapButtonA4->pinMask)  ? mapButtonA4->buttonMask  : 0)
		| ((values & mapButtonE1->pinMask)  ? mapButtonE1->buttonMask  : 0)
		| ((values & mapButtonE2->pinMask)  ? mapButtonE2->buttonMask  : 0)
		| ((values & mapButtonE3->pinMask)  ? mapButtonE3->buttonMask  : 0)
		| ((values & mapButtonE4->pinMask)  ? mapButtonE4->buttonMask  : 0)
		| ((values & mapButtonE5->pinMask)  ? mapButtonE5->buttonMask  : 0)
		| ((values & mapButtonE6->pinMask)  ? mapButtonE6->buttonMask  : 0)
		| ((values & mapButtonE7->pinMask)  ? mapButtonE7->buttonMask  : 0)
		| ((values & mapButtonE8->pinMask)  ? mapButtonE8->buttonMask  : 0)
		| ((values & mapButtonE9->pinMask)  ? mapButtonE9->buttonMask  : 0)
		| ((values & mapButtonE10->pinMask) ? mapButtonE10->buttonMask : 0)
		| ((values & mapButtonE11->pinMask) ? mapButtonE11->buttonMask : 0)
		| ((values & mapButtonE12->pinMask) ? mapButtonE12->buttonMask : 0)
	;

	// set the effective dpad mode based on settings + overrides
	if (values & mapButtonDP->pinMask)	activeDpadMode = DpadMode::DPAD_MODE_DIGITAL;
	else if (values & mapButtonLS->pinMask)	activeDpadMode = DpadMode::DPAD_MODE_LEFT_ANALOG;
	else if (values & mapButtonRS->pinMask)	activeDpadMode = DpadMode::DPAD_MODE_RIGHT_ANALOG;
	else					activeDpadMode = options.dpadMode;

	map48WayModeToggle = (values & map48WayMode->pinMask);

	if (values & mapAnalogLSXNeg->pinMask) {
		state.lx = GAMEPAD_JOYSTICK_MIN;
	} else if (values & mapAnalogLSXPos->pinMask) {
		state.lx = GAMEPAD_JOYSTICK_MAX;
	} else {
		state.lx = joystickMid;
	}
	if (values & mapAnalogLSYNeg->pinMask) {
		state.ly = GAMEPAD_JOYSTICK_MIN;
	} else if (values & mapAnalogLSYPos->pinMask) {
		state.ly = GAMEPAD_JOYSTICK_MAX;
	} else {
		state.ly = joystickMid;
	}

	if (values & mapAnalogRSXNeg->pinMask) {
		state.rx = GAMEPAD_JOYSTICK_MIN;
	} else if (values & mapAnalogRSXPos->pinMask) {
		state.rx = GAMEPAD_JOYSTICK_MAX;
	} else {
		state.rx = joystickMid;
	}
	if (values & mapAnalogRSYNeg->pinMask) {
		state.ry = GAMEPAD_JOYSTICK_MIN;
	} else if (values & mapAnalogRSYPos->pinMask) {
		state.ry = GAMEPAD_JOYSTICK_MAX;
	} else {
		state.ry = joystickMid;
	}

	state.lt = 0;
	state.rt = 0;
}

void Gamepad::hotkey() {
	if (options.lockHotkeys)
		return;

	// Look for a hot-key
	bool hasHotkey = false;
	for(int i = 0; i < 16; i++) {
		if (pressedHotkey(hotkeys[i])) {
			processHotkeyAction(selectHotkey(hotkeys[i]));
			hasHotkey = true;
		}
	}
	if (hasHotkey == false ) {
		lastAction = HOTKEY_NONE;
	}
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

void Gamepad::clearRumbleState() {
	auxState.haptics.leftActuator.active = false;
	auxState.haptics.leftActuator.intensity = 0;
	auxState.haptics.rightActuator.active = false;
	auxState.haptics.rightActuator.intensity = 0;
	auxState.haptics.leftTrigger.active = false;
	auxState.haptics.leftTrigger.intensity = 0;
	auxState.haptics.rightTrigger.active = false;
	auxState.haptics.rightTrigger.intensity = 0;
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
		case HOTKEY_A3_BUTTON:
			state.buttons |= GAMEPAD_MASK_A3;
			break;
		case HOTKEY_A4_BUTTON:
			state.buttons |= GAMEPAD_MASK_A4;
			break;
		case HOTKEY_DPAD_UP:
			state.dpad |= GAMEPAD_MASK_UP;
			break;
		case HOTKEY_DPAD_DOWN:
			state.dpad |= GAMEPAD_MASK_DOWN;
			break;
		case HOTKEY_DPAD_LEFT:
			state.dpad |= GAMEPAD_MASK_LEFT;
			break;
		case HOTKEY_DPAD_RIGHT:
			state.dpad |= GAMEPAD_MASK_RIGHT;
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
			if (action != lastAction) {
				System::reboot(System::BootMode::DEFAULT);
			}
			break;
		case HOTKEY_SAVE_CONFIG:
			if (action != lastAction) {
				Storage::getInstance().save(true);
			}
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
				if (Storage::getInstance().setProfile(1)) {
					userRequestedReinit = true;
					reqSave = true;
				}
			}
			break;
		case HOTKEY_LOAD_PROFILE_2:
			if (action != lastAction) {
				if (Storage::getInstance().setProfile(2)) {
					userRequestedReinit = true;
					reqSave = true;
				}
			}
			break;
		case HOTKEY_LOAD_PROFILE_3:
			if (action != lastAction) {
				if (Storage::getInstance().setProfile(3)) {
					userRequestedReinit = true;
					reqSave = true;
				}
			}
			break;
		case HOTKEY_LOAD_PROFILE_4:
			if (action != lastAction) {
				if (Storage::getInstance().setProfile(4)) {
					userRequestedReinit = true;
					reqSave = true;
				}
			}
			break;
		case HOTKEY_NEXT_PROFILE:
			if (action != lastAction) {
				Storage::getInstance().nextProfile();
				userRequestedReinit = true;
				reqSave = true;
			}
			break;
		case HOTKEY_PREVIOUS_PROFILE:
			if (action != lastAction) {
				Storage::getInstance().previousProfile();
				userRequestedReinit = true;
				reqSave = true;
			}
			break;
		case HOTKEY_MENU_NAV_UP:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_UP));
            }
			break;
		case HOTKEY_MENU_NAV_DOWN:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_DOWN));
            }
			break;
		case HOTKEY_MENU_NAV_LEFT:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_LEFT));
            }
			break;
		case HOTKEY_MENU_NAV_RIGHT:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_RIGHT));
            }
			break;
		case HOTKEY_MENU_NAV_SELECT:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_SELECT));
            }
			break;
		case HOTKEY_MENU_NAV_BACK:
			if (action != lastAction) {
                EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_BACK));
            }
			break;
		case HOTKEY_MENU_NAV_TOGGLE:
			if (action != lastAction) {
				EventManager::getInstance().triggerEvent(new GPMenuNavigateEvent(GpioAction::MENU_NAVIGATION_TOGGLE));
			}
			break;
		default: // Unknown action
			break;
	}

	// only save if requested
	if (reqSave) {
		EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
	}

	lastAction = action;
}
