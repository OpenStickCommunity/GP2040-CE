/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

// #define HAS_PERSISTENT_STORAGE 1 // WHY DOESN'T THIS WORK?!?!?!?!?!?!?
#define GAMEPAD_DEBOUNCE_MILLIS 5

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/multicore.h"

#include "usb_driver.h"
#include "BoardConfig.h"
#include "MPG.h"
#include "NeoPico.hpp"
#include "AnimationStation.hpp"

uint32_t getMillis() { return to_ms_since_boot(get_absolute_time()); }

MPG gamepad;

#ifdef BOARD_LEDS_PIN
NeoPico leds(BOARD_LEDS_PIN, BOARD_LEDS_COUNT);
AnimationStation as(BOARD_LEDS_COUNT);
#endif

void setup();
void loop();
void core1();

int main()
{
	setup();

	multicore_launch_core1(core1);

	while (1)
		loop();

	return 0;
}

void setup()
{
	// Set up controller
	gamepad.setup();

	// Read options from EEPROM
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
		Storage.setInputMode(gamepad.inputMode);
		Storage.save();
	}

	// Initialize USB driver
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
	gamepad.process();
	report = gamepad.getReport();
	send_report(report, reportSize);
	nextRuntime = getMillis() + intervalMS;
}

void core1()
{
#ifdef BOARD_LEDS_PIN

	as.SetBrightness(LEDS_BRIGHTNESS / 100.0);

	switch (LEDS_BASE_ANIMATION)
	{
		case RAINBOW:
			as.SetRainbow(true, LEDS_BASE_ANIMATION_FIRST_PIXEL, LEDS_BASE_ANIMATION_LAST_PIXEL, LEDS_RAINBOW_CYCLE_TIME);
			break;

		case CHASE:
			as.SetChase(true, LEDS_BASE_ANIMATION_FIRST_PIXEL, LEDS_BASE_ANIMATION_LAST_PIXEL, LEDS_CHASE_CYCLE_TIME);
			break;

		default:
			as.SetStaticColor(true, LEDS_STATIC_COLOR_COLOR, LEDS_BASE_ANIMATION_FIRST_PIXEL, LEDS_BASE_ANIMATION_LAST_PIXEL);
			break;
	}

	while (1)
	{
		as.Animate();
		leds.SetFrame(as.frame);
		leds.Show();
	}
#endif
}
