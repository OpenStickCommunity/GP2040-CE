#include "GamepadState.h"
#include "drivermanager.h"

// Convert the horizontal GamepadState dpad axis value into an analog value
uint16_t dpadToAnalogX(uint8_t dpad)
{
	switch (dpad & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case GAMEPAD_MASK_LEFT:
			return GAMEPAD_JOYSTICK_MIN;

		case GAMEPAD_MASK_RIGHT:
			return GAMEPAD_JOYSTICK_MAX;

		default:
			if ( DriverManager::getInstance().getDriver() != nullptr )
				return DriverManager::getInstance().getDriver()->GetJoystickMidValue();
			else
				return GAMEPAD_JOYSTICK_MID;
	}
}

// Convert the vertical GamepadState dpad axis value into an analog value
uint16_t dpadToAnalogY(uint8_t dpad)
{
	switch (dpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
	{
		case GAMEPAD_MASK_UP:
			return GAMEPAD_JOYSTICK_MIN;

		case GAMEPAD_MASK_DOWN:
			return GAMEPAD_JOYSTICK_MAX;

		default:
			if ( DriverManager::getInstance().getDriver() != nullptr )
				return DriverManager::getInstance().getDriver()->GetJoystickMidValue();
			else
				return GAMEPAD_JOYSTICK_MID;
	}
}

uint8_t getMaskFromDirection(DpadDirection direction)
{
	return dpadMasks[direction-1];
}

uint8_t updateDpad(uint8_t dpad, DpadDirection direction)
{
	static bool inList[] = {false, false, false, false, false}; // correspond to DpadDirection: none, up, down, left, right
	static list<DpadDirection> dpadList;

	if(dpad & getMaskFromDirection(direction))
	{
		if(!inList[direction])
		{
			dpadList.push_back(direction);
			inList[direction] = true;
		}
	}
	else
	{
		if(inList[direction])
		{
			dpadList.remove(direction);
			inList[direction] = false;
		}
	}

	if(dpadList.empty()) {
		return 0;
	}
	else {
		return getMaskFromDirection(dpadList.back());
	}
}

/**
 * @brief Filter diagonals out of the dpad, making the device work as a 4-way lever.
 *
 * The most recent cardinal direction wins.
 *
 * @param dpad The GameState.dpad value.
 * @return uint8_t The new dpad value.
 */
uint8_t filterToFourWayMode(uint8_t dpad)
{
	updateDpad(dpad, DIRECTION_UP);
	updateDpad(dpad, DIRECTION_DOWN);
	updateDpad(dpad, DIRECTION_LEFT);
	return updateDpad(dpad, DIRECTION_RIGHT);
}

static DpadDirection lastUD = DIRECTION_NONE;
static DpadDirection lastLR = DIRECTION_NONE;

// maskA/dirA is up or left, maskB/dirB the opposing direction
static uint8_t cleanSOCDAxis(SOCDAxisMode mode, uint8_t dpad, uint8_t maskA, uint8_t maskB,
	DpadDirection dirA, DpadDirection dirB, DpadDirection& last)
{
	const bool pressedA = dpad & maskA;
	const bool pressedB = dpad & maskB;

	if (pressedA && pressedB)
	{
		if (mode == SOCD_AXIS_MODE_OFF)
			return maskA | maskB;
		else if (mode == SOCD_AXIS_MODE_UP_PRIORITY)
		{
			last = dirA;
			return maskA;
		}
		else if (mode == SOCD_AXIS_MODE_LAST_WIN && last != DIRECTION_NONE)
			return (last == dirA) ? maskB : maskA;
		else if (mode == SOCD_AXIS_MODE_FIRST_WIN && last != DIRECTION_NONE)
			return (last == dirA) ? maskA : maskB;

		last = DIRECTION_NONE;
		return 0;
	}

	if (pressedA)
	{
		last = dirA;
		return maskA;
	}

	if (pressedB)
	{
		last = dirB;
		return maskB;
	}

	last = DIRECTION_NONE;
	return 0;
}

uint8_t runSOCDCleanerPerAxis(SOCDAxisMode upDownMode, SOCDAxisMode leftRightMode, uint8_t dpad)
{
	// Up priority has no left/right meaning; we cannot know which way a character faces
	if (leftRightMode == SOCD_AXIS_MODE_UP_PRIORITY) leftRightMode = SOCD_AXIS_MODE_NEUTRAL;

	return cleanSOCDAxis(upDownMode, dpad, GAMEPAD_MASK_UP, GAMEPAD_MASK_DOWN,
			DIRECTION_UP, DIRECTION_DOWN, lastUD)
		| cleanSOCDAxis(leftRightMode, dpad, GAMEPAD_MASK_LEFT, GAMEPAD_MASK_RIGHT,
			DIRECTION_LEFT, DIRECTION_RIGHT, lastLR);
}

/**
 * @brief Run SOCD cleaning against a D-pad value.
 *
 * @param mode The SOCD cleaning mode.
 * @param dpad The GamepadState.dpad value.
 * @return uint8_t The clean D-pad value.
 */
uint8_t runSOCDCleaner(SOCDMode mode, uint8_t dpad)
{
	if (mode == SOCD_MODE_BYPASS) {
		return dpad;
	}

	switch (mode)
	{
		case SOCD_MODE_UP_PRIORITY:
			return runSOCDCleanerPerAxis(SOCD_AXIS_MODE_UP_PRIORITY, SOCD_AXIS_MODE_NEUTRAL, dpad);
		case SOCD_MODE_SECOND_INPUT_PRIORITY:
			return runSOCDCleanerPerAxis(SOCD_AXIS_MODE_LAST_WIN, SOCD_AXIS_MODE_LAST_WIN, dpad);
		case SOCD_MODE_FIRST_INPUT_PRIORITY:
			return runSOCDCleanerPerAxis(SOCD_AXIS_MODE_FIRST_WIN, SOCD_AXIS_MODE_FIRST_WIN, dpad);
		default:
			return runSOCDCleanerPerAxis(SOCD_AXIS_MODE_NEUTRAL, SOCD_AXIS_MODE_NEUTRAL, dpad);
	}
}
