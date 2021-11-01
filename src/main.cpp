/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#define GAMEPAD_DEBOUNCE_MILLIS 5

#include "BoardConfig.h"

#include <vector>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "pico/util/queue.h"
#include "tusb.h"

#include "gp2040.h"
#include "usb_driver.h"
#include "gamepad.h"
#include "webserver.h"

#ifdef BOARD_LEDS_PIN
#include "leds.h"
LEDModule ledModule;
#endif

#ifdef PLED_TYPE
#include "pleds.h"
PLEDModule pledModule(PLED_TYPE);
#endif

uint32_t getMillis() { return to_ms_since_boot(get_absolute_time()); }

static Gamepad gamepad(GAMEPAD_DEBOUNCE_MILLIS);
queue_t gamepadQueue;
std::vector<GPModule*> modules;

void setup();
void loop();
void core1();

int main()
{
	setup();
	multicore_launch_core1(core1);

	if (gamepad.options.inputMode == INPUT_MODE_CONFIG)
	{
		webserver(&gamepad);
	}
	else
	{
		while (1)
			loop();

		return 0;
	}
}

void setup()
{
	gamepad.setup();

	// Check for input mode override
	gamepad.read();
	InputMode newInputMode = gamepad.options.inputMode;
	if (gamepad.pressedS2())
		newInputMode = INPUT_MODE_CONFIG;
	else if (gamepad.pressedB3())
		newInputMode = INPUT_MODE_HID;
	else if (gamepad.pressedB1())
		newInputMode = INPUT_MODE_SWITCH;
	else if (gamepad.pressedB2())
		newInputMode = INPUT_MODE_XINPUT;
	else if (gamepad.pressedF1() && gamepad.pressedUp())
		reset_usb_boot(0, 0);

	bool configMode = newInputMode == INPUT_MODE_CONFIG;
	if (newInputMode != gamepad.options.inputMode && !configMode)
	{
		gamepad.options.inputMode = newInputMode;
		gamepad.save();
	}
	else
	{
		gamepad.options.inputMode = newInputMode;
	}

	queue_init(&gamepadQueue, sizeof(Gamepad), 1);

#ifdef BOARD_LEDS_PIN
	modules.push_back(&ledModule);
#endif
#ifdef PLED_TYPE
	modules.push_back(&pledModule);
#endif

	for (auto module : modules)
		module->setup();

	initialize_driver(gamepad.options.inputMode);
}

void loop()
{
	static void *report;
	static const uint16_t reportSize = gamepad.getReportSize();
	static const uint32_t intervalMS = 1;
	static uint32_t nextRuntime = 0;
	static uint8_t featureData[32] = { };
	static Gamepad snapshot;

	if (getMillis() - nextRuntime < 0)
		return;

	gamepad.read();
#if GAMEPAD_DEBOUNCE_MILLIS > 0
	gamepad.debounce();
#endif
	gamepad.hotkey();
	gamepad.process();
	report = gamepad.getReport();
	send_report(report, reportSize);

	memset(featureData, 0, sizeof(featureData));
	receive_report(featureData);
#ifdef PLED_TYPE
	if (featureData[0])
		queue_try_add(&pledModule.featureQueue, featureData);
#endif
	tud_task();

	if (queue_is_empty(&gamepadQueue))
	{
		memcpy(&snapshot, &gamepad, sizeof(Gamepad));
		queue_try_add(&gamepadQueue, &snapshot);
	}

	nextRuntime = getMillis() + intervalMS;
}

void core1()
{
	multicore_lockout_victim_init();

	while (1)
	{
		static Gamepad snapshot;

		if (queue_try_remove(&gamepadQueue, &snapshot))
		{
			for (auto module : modules)
				module->process(&snapshot);
		}

		for (auto module : modules)
			module->loop();
	}
}
