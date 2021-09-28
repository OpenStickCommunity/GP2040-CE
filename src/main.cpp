/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#define GAMEPAD_DEBOUNCE_MILLIS 5

#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pico/util/queue.h"


#include <MPGS.h>

#include "BoardConfig.h"
#include "leds.h"
#include "usb_driver.h"


uint32_t getMillis() { return to_ms_since_boot(get_absolute_time()); }

MPGS gamepad(GAMEPAD_DEBOUNCE_MILLIS);
LEDs leds;

void setup();
void loop();
void core1();

int main() {
  setup();

  while (1)
    loop();

  return 0;
}

void setup() {
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

  if (newInputMode != gamepad.inputMode) {
    gamepad.inputMode = newInputMode;
    gamepad.save();
  }

#ifdef BOARD_LEDS_PIN
  multicore_launch_core1(core1);
  leds.setup();
#endif

  initialize_driver(gamepad.inputMode);
}

void loop() {
  static void *report;
  static const uint16_t reportSize = gamepad.getReportSize();
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
	leds.process(&gamepad);
#endif

  gamepad.process();
  report = gamepad.getReport();
  send_report(report, reportSize);

  // Ensure next runtime ahead of current time
  nextRuntime = getMillis() + intervalMS;
}

void core1() {
  multicore_lockout_victim_init();

  while (1) {
#ifdef BOARD_LEDS_PIN
    leds.loop();
#endif
  }
}
