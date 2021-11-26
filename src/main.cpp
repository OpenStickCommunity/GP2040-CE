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

#include "rndis/rndis.h"
#include "usb_driver.h"
#include "gp2040.h"
#include "gamepad.h"
#include "leds.h"
#include "pleds.h"
#include "display.h"

uint32_t getMillis() { return to_ms_since_boot(get_absolute_time()); }

Gamepad gamepad(GAMEPAD_DEBOUNCE_MILLIS);
static InputMode inputMode;
queue_t gamepadQueue;

DisplayModule displayModule;
LEDModule ledModule;
PLEDModule pledModule(PLED_TYPE);
std::vector<GPModule*> modules =
{
	&displayModule,
	&ledModule,
	&pledModule,
};

void setup();
void loop();
void core1();
void webserver();

int main()
{
	setup();
	multicore_launch_core1(core1);

	if (inputMode == INPUT_MODE_CONFIG)
	{
		webserver();
	}
	else
	{
		while (1)
			loop();
	}

	return 0;
}

void setup()
{
	gamepad.setup();

	// Check for input mode override
	gamepad.read();
	inputMode = gamepad.options.inputMode;
	if (gamepad.pressedS2())
		inputMode = INPUT_MODE_CONFIG;
	else if (gamepad.pressedB3())
		inputMode = INPUT_MODE_HID;
	else if (gamepad.pressedB1())
		inputMode = INPUT_MODE_SWITCH;
	else if (gamepad.pressedB2())
		inputMode = INPUT_MODE_XINPUT;
	else if (gamepad.pressedF1() && gamepad.pressedUp())
		reset_usb_boot(0, 0);

	queue_init(&gamepadQueue, sizeof(Gamepad), 1);

	for (auto it = modules.begin(); it != modules.end();)
	{
		GPModule *module = (*it);
		module->setup();

		if (module->isEnabled())
			it++;
		else
			it = modules.erase(it);
	}

	bool configMode = inputMode == INPUT_MODE_CONFIG;
	if (inputMode != gamepad.options.inputMode && !configMode)
	{
		gamepad.options.inputMode = inputMode;
		gamepad.save();
	}

	initialize_driver(inputMode);
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
	if (featureData[0])
		queue_try_add(&pledModule.featureQueue, featureData);

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

void webserver()
{
	static Gamepad snapshot;

	rndis_init();
	while (1)
	{
		gamepad.read();
#if GAMEPAD_DEBOUNCE_MILLIS > 0
		gamepad.debounce();
#endif
		gamepad.hotkey();
		gamepad.process();

		if (queue_is_empty(&gamepadQueue))
		{
			memcpy(&snapshot, &gamepad, sizeof(Gamepad));
			queue_try_add(&gamepadQueue, &snapshot);
		}

		rndis_task();
	}
}
