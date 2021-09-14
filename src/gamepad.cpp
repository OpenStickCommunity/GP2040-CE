/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include <MPGS.h>

#include "pico/stdlib.h"

#include "BoardConfig.h"

struct GamepadButtonMapping
{
	const uint8_t pin;
	const uint32_t pinMask;
	const uint32_t buttonMask;
};

GamepadButtonMapping mapDpadUp    = { .pin = PIN_DPAD_UP,    .pinMask = (1 << PIN_DPAD_UP),    .buttonMask = GAMEPAD_MASK_UP    };
GamepadButtonMapping mapDpadDown  = { .pin = PIN_DPAD_DOWN,  .pinMask = (1 << PIN_DPAD_DOWN),  .buttonMask = GAMEPAD_MASK_DOWN  };
GamepadButtonMapping mapDpadLeft  = { .pin = PIN_DPAD_LEFT,  .pinMask = (1 << PIN_DPAD_LEFT),  .buttonMask = GAMEPAD_MASK_LEFT  };
GamepadButtonMapping mapDpadRight = { .pin = PIN_DPAD_RIGHT, .pinMask = (1 << PIN_DPAD_RIGHT), .buttonMask = GAMEPAD_MASK_RIGHT };
GamepadButtonMapping mapButtonB1  = { .pin = PIN_BUTTON_B1,  .pinMask = (1 << PIN_BUTTON_B1),  .buttonMask = GAMEPAD_MASK_B1    };
GamepadButtonMapping mapButtonB2  = { .pin = PIN_BUTTON_B2,  .pinMask = (1 << PIN_BUTTON_B2),  .buttonMask = GAMEPAD_MASK_B2    };
GamepadButtonMapping mapButtonB3  = { .pin = PIN_BUTTON_B3,  .pinMask = (1 << PIN_BUTTON_B3),  .buttonMask = GAMEPAD_MASK_B3    };
GamepadButtonMapping mapButtonB4  = { .pin = PIN_BUTTON_B4,  .pinMask = (1 << PIN_BUTTON_B4),  .buttonMask = GAMEPAD_MASK_B4    };
GamepadButtonMapping mapButtonL1  = { .pin = PIN_BUTTON_L1,  .pinMask = (1 << PIN_BUTTON_L1),  .buttonMask = GAMEPAD_MASK_L1    };
GamepadButtonMapping mapButtonR1  = { .pin = PIN_BUTTON_R1,  .pinMask = (1 << PIN_BUTTON_R1),  .buttonMask = GAMEPAD_MASK_R1    };
GamepadButtonMapping mapButtonL2  = { .pin = PIN_BUTTON_L2,  .pinMask = (1 << PIN_BUTTON_L2),  .buttonMask = GAMEPAD_MASK_L2    };
GamepadButtonMapping mapButtonR2  = { .pin = PIN_BUTTON_R2,  .pinMask = (1 << PIN_BUTTON_R2),  .buttonMask = GAMEPAD_MASK_R2    };
GamepadButtonMapping mapButtonS1  = { .pin = PIN_BUTTON_S1,  .pinMask = (1 << PIN_BUTTON_S1),  .buttonMask = GAMEPAD_MASK_S1    };
GamepadButtonMapping mapButtonS2  = { .pin = PIN_BUTTON_S2,  .pinMask = (1 << PIN_BUTTON_S2),  .buttonMask = GAMEPAD_MASK_S2    };
GamepadButtonMapping mapButtonL3  = { .pin = PIN_BUTTON_L3,  .pinMask = (1 << PIN_BUTTON_L3),  .buttonMask = GAMEPAD_MASK_L3    };
GamepadButtonMapping mapButtonR3  = { .pin = PIN_BUTTON_R3,  .pinMask = (1 << PIN_BUTTON_R3),  .buttonMask = GAMEPAD_MASK_R3    };
GamepadButtonMapping mapButtonA1  = { .pin = PIN_BUTTON_A1,  .pinMask = (1 << PIN_BUTTON_A1),  .buttonMask = GAMEPAD_MASK_A1    };
GamepadButtonMapping mapButtonA2  = { .pin = PIN_BUTTON_A2,  .pinMask = (1 << PIN_BUTTON_A2),  .buttonMask = GAMEPAD_MASK_A2    };

uint32_t GamepadDebouncer::getMillis() { return to_ms_since_boot(get_absolute_time()); }

void MPG::setup()
{
	GamepadStore.start();

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

void MPG::read()
{
	// Need to invert since we're using pullups
	uint32_t values = ~gpio_get_all();

	state.buttons = 0
		| ((values & mapDpadUp.pinMask)    ? mapDpadUp.buttonMask    : 0)
		| ((values & mapDpadDown.pinMask)  ? mapDpadDown.buttonMask  : 0)
		| ((values & mapDpadLeft.pinMask)  ? mapDpadLeft.buttonMask  : 0)
		| ((values & mapDpadRight.pinMask) ? mapDpadRight.buttonMask : 0)
		| ((values & mapButtonB1.pinMask)  ? mapButtonB1.buttonMask  : 0)
		| ((values & mapButtonB2.pinMask)  ? mapButtonB2.buttonMask  : 0)
		| ((values & mapButtonB3.pinMask)  ? mapButtonB3.buttonMask  : 0)
		| ((values & mapButtonB4.pinMask)  ? mapButtonB4.buttonMask  : 0)
		| ((values & mapButtonL1.pinMask)  ? mapButtonL1.buttonMask  : 0)
		| ((values & mapButtonR1.pinMask)  ? mapButtonR1.buttonMask  : 0)
		| ((values & mapButtonL2.pinMask)  ? mapButtonL2.buttonMask  : 0)
		| ((values & mapButtonR2.pinMask)  ? mapButtonR2.buttonMask  : 0)
		| ((values & mapButtonS1.pinMask)  ? mapButtonS1.buttonMask  : 0)
		| ((values & mapButtonS2.pinMask)  ? mapButtonS2.buttonMask  : 0)
		| ((values & mapButtonL3.pinMask)  ? mapButtonL3.buttonMask  : 0)
		| ((values & mapButtonR3.pinMask)  ? mapButtonR3.buttonMask  : 0)
		| ((values & mapButtonA1.pinMask)  ? mapButtonA1.buttonMask  : 0)
		| ((values & mapButtonA2.pinMask)  ? mapButtonA2.buttonMask  : 0)
	;

	state.lx = GAMEPAD_JOYSTICK_MID;
	state.ly = GAMEPAD_JOYSTICK_MID;
	state.rx = GAMEPAD_JOYSTICK_MID;
	state.ry = GAMEPAD_JOYSTICK_MID;
	state.lt = 0;
	state.rt = 0;
}
