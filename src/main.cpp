/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#define GAMEPAD_DEBOUNCE_MILLIS 5

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/util/queue.h"

#include <MPGS.h>

#include "usb_driver.h"
#include "BoardConfig.h"

#ifdef BOARD_LEDS_PIN
#include "leds.h"
#include "NeoPico.hpp"
#include "AnimationStation.hpp"
#include "AnimationStorage.hpp"
#include "Pixel.hpp"
#ifdef LEDS_PER_PIXEL
	PixelMatrix matrix = createLedButtonLayout(LED_LAYOUT, LEDS_PER_PIXEL);
#else
	PixelMatrix matrix = createLedButtonLayout(LED_LAYOUT, ledPositions);
#endif
#ifdef LED_FORMAT
NeoPico leds(BOARD_LEDS_PIN, matrix.getLedCount(), LED_FORMAT);
#else
NeoPico leds(BOARD_LEDS_PIN, matrix.getLedCount());
#endif
AnimationStation as(matrix);
queue_t animationQueue;
queue_t animationSaveQueue;
#endif

uint32_t getMillis() { return to_ms_since_boot(get_absolute_time()); }

MPGS gamepad(GAMEPAD_DEBOUNCE_MILLIS);

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
	if (gamepad.pressedR3())
		newInputMode = INPUT_MODE_HID;
	else if (gamepad.pressedS1())
		newInputMode = INPUT_MODE_SWITCH;
	else if (gamepad.pressedS2())
		newInputMode = INPUT_MODE_XINPUT;

	if (newInputMode != gamepad.options.inputMode)
	{
		gamepad.options.inputMode = newInputMode;
		gamepad.save();
	}

	initialize_driver(gamepad.options.inputMode);

#ifdef BOARD_LEDS_PIN
	queue_init(&animationQueue, sizeof(AnimationHotkey), 1);
	queue_init(&animationSaveQueue, sizeof(int), 1);
#endif
}


void loop()
{
	static void *report;
	static const uint16_t reportSize = gamepad.getReportSize();
#ifdef BOARD_LEDS_PIN
	static int saveValue = 0;
#endif

	gamepad.read();

#if GAMEPAD_DEBOUNCE_MILLIS > 0
	gamepad.debounce();
#endif

	gamepad.hotkey();
#ifdef BOARD_LEDS_PIN
	AnimationHotkey action = animationHotkeys(&gamepad);
	if (action != HOTKEY_LEDS_NONE)
		queue_try_add(&animationQueue, &action);
#endif

	gamepad.process();
	report = gamepad.getReport();
	send_report(report, reportSize);

#ifdef BOARD_LEDS_PIN
	if (queue_try_remove(&animationSaveQueue, &saveValue))
		AnimationStore.save();
#endif
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
		if (getMillis() - nextRuntime < 0)
			return;

#ifdef BOARD_LEDS_PIN
		if (queue_try_remove(&animationQueue, &action))
		{
			as.HandleEvent(action);
			queue_try_add(&animationSaveQueue, 0);
		}

		as.Animate();
		as.ApplyBrightness(frame);
		leds.SetFrame(frame);
		leds.Show();
#endif

		// Ensure next runtime ahead of current time
		nextRuntime = getMillis() + intervalMS;
	}
}
