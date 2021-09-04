/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GAMEPAD_H_
#define GAMEPAD_H_

#include <stdio.h>
#include <stdint.h>

#include "GamepadConfig.h"
#include "GamepadDescriptors.h"
#include "GamepadEnums.h"
#include "GamepadState.h"
#include "GamepadDebouncer.h"
#include "GamepadStorage.h"

#ifndef GAMEPAD_DEBOUNCE_MILLIS
#define GAMEPAD_DEBOUNCE_MILLIS 0
#endif

#define GAMEPAD_DPAD_INPUT_COUNT 4
#define GAMEPAD_BUTTON_INPUT_COUNT 14	 // Number of normal buttons (not D-pad)
#define GAMEPAD_DIGITAL_INPUT_COUNT 18 // Total number of buttons, including D-pad

struct GamepadButtonMapping
{
	const uint8_t port;
	const uint8_t pin;
	const uint32_t pinMask;
	const uint32_t buttonMask;
	const int8_t ledPos;
};

class GamepadClass
{
	public:
		GamepadClass()
		{
			#if GAMEPAD_DEBOUNCE_MILLIS > 0
				for (int i = 0; i < GAMEPAD_DIGITAL_INPUT_COUNT; i++)
					debouncers[i].setGamepadState(state);
			#endif
		}

		static GamepadButtonMapping mapDpadUp;
		static GamepadButtonMapping mapDpadDown;
		static GamepadButtonMapping mapDpadLeft;
		static GamepadButtonMapping mapDpadRight;
		static GamepadButtonMapping mapButtonB1;
		static GamepadButtonMapping mapButtonB2;
		static GamepadButtonMapping mapButtonB3;
		static GamepadButtonMapping mapButtonB4;
		static GamepadButtonMapping mapButtonL1;
		static GamepadButtonMapping mapButtonR1;
		static GamepadButtonMapping mapButtonL2;
		static GamepadButtonMapping mapButtonR2;
		static GamepadButtonMapping mapButtonS1;
		static GamepadButtonMapping mapButtonS2;
		static GamepadButtonMapping mapButtonL3;
		static GamepadButtonMapping mapButtonR3;
		static GamepadButtonMapping mapButtonA1;
		static GamepadButtonMapping mapButtonA2;

		DpadMode dpadMode = DPAD_MODE_DIGITAL;
		InputMode inputMode = INPUT_MODE_XINPUT;
		SOCDMode socdMode = SOCD_MODE_UP_PRIORITY;
		GamepadState state =
		{
			.dpad = 0,
			.buttons = 0,
			.lx = GAMEPAD_JOYSTICK_MID,
			.ly = GAMEPAD_JOYSTICK_MID,
			.rx = GAMEPAD_JOYSTICK_MID,
			.ry = GAMEPAD_JOYSTICK_MID,
			.lt = 0,
			.rt = 0,
		};

		bool hasAnalogTriggers = false;
		bool hasLeftAnalogStick = false;
		bool hasRightAnalogStick = false;

		/**
		 * Load the saved configuration from persitent storage
		 */
		virtual void load();

		/**
		 * Save the current configuration to persitent storage
		 */
		virtual void save();

		/**
		 * Perform pin setup and any other initialization the board requires
		 */
		virtual void setup();

		/**
		 * Retrieve the inputs and save to the current state
		 */
		virtual void read();

		/**
		 * Checks and executes any hotkey being pressed
		 */
		virtual GamepadHotkey hotkey();

		/**
		 * Run debouncing algorithm against current state inputs
		 */
		void debounce();

		/**
		 * Process the inputs before sending state to host
		 */
		void process();

		uint8_t *getReport();
		uint8_t getReportSize();

		HIDReport *getHIDReport();
		SwitchReport *getSwitchReport();
		XInputReport *getXInputReport();

		/**
		 * Returns if the function button/hotkey is pressed, override in derived board class
		 */
		virtual bool isDpadHotkeyPressed();

		/**
		 * Returns if the function button/hotkey is pressed, override in derived board class
		 */
		virtual bool isSOCDHotkeyPressed();

		inline bool __attribute__((always_inline)) pressedDpad(const uint8_t mask)    { return (state.dpad & mask) == mask; }
		inline bool __attribute__((always_inline)) pressedButton(const uint32_t mask) { return (state.buttons & mask) == mask; }

