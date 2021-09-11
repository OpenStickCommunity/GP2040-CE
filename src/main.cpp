/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#define GAMEPAD_DEBOUNCE_MILLIS 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include "usb_driver.h"
#include "BoardConfig.h"
#include "LEDConfig.h"
#include "FlashPROM.h"
#include "MPG.h"
#include "NeoPico.hpp"
#include "AnimationStation.hpp"
#include "AnimationStorage.hpp"
#include "Animation.hpp"
#include "Effects/StaticColor.hpp"

uint32_t getMillis() { return to_ms_since_boot(get_absolute_time()); }

MPG gamepad;

#ifdef BOARD_LEDS_PIN
NeoPico leds(BOARD_LEDS_PIN, BOARD_LEDS_COUNT);
AnimationStation as(BOARD_LEDS_COUNT);
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
	static uint8_t *report;
	static const uint8_t reportSize = gamepad.getReportSize();
	static const uint32_t intervalMS = 1;
	static uint32_t nextRuntime = 0;

	if (getMillis() - nextRuntime < 0)
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
	nextRuntime = getMillis() + intervalMS;
}

void core1()
{
	static const uint32_t intervalMS = 20;
	static uint32_t nextRuntime = 0;

	multicore_lockout_victim_init();

#ifdef BOARD_LEDS_PIN
	static AnimationHotkey action;

	AnimationStore.setup();

	AnimationMode mode = AnimationStore.getBaseAnimation();
	switch (mode)
	{
		case RAINBOW:
			as.SetRainbow();
			break;

		case CHASE:
			as.SetChase();
			break;

		default:
			as.SetStaticColor();
			break;
	}
#endif

	while (1)
	{
		if (getMillis() - nextRuntime < 0)
			return;

#ifdef BOARD_LEDS_PIN
		if (queue_try_peek(&animationQueue, &action))
		{
			queue_remove_blocking(&animationQueue, &action);
			as.HandleEvent(action);
			AnimationStore.save(as);
		}

		as.Animate();
		leds.SetFrame(as.frame);
		leds.Show();
#endif

		// Ensure next runtime ahead of current time
		nextRuntime = getMillis() + intervalMS;
	}
}
