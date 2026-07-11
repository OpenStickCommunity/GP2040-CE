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

/**
 * @brief Run SOCD cleaning against a D-pad value.
 *
 * @param mode The SOCD cleaning mode.
 * @param dpad The GamepadState.dpad value.
 * @return uint8_t The clean D-pad value.
 */
uint8_t runSOCDCleaner(SOCDMode mode, uint8_t dpad)
{
	if (mode == SOCD_MODE_BYPASS) return dpad;

	static DpadDirection lastUD = DIRECTION_NONE;
	static DpadDirection lastLR = DIRECTION_NONE;
	static uint8_t prevDpad = 0;
	static bool lastOutputDiagonal = false;
	
	uint8_t newDpad = 0;
	uint8_t pressed = dpad & ~prevDpad; // 检测本帧新按下的按键
	bool lrConflict = (dpad & GAMEPAD_MASK_LEFT) && (dpad & GAMEPAD_MASK_RIGHT);

	// 上下方向：后输入优先
	switch (dpad & (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN))
	{
		case (GAMEPAD_MASK_UP | GAMEPAD_MASK_DOWN):
			if (lastUD != DIRECTION_NONE)
				newDpad |= (lastUD == DIRECTION_UP) ? GAMEPAD_MASK_DOWN : GAMEPAD_MASK_UP;
			else
				lastUD = DIRECTION_NONE;
			break;
		case GAMEPAD_MASK_UP:
			newDpad |= GAMEPAD_MASK_UP;
			lastUD = DIRECTION_UP;
			break;
		case GAMEPAD_MASK_DOWN:
			newDpad |= GAMEPAD_MASK_DOWN;
			lastUD = DIRECTION_DOWN;
			break;
		default:
			lastUD = DIRECTION_NONE;
			break;
	}

	// 左右方向：后输入优先
	switch (dpad & (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT))
	{
		case (GAMEPAD_MASK_LEFT | GAMEPAD_MASK_RIGHT):
			if (lastLR != DIRECTION_NONE)
				newDpad |= (lastLR == DIRECTION_LEFT) ? GAMEPAD_MASK_RIGHT : GAMEPAD_MASK_LEFT;
			else
				lastLR = DIRECTION_NONE;
			break;
		case GAMEPAD_MASK_LEFT:
			newDpad |= GAMEPAD_MASK_LEFT;
			lastLR = DIRECTION_LEFT;
			break;
		case GAMEPAD_MASK_RIGHT:
			newDpad |= GAMEPAD_MASK_RIGHT;
			lastLR = DIRECTION_RIGHT;
			break;
		default:
			lastLR = DIRECTION_NONE;
			break;
	}

	// 核心规则：斜向切换到对侧纯横向时，清除下方向，无中间斜向过渡
	// 仅当：左右冲突 + 新按下的是纯左右（不带下） + 上一帧输出是斜向 时触发
	if (lrConflict 
		&& !(pressed & GAMEPAD_MASK_DOWN) 
		&& lastOutputDiagonal)
	{
		newDpad &= ~GAMEPAD_MASK_DOWN;
	}

	// 更新状态
	prevDpad = dpad;
	lastOutputDiagonal = (newDpad & GAMEPAD_MASK_DOWN) 
		&& ((newDpad & GAMEPAD_MASK_LEFT) || (newDpad & GAMEPAD_MASK_RIGHT));

	return newDpad;
}
