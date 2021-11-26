#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "BoardConfig.h"
#include <string.h>
#include <MPGS.h>
#include "pico/stdlib.h"
#include "storage.h"

#define GAMEPAD_FEATURE_REPORT_SIZE 32

struct GamepadButtonMapping
{
	GamepadButtonMapping(uint8_t p, uint16_t bm) : pin(p), pinMask((1 << p)), buttonMask(bm) {}

	uint8_t pin;
	uint32_t pinMask;
	const uint16_t buttonMask;

	inline void setPin(uint8_t p)
	{
		pin = p;
		pinMask = 1 << p;
	}
};

class Gamepad : public MPGS
{
public:
	Gamepad(int debounceMS = 5, GamepadStorage *storage = &GamepadStore)
			: MPGS(debounceMS, storage) {}

	void setup();
	void read();

	void process()
	{
		memcpy(&rawState, &state, sizeof(GamepadState));
		MPGS::process();
	}

	inline bool __attribute__((always_inline)) pressedF1()
	{
#ifdef PIN_SETTINGS
		return state.aux & (1 << 0);
#else
		return MPGS::pressedF1();
#endif
	}

	GamepadState rawState;

	GamepadButtonMapping *mapDpadUp;
	GamepadButtonMapping *mapDpadDown;
	GamepadButtonMapping *mapDpadLeft;
	GamepadButtonMapping *mapDpadRight;
	GamepadButtonMapping *mapButtonB1;
	GamepadButtonMapping *mapButtonB2;
	GamepadButtonMapping *mapButtonB3;
	GamepadButtonMapping *mapButtonB4;
	GamepadButtonMapping *mapButtonL1;
	GamepadButtonMapping *mapButtonR1;
	GamepadButtonMapping *mapButtonL2;
	GamepadButtonMapping *mapButtonR2;
	GamepadButtonMapping *mapButtonS1;
	GamepadButtonMapping *mapButtonS2;
	GamepadButtonMapping *mapButtonL3;
	GamepadButtonMapping *mapButtonR3;
	GamepadButtonMapping *mapButtonA1;
	GamepadButtonMapping *mapButtonA2;

	GamepadButtonMapping **gamepadMappings;
};

#endif
