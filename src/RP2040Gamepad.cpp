/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "pico/stdlib.h"

#include "usb_driver.h"
#include "definitions/BoardConfig.h"
#include "Gamepad.h"
#include "GamepadStorage.h"
#include "definitions/BoardConfig.h"

GamepadButtonMapping GamepadClass::mapDpadUp    = { .port = 0, .pin = PIN_DPAD_UP,    .pinMask = (1 << PIN_DPAD_UP),    .buttonMask = GAMEPAD_MASK_UP,    .ledPos = LEDS_DPAD_UP    };
GamepadButtonMapping GamepadClass::mapDpadDown  = { .port = 0, .pin = PIN_DPAD_DOWN,  .pinMask = (1 << PIN_DPAD_DOWN),  .buttonMask = GAMEPAD_MASK_DOWN,  .ledPos = LEDS_DPAD_DOWN  };
GamepadButtonMapping GamepadClass::mapDpadLeft  = { .port = 0, .pin = PIN_DPAD_LEFT,  .pinMask = (1 << PIN_DPAD_LEFT),  .buttonMask = GAMEPAD_MASK_LEFT,  .ledPos = LEDS_DPAD_LEFT  };
GamepadButtonMapping GamepadClass::mapDpadRight = { .port = 0, .pin = PIN_DPAD_RIGHT, .pinMask = (1 << PIN_DPAD_RIGHT), .buttonMask = GAMEPAD_MASK_RIGHT, .ledPos = LEDS_DPAD_RIGHT };
GamepadButtonMapping GamepadClass::mapButton01  = { .port = 0, .pin = PIN_BUTTON_01,  .pinMask = (1 << PIN_BUTTON_01),  .buttonMask = GAMEPAD_MASK_B1,    .ledPos = LEDS_BUTTON_01  };
GamepadButtonMapping GamepadClass::mapButton02  = { .port = 0, .pin = PIN_BUTTON_02,  .pinMask = (1 << PIN_BUTTON_02),  .buttonMask = GAMEPAD_MASK_B2,    .ledPos = LEDS_BUTTON_02  };
GamepadButtonMapping GamepadClass::mapButton03  = { .port = 0, .pin = PIN_BUTTON_03,  .pinMask = (1 << PIN_BUTTON_03),  .buttonMask = GAMEPAD_MASK_B3,    .ledPos = LEDS_BUTTON_03  };
GamepadButtonMapping GamepadClass::mapButton04  = { .port = 0, .pin = PIN_BUTTON_04,  .pinMask = (1 << PIN_BUTTON_04),  .buttonMask = GAMEPAD_MASK_B4,    .ledPos = LEDS_BUTTON_04  };
GamepadButtonMapping GamepadClass::mapButton05  = { .port = 0, .pin = PIN_BUTTON_05,  .pinMask = (1 << PIN_BUTTON_05),  .buttonMask = GAMEPAD_MASK_L1,    .ledPos = LEDS_BUTTON_05  };
GamepadButtonMapping GamepadClass::mapButton06  = { .port = 0, .pin = PIN_BUTTON_06,  .pinMask = (1 << PIN_BUTTON_06),  .buttonMask = GAMEPAD_MASK_R1,    .ledPos = LEDS_BUTTON_06  };
GamepadButtonMapping GamepadClass::mapButton07  = { .port = 0, .pin = PIN_BUTTON_07,  .pinMask = (1 << PIN_BUTTON_07),  .buttonMask = GAMEPAD_MASK_L2,    .ledPos = LEDS_BUTTON_07  };
GamepadButtonMapping GamepadClass::mapButton08  = { .port = 0, .pin = PIN_BUTTON_08,  .pinMask = (1 << PIN_BUTTON_08),  .buttonMask = GAMEPAD_MASK_R2,    .ledPos = LEDS_BUTTON_08  };
GamepadButtonMapping GamepadClass::mapButton09  = { .port = 0, .pin = PIN_BUTTON_09,  .pinMask = (1 << PIN_BUTTON_09),  .buttonMask = GAMEPAD_MASK_S1,    .ledPos = LEDS_BUTTON_09  };
GamepadButtonMapping GamepadClass::mapButton10  = { .port = 0, .pin = PIN_BUTTON_10,  .pinMask = (1 << PIN_BUTTON_10),  .buttonMask = GAMEPAD_MASK_S2,    .ledPos = LEDS_BUTTON_10  };
GamepadButtonMapping GamepadClass::mapButton11  = { .port = 0, .pin = PIN_BUTTON_11,  .pinMask = (1 << PIN_BUTTON_11),  .buttonMask = GAMEPAD_MASK_L3,    .ledPos = LEDS_BUTTON_11  };
GamepadButtonMapping GamepadClass::mapButton12  = { .port = 0, .pin = PIN_BUTTON_12,  .pinMask = (1 << PIN_BUTTON_12),  .buttonMask = GAMEPAD_MASK_R3,    .ledPos = LEDS_BUTTON_12  };
GamepadButtonMapping GamepadClass::mapButton13  = { .port = 0, .pin = PIN_BUTTON_13,  .pinMask = (1 << PIN_BUTTON_13),  .buttonMask = GAMEPAD_MASK_A1,    .ledPos = LEDS_BUTTON_13  };
GamepadButtonMapping GamepadClass::mapButton14  = { .port = 0, .pin = PIN_BUTTON_14,  .pinMask = (1 << PIN_BUTTON_14),  .buttonMask = GAMEPAD_MASK_A2,    .ledPos = LEDS_BUTTON_14  };

