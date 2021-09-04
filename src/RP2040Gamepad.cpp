/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "pico/stdlib.h"

#include "usb_driver.h"
#include "Gamepad.h"

#include "definitions/BoardConfig.h"

GamepadButtonMapping GamepadClass::mapDpadUp    = { .port = 0, .pin = PIN_DPAD_UP,    .pinMask = (1 << PIN_DPAD_UP),    .buttonMask = GAMEPAD_MASK_UP,    .ledPos = LEDS_DPAD_UP    };
GamepadButtonMapping GamepadClass::mapDpadDown  = { .port = 0, .pin = PIN_DPAD_DOWN,  .pinMask = (1 << PIN_DPAD_DOWN),  .buttonMask = GAMEPAD_MASK_DOWN,  .ledPos = LEDS_DPAD_DOWN  };
GamepadButtonMapping GamepadClass::mapDpadLeft  = { .port = 0, .pin = PIN_DPAD_LEFT,  .pinMask = (1 << PIN_DPAD_LEFT),  .buttonMask = GAMEPAD_MASK_LEFT,  .ledPos = LEDS_DPAD_LEFT  };
GamepadButtonMapping GamepadClass::mapDpadRight = { .port = 0, .pin = PIN_DPAD_RIGHT, .pinMask = (1 << PIN_DPAD_RIGHT), .buttonMask = GAMEPAD_MASK_RIGHT, .ledPos = LEDS_DPAD_RIGHT };
GamepadButtonMapping GamepadClass::mapButtonB1  = { .port = 0, .pin = PIN_BUTTON_B1,  .pinMask = (1 << PIN_BUTTON_B1),  .buttonMask = GAMEPAD_MASK_B1,    .ledPos = LEDS_BUTTON_01  };
GamepadButtonMapping GamepadClass::mapButtonB2  = { .port = 0, .pin = PIN_BUTTON_B2,  .pinMask = (1 << PIN_BUTTON_B2),  .buttonMask = GAMEPAD_MASK_B2,    .ledPos = LEDS_BUTTON_02  };
GamepadButtonMapping GamepadClass::mapButtonB3  = { .port = 0, .pin = PIN_BUTTON_B3,  .pinMask = (1 << PIN_BUTTON_B3),  .buttonMask = GAMEPAD_MASK_B3,    .ledPos = LEDS_BUTTON_03  };
GamepadButtonMapping GamepadClass::mapButtonB4  = { .port = 0, .pin = PIN_BUTTON_B4,  .pinMask = (1 << PIN_BUTTON_B4),  .buttonMask = GAMEPAD_MASK_B4,    .ledPos = LEDS_BUTTON_04  };
GamepadButtonMapping GamepadClass::mapButtonL1  = { .port = 0, .pin = PIN_BUTTON_L1,  .pinMask = (1 << PIN_BUTTON_L1),  .buttonMask = GAMEPAD_MASK_L1,    .ledPos = LEDS_BUTTON_05  };
GamepadButtonMapping GamepadClass::mapButtonR1  = { .port = 0, .pin = PIN_BUTTON_R1,  .pinMask = (1 << PIN_BUTTON_R1),  .buttonMask = GAMEPAD_MASK_R1,    .ledPos = LEDS_BUTTON_06  };
GamepadButtonMapping GamepadClass::mapButtonL2  = { .port = 0, .pin = PIN_BUTTON_L2,  .pinMask = (1 << PIN_BUTTON_L2),  .buttonMask = GAMEPAD_MASK_L2,    .ledPos = LEDS_BUTTON_07  };
GamepadButtonMapping GamepadClass::mapButtonR2  = { .port = 0, .pin = PIN_BUTTON_R2,  .pinMask = (1 << PIN_BUTTON_R2),  .buttonMask = GAMEPAD_MASK_R2,    .ledPos = LEDS_BUTTON_08  };
GamepadButtonMapping GamepadClass::mapButtonS1  = { .port = 0, .pin = PIN_BUTTON_S1,  .pinMask = (1 << PIN_BUTTON_S1),  .buttonMask = GAMEPAD_MASK_S1,    .ledPos = LEDS_BUTTON_09  };
GamepadButtonMapping GamepadClass::mapButtonS2  = { .port = 0, .pin = PIN_BUTTON_S2,  .pinMask = (1 << PIN_BUTTON_S2),  .buttonMask = GAMEPAD_MASK_S2,    .ledPos = LEDS_BUTTON_10  };
GamepadButtonMapping GamepadClass::mapButtonL3  = { .port = 0, .pin = PIN_BUTTON_L3,  .pinMask = (1 << PIN_BUTTON_L3),  .buttonMask = GAMEPAD_MASK_L3,    .ledPos = LEDS_BUTTON_11  };
GamepadButtonMapping GamepadClass::mapButtonR3  = { .port = 0, .pin = PIN_BUTTON_R3,  .pinMask = (1 << PIN_BUTTON_R3),  .buttonMask = GAMEPAD_MASK_R3,    .ledPos = LEDS_BUTTON_12  };
GamepadButtonMapping GamepadClass::mapButtonA1  = { .port = 0, .pin = PIN_BUTTON_A1,  .pinMask = (1 << PIN_BUTTON_A1),  .buttonMask = GAMEPAD_MASK_A1,    .ledPos = LEDS_BUTTON_13  };
GamepadButtonMapping GamepadClass::mapButtonA2  = { .port = 0, .pin = PIN_BUTTON_A2,  .pinMask = (1 << PIN_BUTTON_A2),  .buttonMask = GAMEPAD_MASK_A2,    .ledPos = LEDS_BUTTON_14  };

