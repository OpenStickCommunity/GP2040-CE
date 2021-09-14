/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#define GAMEPAD_DEBOUNCE_MILLIS 5

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include <MPGS.h>

#include "NeoPico.hpp"
#include "AnimationStation.hpp"
#include "AnimationStorage.hpp"
#include "Pixel.hpp"

#include "usb_driver.h"
#include "BoardConfig.h"

MPGS gamepad(GAMEPAD_DEBOUNCE_MILLIS);

#ifdef BOARD_LEDS_PIN
NeoPico leds(BOARD_LEDS_PIN, Pixel::getPixelCount(pixels));
AnimationStation as(pixels);
queue_t animationQueue;
#endif

void setup();
void loop();
void core1();

int main()
{
	setup();

	while (1)
		loop();

	return 0;
}

void setup()
{
	gamepad.setup();
	gamepad.load();

	// Check for input mode override
	gamepad.read();
	InputMode newInputMode = gamepad.inputMode;
	if (gamepad.pressedR3())
		newInputMode = INPUT_MODE_HID;
	else if (gamepad.pressedS1())
		newInputMode = INPUT_MODE_SWITCH;
	else if (gamepad.pressedS2())
		newInputMode = INPUT_MODE_XINPUT;

	if (newInputMode != gamepad.inputMode)
	{
		gamepad.inputMode = newInputMode;
		gamepad.save();
	}

#ifdef BOARD_LEDS_PIN
	queue_init(&animationQueue, sizeof(AnimationHotkey), 1);
	multicore_launch_core1(core1);
#endif

	initialize_driver(gamepad.inputMode);
}

void loop()
{
	static void *report;
	static const uint16_t reportSize = gamepad.getReportSize();
	static const uint32_t intervalMS = 1;
	static uint32_t nextRuntime = 0;

	if (GamepadDebouncer::getMillis() - nextRuntime < 0)
		return;

	gamepad.read();

#if GAMEPAD_DEBOUNCE_MILLIS > 0
	gamepad.debounce();
#endif

	gamepad.hotkey();

#ifdef BOARD_LEDS_PIN
	AnimationHotkey action = animationHotkeys(gamepad);
	if (action != HOTKEY_LEDS_NONE)
		queue_add_blocking(&animationQueue, &action);
#endif

	gamepad.process();
	report = gamepad.getReport();
	send_report(report, reportSize);

	// Ensure next runtime ahead of current time
	nextRuntime = GamepadDebouncer::getMillis() + intervalMS;
}

void core1()
{
	static const uint32_t intervalMS = 10;
	static uint32_t nextRuntime = 0;

	multicore_lockout_victim_init();

#ifdef BOARD_LEDS_PIN
	static AnimationHotkey action;
	static uint32_t frame[100];

	AnimationStore.setup(&as);
#endif

	while (1)
	{
		if (GamepadDebouncer::getMillis() - nextRuntime < 0)
			return;

#ifdef BOARD_LEDS_PIN
		if (queue_try_peek(&animationQueue, &action))
		{
			queue_remove_blocking(&animationQueue, &action);
			as.HandleEvent(action);
			AnimationStore.save();
		}

		as.Animate();
		as.ApplyBrightness(frame);
		leds.SetFrame(frame);
		leds.Show();
#endif

		// Ensure next runtime ahead of current time
		nextRuntime = GamepadDebouncer::getMillis() + intervalMS;
	}
}
