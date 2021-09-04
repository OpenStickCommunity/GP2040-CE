/* 
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 * SPDX-FileCopyrightText: Copyright (c) 2013 thomasfredericks
 *
 * Debouncer class ported from the Arduino Bounce2 library (MIT license).
 */

#include "GamepadDebouncer.h"

GamepadDebouncer::GamepadDebouncer() : previousMillis(0), intervalMillis(10), state(0) {}

void GamepadDebouncer::setInterval(uint16_t intervalMillis)
{
	this->intervalMillis = intervalMillis;
}

void GamepadDebouncer::begin()
{
	state = 0;
	if (readCurrentState())
	{
		setStateFlag(DEBOUNCED_STATE | UNSTABLE_STATE);
	}

#ifdef BOUNCE_LOCK_OUT
	previousMillis = 0;
#else
	previousMillis = getMillis();
#endif
}

bool GamepadDebouncer::update()
{
	unsetStateFlag(CHANGED_STATE);

	bool readState = readCurrentState();

	if (readState != getStateFlag(DEBOUNCED_STATE))
	{
		if (getMillis() - previousMillis >= intervalMillis)
		{
			changeState();
		}
	}

	if (readState != getStateFlag(UNSTABLE_STATE))
	{
		toggleStateFlag(UNSTABLE_STATE);
		previousMillis = getMillis();
	}

	return changed();
}

unsigned long GamepadDebouncer::previousDuration() const
{
	return durationOfPreviousState;
}

unsigned long GamepadDebouncer::currentDuration() const
{
	return getMillis() - stateChangeLastTime;
}

inline void GamepadDebouncer::changeState()
{
	toggleStateFlag(DEBOUNCED_STATE);
	setStateFlag(CHANGED_STATE);
	durationOfPreviousState = getMillis() - stateChangeLastTime;
	stateChangeLastTime = getMillis();
}

bool GamepadDebouncer::read() const
{
	return getStateFlag(DEBOUNCED_STATE);
}

bool GamepadDebouncer::rose() const
{
	return getStateFlag(DEBOUNCED_STATE) && getStateFlag(CHANGED_STATE);
}

bool GamepadDebouncer::fell() const
{
	return !getStateFlag(DEBOUNCED_STATE) && getStateFlag(CHANGED_STATE);
}

bool GamepadDebouncer::readCurrentState()
{
	if (isDpad)
		return gamepadState->dpad & inputMask;
	else
		return gamepadState->buttons & inputMask;
}
