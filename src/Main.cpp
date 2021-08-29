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
#include "StateConverter.h"
#include "NeoPico.hpp"
#include "definitions/BoardConfig.h"

void *report;
uint8_t report_size;

uint32_t getMillis() { return to_ms_since_boot(get_absolute_time()); }

#ifdef BOARD_LEDS_PIN
NeoPico leds(BOARD_LEDS_PIN, BOARD_LEDS_COUNT);

bool isDpadPressed(GamepadButtonMapping button) {
	return ((Gamepad.state.dpad & button.buttonMask) == button.buttonMask);
}

bool isButtonPressed(GamepadButtonMapping button) {
	return ((Gamepad.state.buttons & button.buttonMask) == button.buttonMask);
}

void handleLed(GamepadButtonMapping button, bool pressed) {
	if (button.ledPos < 0) 
		return;

	uint32_t color = pressed ? leds.RGB(255, 255, 255) : 0;
	leds.SetPixel(button.ledPos, color);
}

void handleLeds()
{
	static GamepadButtonMapping dPadButtons[4] = {Gamepad.mapDpadLeft, Gamepad.mapDpadDown, Gamepad.mapDpadRight, Gamepad.mapDpadUp};
	static GamepadButtonMapping actionButtons[14] = {Gamepad.mapButton01, Gamepad.mapButton02, Gamepad.mapButton03, Gamepad.mapButton04, Gamepad.mapButton05, Gamepad.mapButton06, Gamepad.mapButton07, Gamepad.mapButton08, Gamepad.mapButton09, Gamepad.mapButton10, Gamepad.mapButton11, Gamepad.mapButton12, Gamepad.mapButton13, Gamepad.mapButton14};

	for (const GamepadButtonMapping &button : dPadButtons)
		handleLed(button, isDpadPressed(button));

	for (const GamepadButtonMapping &button : actionButtons)
		handleLed(button, isButtonPressed(button));
}
#endif

static inline void setup()
{
	// Set up controller
	Gamepad.setup();

	// Read options from EEPROM
	Gamepad.load();

	// Check for input mode override
	Gamepad.read();
	InputMode newInputMode = current_input_mode;
	if (Gamepad.isSelectPressed())
		newInputMode = SWITCH;
	else if (Gamepad.isStartPressed())
		newInputMode = XINPUT;

	if (newInputMode != current_input_mode)
	{
		current_input_mode = newInputMode;
		Storage.setInputMode(current_input_mode);
		Storage.save();
	}

	// Grab a pointer to the USB report for the selected input
	report = select_report(&report_size, current_input_mode);

	// Initialize USB/HID driver
	initialize_driver();
}

static inline void loop() {
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
	report = fill_report((GamepadState *)&Gamepad.state, false);

	// Send it!
	send_report(report, report_size);

	// Ensure next runtime ahead of current time
	nextRuntime = getMillis() + intervalMS;
}

void core1()
{
	while (1)
	{
#ifdef BOARD_LEDS_PIN
		handleLeds();
		leds.Show();
#endif
	}
}

int main()
{
	setup();

	multicore_launch_core1(core1);

	while (1)
		loop();

	return 0;
}
