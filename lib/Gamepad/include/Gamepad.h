/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef GAMEPAD_H_
#define GAMEPAD_H_

#include <stdio.h>
#include <stdint.h>

#include "usb_driver.h"
#include "GamepadState.h"
#include "GamepadDebouncer.h"

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

// The available stick emulation modes
typedef enum
{
	DPAD_MODE_DIGITAL,
	DPAD_MODE_LEFT_ANALOG,
	DPAD_MODE_RIGHT_ANALOG,
} DpadMode;

// The available SOCD cleaning methods
typedef enum
{
	SOCD_MODE_UP_PRIORITY,					 // U+D=U, L+R=N
	SOCD_MODE_NEUTRAL,							 // U+D=N, L+R=N
	SOCD_MODE_SECOND_INPUT_PRIORITY, // U>D=D, L>R=R (Last Input Priority, aka Last Win)
} SOCDMode;

// Enum for tracking last direction state of Second Input SOCD method
typedef enum
{
	DIRECTION_NONE,
	DIRECTION_UP,
	DIRECTION_DOWN,
	DIRECTION_LEFT,
	DIRECTION_RIGHT
} DpadDirection;

// The available hotkey actions
typedef enum
{
	HOTKEY_NONE = 0x00,
	HOTKEY_DPAD_DIGITAL = 0x01,
	HOTKEY_DPAD_LEFT_ANALOG = 0x02,
	HOTKEY_DPAD_RIGHT_ANALOG = 0x04,
	HOTKEY_HOME_BUTTON = 0x08,
	HOTKEY_CAPTURE_BUTTON = 0x10,
	HOTKEY_SOCD_UP_PRIORITY = 0x20,
	HOTKEY_SOCD_NEUTRAL = 0x40,
	HOTKEY_SOCD_LAST_INPUT = 0x80,
} GamepadHotkey;

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
		static GamepadButtonMapping mapButton01;
		static GamepadButtonMapping mapButton02;
		static GamepadButtonMapping mapButton03;
		static GamepadButtonMapping mapButton04;
		static GamepadButtonMapping mapButton05;
		static GamepadButtonMapping mapButton06;
		static GamepadButtonMapping mapButton07;
		static GamepadButtonMapping mapButton08;
		static GamepadButtonMapping mapButton09;
		static GamepadButtonMapping mapButton10;
		static GamepadButtonMapping mapButton11;
		static GamepadButtonMapping mapButton12;
		static GamepadButtonMapping mapButton13;
		static GamepadButtonMapping mapButton14;

		DpadMode dpadMode = DPAD_MODE_DIGITAL;
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

		/**
		 * Returns if the function button/hotkey is pressed, override in derived board class
		 */
		virtual bool isDpadHotkeyPressed();

		/**
		 * Returns if the function button/hotkey is pressed, override in derived board class
		 */
		virtual bool isSOCDHotkeyPressed();

		inline bool isDpadUpPressed()           { return (state.dpad    & GAMEPAD_MASK_UP)    == GAMEPAD_MASK_UP   ; }
		inline bool isDpadDownPressed()         { return (state.dpad    & GAMEPAD_MASK_DOWN)  == GAMEPAD_MASK_DOWN ; }
		inline bool isDpadLeftPressed()         { return (state.dpad    & GAMEPAD_MASK_LEFT)  == GAMEPAD_MASK_LEFT ; }
		inline bool isDpadRightPressed()        { return (state.dpad    & GAMEPAD_MASK_RIGHT) == GAMEPAD_MASK_RIGHT; }
		inline bool isLeftStickButtonPressed()  { return (state.buttons & GAMEPAD_MASK_11)    == GAMEPAD_MASK_11   ; }
		inline bool isRightStickButtonPressed() { return (state.buttons & GAMEPAD_MASK_12)    == GAMEPAD_MASK_12   ; }
		inline bool isSelectPressed()           { return (state.buttons & GAMEPAD_MASK_09)    == GAMEPAD_MASK_09   ; }
		inline bool isStartPressed()            { return (state.buttons & GAMEPAD_MASK_10)    == GAMEPAD_MASK_10   ; }

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
					GamepadDebouncer(GAMEPAD_MASK_01, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_02, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_03, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_04, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_05, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_06, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_07, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_08, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_09, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_10, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_11, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_12, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_13, GAMEPAD_DEBOUNCE_MILLIS, false),
					GamepadDebouncer(GAMEPAD_MASK_14, GAMEPAD_DEBOUNCE_MILLIS, false),
				};
		#else
				GamepadDebouncer debouncers[GAMEPAD_DIGITAL_INPUT_COUNT];
		#endif
};

static GamepadClass Gamepad;

#endif
