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

#include "usb_driver.h"
#include "Gamepad.h"
#include "GamepadStorage.h"
#include "NeoPico.hpp"
#include "AnimationStation.hpp"
#include "definitions/BoardConfig.h"

uint32_t getMillis() { return to_ms_since_boot(get_absolute_time()); }

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
	Gamepad.setup();

	// Read options from EEPROM
	Gamepad.load();

	// Check for input mode override
	Gamepad.read();
	InputMode newInputMode = Gamepad.inputMode;
	if (Gamepad.pressedR3())
		newInputMode = INPUT_MODE_HID;
	else if (Gamepad.pressedS1())
		newInputMode = INPUT_MODE_SWITCH;
	else if (Gamepad.pressedS2())
		newInputMode = INPUT_MODE_XINPUT;

	if (newInputMode != Gamepad.inputMode)
	{
		Gamepad.inputMode = newInputMode;
		Storage.setInputMode(Gamepad.inputMode);
		Storage.save();
	}

	// Initialize USB driver
	initialize_driver(Gamepad.inputMode);
}

void loop()
{
	static const uint8_t reportSize = Gamepad.getReportSize();
	static uint8_t *report;

	// Poll every 1ms
	const uint32_t intervalMS = 1;
	static uint32_t nextRuntime = 0;

	if (getMillis() - nextRuntime < 0)
		return;

	// Read raw inputs
	Gamepad.read();

#if GAMEPAD_DEBOUNCE_MILLIS > 0
	// Run debouncing if enabled
	Gamepad.debounce();
#endif

	// Check for hotkey presses, can react to return value
	Gamepad.hotkey();

	// Perform final input processing before report conversion
	Gamepad.process();

	// Convert to USB report
	report = Gamepad.getReport();

	// Send it!
	send_report(report, reportSize);

	// Ensure next runtime ahead of current time
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