void GamepadClass::setup()
{
	GamepadButtonMapping *gamepadMappings[] =
	{
		&mapDpadUp,   &mapDpadDown, &mapDpadLeft, &mapDpadRight,
		&mapButton01, &mapButton02, &mapButton03, &mapButton04,
		&mapButton05, &mapButton06, &mapButton07, &mapButton08,
		&mapButton09, &mapButton10, &mapButton11, &mapButton12,
		&mapButton13, &mapButton14,
	};

	for (int i = 0; i < GAMEPAD_DIGITAL_INPUT_COUNT; i++)
	{
		gpio_init(gamepadMappings[i]->pin);             // Initialize pin
		gpio_set_dir(gamepadMappings[i]->pin, GPIO_IN); // Set as INPUT
		gpio_pull_up(gamepadMappings[i]->pin);          // Set as PULLUP
	}
}

void GamepadClass::read()
{
	// Need to invert since we're using pullups
	uint32_t values = ~gpio_get_all();

	state.dpad = 0
		| ((values & mapDpadUp.pinMask)    ? mapDpadUp.buttonMask    : 0)
		| ((values & mapDpadDown.pinMask)  ? mapDpadDown.buttonMask  : 0)
		| ((values & mapDpadLeft.pinMask)  ? mapDpadLeft.buttonMask  : 0)
		| ((values & mapDpadRight.pinMask) ? mapDpadRight.buttonMask : 0)
	;

	state.buttons = 0
		| ((values & mapButton01.pinMask) ? mapButton01.buttonMask : 0)
		| ((values & mapButton02.pinMask) ? mapButton02.buttonMask : 0)
		| ((values & mapButton03.pinMask) ? mapButton03.buttonMask : 0)
		| ((values & mapButton04.pinMask) ? mapButton04.buttonMask : 0)
		| ((values & mapButton05.pinMask) ? mapButton05.buttonMask : 0)
		| ((values & mapButton06.pinMask) ? mapButton06.buttonMask : 0)
		| ((values & mapButton07.pinMask) ? mapButton07.buttonMask : 0)
		| ((values & mapButton08.pinMask) ? mapButton08.buttonMask : 0)
		| ((values & mapButton09.pinMask) ? mapButton09.buttonMask : 0)
		| ((values & mapButton10.pinMask) ? mapButton10.buttonMask : 0)
		| ((values & mapButton11.pinMask) ? mapButton11.buttonMask : 0)
		| ((values & mapButton12.pinMask) ? mapButton12.buttonMask : 0)
		| ((values & mapButton13.pinMask) ? mapButton13.buttonMask : 0)
		| ((values & mapButton14.pinMask) ? mapButton14.buttonMask : 0)
	;

	state.lx = GAMEPAD_JOYSTICK_MID;
	state.ly = GAMEPAD_JOYSTICK_MID;
	state.rx = GAMEPAD_JOYSTICK_MID;
	state.ry = GAMEPAD_JOYSTICK_MID;
	state.lt = 0;
	state.rt = 0;
}

void GamepadClass::save()
{
	Storage.setInputMode(current_input_mode);
	Storage.setDpadMode(dpadMode);
	Storage.setSOCDMode(socdMode);
	Storage.save();
}

void GamepadClass::load()
{
	Storage.start();
	current_input_mode = Storage.getInputMode();
	if (current_input_mode > HID)
		current_input_mode = DEFAULT_INPUT_MODE;

	dpadMode = Storage.getDpadMode();
	if (dpadMode > DPAD_MODE_RIGHT_ANALOG)
		dpadMode = DEFAULT_DPAD_MODE;

	socdMode = Storage.getSOCDMode();;
	if (socdMode > SOCD_MODE_SECOND_INPUT_PRIORITY)
		socdMode = DEFAULT_SOCD_MODE;
}