void GamepadClass::setup()
{
	GamepadButtonMapping *gamepadMappings[] =
	{
		&mapDpadUp,   &mapDpadDown, &mapDpadLeft, &mapDpadRight,
		&mapButtonB1, &mapButtonB2, &mapButtonB3, &mapButtonB4,
		&mapButtonL1, &mapButtonR1, &mapButtonL2, &mapButtonR2,
		&mapButtonS1, &mapButtonS2, &mapButtonL3, &mapButtonR3,
		&mapButtonA1, &mapButtonA2,
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
		| ((values & mapButtonB1.pinMask) ? mapButtonB1.buttonMask : 0)
		| ((values & mapButtonB2.pinMask) ? mapButtonB2.buttonMask : 0)
		| ((values & mapButtonB3.pinMask) ? mapButtonB3.buttonMask : 0)
		| ((values & mapButtonB4.pinMask) ? mapButtonB4.buttonMask : 0)
		| ((values & mapButtonL1.pinMask) ? mapButtonL1.buttonMask : 0)
		| ((values & mapButtonR1.pinMask) ? mapButtonR1.buttonMask : 0)
		| ((values & mapButtonL2.pinMask) ? mapButtonL2.buttonMask : 0)
		| ((values & mapButtonR2.pinMask) ? mapButtonR2.buttonMask : 0)
		| ((values & mapButtonS1.pinMask) ? mapButtonS1.buttonMask : 0)
		| ((values & mapButtonS2.pinMask) ? mapButtonS2.buttonMask : 0)
		| ((values & mapButtonL3.pinMask) ? mapButtonL3.buttonMask : 0)
		| ((values & mapButtonR3.pinMask) ? mapButtonR3.buttonMask : 0)
		| ((values & mapButtonA1.pinMask) ? mapButtonA1.buttonMask : 0)
		| ((values & mapButtonA2.pinMask) ? mapButtonA2.buttonMask : 0)
	;

	state.lx = GAMEPAD_JOYSTICK_MID;
	state.ly = GAMEPAD_JOYSTICK_MID;
	state.rx = GAMEPAD_JOYSTICK_MID;
	state.ry = GAMEPAD_JOYSTICK_MID;
	state.lt = 0;
	state.rt = 0;
}
