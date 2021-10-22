/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#define GAMEPAD_DEBOUNCE_MILLIS 5

#include "pico/multicore.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"

#include "BoardConfig.h"
#include "usb_driver.h"
#include "gamepad.hpp"

#ifdef BOARD_LEDS_PIN
#include "leds.h"
LEDs leds;
#endif

uint32_t getMillis() { return to_ms_since_boot(get_absolute_time()); }

Gamepad gamepad(GAMEPAD_DEBOUNCE_MILLIS);

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
	gamepad.setup();
	gamepad.load();

	// Check for input mode override
	gamepad.read();
	InputMode newInputMode = gamepad.options.inputMode;
	if (gamepad.pressedB3())
		newInputMode = INPUT_MODE_HID;
	else if (gamepad.pressedB1())
		newInputMode = INPUT_MODE_SWITCH;
	else if (gamepad.pressedB2())
		newInputMode = INPUT_MODE_XINPUT;
  else if (gamepad.pressedF1() && gamepad.pressedUp())
    reset_usb_boot(0, 0);

	if (newInputMode != gamepad.options.inputMode)
	{
		gamepad.options.inputMode = newInputMode;
		gamepad.save();
	}

	initialize_driver(gamepad.options.inputMode);

#ifdef BOARD_LEDS_PIN
  leds.setup();
#endif
}

void loop()
{
	static void *report;
	static const uint16_t reportSize = gamepad.getReportSize();

  gamepad.read();

#if GAMEPAD_DEBOUNCE_MILLIS > 0
  gamepad.debounce();
#endif

	gamepad.hotkey();

#ifdef BOARD_LEDS_PIN
	leds.process(&gamepad);
#endif

  gamepad.process();
  report = gamepad.getReport();
  send_report(report, reportSize);

#ifdef BOARD_LEDS_PIN
	leds.trySave();
#endif
}

void core1() {
  multicore_lockout_victim_init();

  while (1) {
#ifdef BOARD_LEDS_PIN
    leds.loop();
#endif
  }
}