		inline bool __attribute__((always_inline)) pressedUp()    { return pressedDpad(GAMEPAD_MASK_UP); }
		inline bool __attribute__((always_inline)) pressedDown()  { return pressedDpad(GAMEPAD_MASK_DOWN); }
		inline bool __attribute__((always_inline)) pressedLeft()  { return pressedDpad(GAMEPAD_MASK_LEFT); }
		inline bool __attribute__((always_inline)) pressedRight() { return pressedDpad(GAMEPAD_MASK_RIGHT); }
		inline bool __attribute__((always_inline)) pressedB1()    { return pressedButton(GAMEPAD_MASK_B1); }
		inline bool __attribute__((always_inline)) pressedB2()    { return pressedButton(GAMEPAD_MASK_B2); }
		inline bool __attribute__((always_inline)) pressedB3()    { return pressedButton(GAMEPAD_MASK_B3); }
		inline bool __attribute__((always_inline)) pressedB4()    { return pressedButton(GAMEPAD_MASK_B4); }
		inline bool __attribute__((always_inline)) pressedL1()    { return pressedButton(GAMEPAD_MASK_L1); }
		inline bool __attribute__((always_inline)) pressedR1()    { return pressedButton(GAMEPAD_MASK_R1); }
		inline bool __attribute__((always_inline)) pressedL2()    { return pressedButton(GAMEPAD_MASK_L2); }
		inline bool __attribute__((always_inline)) pressedR2()    { return pressedButton(GAMEPAD_MASK_R2); }
		inline bool __attribute__((always_inline)) pressedS1()    { return pressedButton(GAMEPAD_MASK_S1); }
		inline bool __attribute__((always_inline)) pressedS2()    { return pressedButton(GAMEPAD_MASK_S2); }
		inline bool __attribute__((always_inline)) pressedL3()    { return pressedButton(GAMEPAD_MASK_L3); }
		inline bool __attribute__((always_inline)) pressedR3()    { return pressedButton(GAMEPAD_MASK_R3); }
		inline bool __attribute__((always_inline)) pressedA1()    { return pressedButton(GAMEPAD_MASK_A1); }
		inline bool __attribute__((always_inline)) pressedA2()    { return pressedButton(GAMEPAD_MASK_A2); }

	protected:
		// Convert the horizontal GamepadState dpad axis value into an analog value
		inline uint16_t dpadToAnalogX(uint8_t dpad)
		{
			switch (dpad & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
			{
			case GAMEPAD_MASK_LEFT:
				return GAMEPAD_JOYSTICK_MIN;
			case GAMEPAD_MASK_RIGHT:
				return GAMEPAD_JOYSTICK_MAX;
			default:
				return GAMEPAD_JOYSTICK_MID;
			}
		}

		// Convert the vertical GamepadState dpad axis value into an analog value
		inline uint16_t dpadToAnalogY(uint8_t dpad)
		{
			switch (dpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
			{
			case GAMEPAD_MASK_UP:
				return GAMEPAD_JOYSTICK_MIN;
			case GAMEPAD_MASK_DOWN:
				return GAMEPAD_JOYSTICK_MAX;
			default:
				return GAMEPAD_JOYSTICK_MID;
			}
		}

		inline uint8_t runSOCDCleaner(SOCDMode mode, uint8_t dpad)
		{
			static DpadDirection last_ud = DIRECTION_NONE;
			static DpadDirection last_lr = DIRECTION_NONE;
			uint8_t new_dpad = 0;

			switch (dpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
			{
				case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN):
					if (mode == SOCD_MODE_UP_PRIORITY)
					{
						new_dpad |= GAMEPAD_MASK_UP;
						last_ud = DIRECTION_UP;
					}
					else if (mode == SOCD_MODE_SECOND_INPUT_PRIORITY && last_ud != DIRECTION_NONE)
						new_dpad |= (last_ud == DIRECTION_UP) ? GAMEPAD_MASK_DOWN : GAMEPAD_MASK_UP;
					else
						last_ud = DIRECTION_NONE;
					break;
				case GAMEPAD_MASK_UP:
					new_dpad |= GAMEPAD_MASK_UP;
					last_ud = DIRECTION_UP;
					break;
				case GAMEPAD_MASK_DOWN:
					new_dpad |= GAMEPAD_MASK_DOWN;
					last_ud = DIRECTION_DOWN;
					break;
				default:
					last_ud = DIRECTION_NONE;
					break;
			}

			switch (dpad & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
			{
				case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
					if (mode == SOCD_MODE_SECOND_INPUT_PRIORITY && last_lr != DIRECTION_NONE)
						new_dpad |= (last_lr == DIRECTION_LEFT) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT;
					else
						last_lr = DIRECTION_NONE;
					break;
				case GAMEPAD_MASK_LEFT:
					new_dpad |= GAMEPAD_MASK_LEFT;
					last_lr = DIRECTION_LEFT;
					break;
				case GAMEPAD_MASK_RIGHT:
					new_dpad |= GAMEPAD_MASK_RIGHT;
					last_lr = DIRECTION_RIGHT;
					break;
				default:
					last_lr = DIRECTION_NONE;
					break;
			}

			return new_dpad;
		}
		#if GAMEPAD_DEBOUNCE_MILLIS > 0
				GamepadDebouncer debouncers[GAMEPAD_DIGITAL_INPUT_COUNT] =
				{
					GamepadDebouncer(GAMEPAD_MASK_UP, GAMEPAD_DEBOUNCE_MILLIS, true),
					GamepadDebouncer(GAMEPAD_MASK_DOWN, GAMEPAD_DEBOUNCE_MILLIS, true),
					GamepadDebouncer(GAMEPAD_MASK_LEFT, GAMEPAD_DEBOUNCE_MILLIS, true),
					GamepadDebouncer(GAMEPAD_MASK_RIGHT, GAMEPAD_DEBOUNCE_MILLIS, true),
					GamepadDebouncer(GAMEPAD_MASK_B1, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_B2, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_B3, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_B4, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_L1, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_R1, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_L2, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_R2, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_S1, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_S2, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_L3, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_R3, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_A1, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_A2, GAMEPAD_DEBOUNCE_MILLIS, false),
				};
		#else
				GamepadDebouncer debouncers[GAMEPAD_DIGITAL_INPUT_COUNT];
		#endif
};

static GamepadClass Gamepad;

#endif
