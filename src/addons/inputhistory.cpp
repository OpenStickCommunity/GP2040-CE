#include <map>
#include "addons/inputhistory.h"
#include "storagemanager.h"
#include "math.h"
#include "usb_driver.h"
#include "helper.h"
#include "config.pb.h"

const map<uint16_t, uint16_t> displayModeLookup = {
    {INPUT_MODE_HID, 0},
    {INPUT_MODE_SWITCH, 1},
    {INPUT_MODE_XINPUT, 2},
    {INPUT_MODE_XBONE, 2},
    {INPUT_MODE_KEYBOARD, 3},
    {INPUT_MODE_CONFIG, 3}, 
    {INPUT_MODE_PS4, 4},
    {INPUT_MODE_PSCLASSIC, 4},
    {INPUT_MODE_MDMINI, 5},
    {INPUT_MODE_NEOGEO, 6},
    {INPUT_MODE_PCEMINI, 7},
    {INPUT_MODE_EGRET, 8},
    {INPUT_MODE_ASTRO, 9},
    {INPUT_MODE_XBOXORIGINAL, 10},
};

static const std::string displayNames[][INPUT_HISTORY_MAX_INPUTS] = {
	{		// HID / DINPUT
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
			CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			CHAR_CROSS, CHAR_CIRCLE, CHAR_SQUARE, CHAR_TRIANGLE,
			"L1", "R1", "L2", "R2",
			"SL", "ST", "L3", "R3", "PS", "A2"
	},
	{		// Switch
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			"B", "A", "Y", "X",
			"L", "R", "ZL", "ZR",
			"-", "+", "LS", "RS", CHAR_HOME_S, CHAR_CAP_S
	},
	{		// XInput
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			"A", "B", "X", "Y",
			"LB", "RB", "LT", "RT",
			CHAR_VIEW_X, CHAR_MENU_X, "LS", "RS", CHAR_HOME_X, "A2"
	},
	{		// Keyboard / HID-KB
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			"B1", "B2", "B3", "B4",
			"L1", "R1", "L2", "R2",
			"S1", "S2", "L3", "R3", "A1", "A2"
	},
	{		// PS4
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			CHAR_CROSS, CHAR_CIRCLE, CHAR_SQUARE, CHAR_TRIANGLE,
			"L1", "R1", "L2", "R2",
			CHAR_SHARE_P, "OP", "L3", "R3", CHAR_HOME_P, CHAR_TPAD_P
	},
	{		// GEN/MD Mini
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			"A", "B", "X", "Y",
			"", "Z", "", "C",
			"M", "S", "", "", "", ""
	},
	{		// Neo Geo Mini
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			"B", "D", "A", "C",
			"", "", "", "",
			"SE", "ST", "", "", "", ""
	},
	{		// PC Engine/TG16 Mini
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			"I", "II", "", "",
			"", "", "", "",
			"SE", "RUN", "", "", "", ""
	},
	{		// Egret II Mini
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			"A", "B", "C", "D",
			"", "E", "", "F",
			"CRD", "ST", "", "", "MN", ""
	},
	{		// Astro City Mini
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			"A", "B", "D", "E",
			"", "C", "", "F",
			"CRD", "ST", "", "", "", ""
	},
	{		// Original Xbox
			CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
            CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
			"A", "B", "X", "Y",
			"BL", "WH", "L", "R",
			"BK", "ST", "LS", "RS", "", ""
	}
};

bool InputHistoryAddon::available() {
	const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
	const InputHistoryOptions& options = Storage::getInstance().getAddonOptions().inputHistoryOptions;
	return displayOptions.enabled && options.enabled;
}

void InputHistoryAddon::setup() {
	const InputHistoryOptions& options = Storage::getInstance().getAddonOptions().inputHistoryOptions;
	historyLength = options.length;
	col = options.col;
	row = options.row;

	gamepad = Storage::getInstance().GetGamepad();
	pGamepad = Storage::getInstance().GetProcessedGamepad();
}

void InputHistoryAddon::process() {
	std::deque<std::string> pressed;

	// Get key states
	std::array<bool, INPUT_HISTORY_MAX_INPUTS> currentInput = {

		pressedUp(),
		pressedDown(),
		pressedLeft(),
		pressedRight(),

		pressedUpLeft(),
		pressedUpRight(),
		pressedDownLeft(),
		pressedDownRight(),

		gamepad->pressedB1(),
		gamepad->pressedB2(),
		gamepad->pressedB3(),
		gamepad->pressedB4(),
		gamepad->pressedL1(),
		gamepad->pressedR1(),
		gamepad->pressedL2(),
		gamepad->pressedR2(),
		gamepad->pressedS1(),
		gamepad->pressedS2(),
		gamepad->pressedL3(),
		gamepad->pressedR3(),
		gamepad->pressedA1(),
		gamepad->pressedA2(),
	};

	uint8_t mode = ((displayModeLookup.count(gamepad->getOptions().inputMode) > 0) ? displayModeLookup.at(gamepad->getOptions().inputMode) : 0);

	// Check if any new keys have been pressed
	if (lastInput != currentInput) {
		// Iterate through array
		for (uint8_t x=0; x<INPUT_HISTORY_MAX_INPUTS; x++) {
			// Add any pressed keys to deque
			if (currentInput[x] && (displayNames[mode][x] != "")) pressed.push_back(displayNames[mode][x]);
		}
		// Update the last keypress array
		lastInput = currentInput;
	}

	if (pressed.size() > 0) {
		std::string newInput;
		for(const auto &s : pressed) {
				if(!newInput.empty())
						newInput += "+";
				newInput += s;
		}

		inputHistory.push_back(newInput);
	}

	if (inputHistory.size() > (historyLength / 2) + 1) {
		inputHistory.pop_front();
	}

	std::string ret;

	for (auto it = inputHistory.crbegin(); it != inputHistory.crend(); ++it) {
		std::string newRet = ret;
		if (!newRet.empty())
			newRet = " " + newRet;

		newRet = *it + newRet;
		ret = newRet;

		if (ret.size() >= historyLength) {
			break;
		}
	}

	if(ret.size() >= historyLength) {
		historyString = ret.substr(ret.size() - historyLength);
	} else {
		historyString = ret;
	}
}

void InputHistoryAddon::drawHistory(OBDISP *pOBD) {
	obdWriteString(pOBD, 0, col * 6, row, (char *)historyString.c_str(), FONT_6x8, 0, 0);
}

bool InputHistoryAddon::pressedUp()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((pGamepad->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_UP);
		case DPAD_MODE_LEFT_ANALOG:  return pGamepad->state.ly == GAMEPAD_JOYSTICK_MIN;
		case DPAD_MODE_RIGHT_ANALOG: return pGamepad->state.ry == GAMEPAD_JOYSTICK_MIN;
	}

	return false;
}

bool InputHistoryAddon::pressedDown()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((pGamepad->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_DOWN);
		case DPAD_MODE_LEFT_ANALOG:  return pGamepad->state.ly == GAMEPAD_JOYSTICK_MAX;
		case DPAD_MODE_RIGHT_ANALOG: return pGamepad->state.ry == GAMEPAD_JOYSTICK_MAX;
	}

	return false;
}

bool InputHistoryAddon::pressedLeft()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((pGamepad->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_LEFT);
		case DPAD_MODE_LEFT_ANALOG:  return pGamepad->state.lx == GAMEPAD_JOYSTICK_MIN;
		case DPAD_MODE_RIGHT_ANALOG: return pGamepad->state.rx == GAMEPAD_JOYSTICK_MIN;
	}

	return false;
}

bool InputHistoryAddon::pressedRight()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((pGamepad->state.dpad & GAMEPAD_MASK_DPAD) == GAMEPAD_MASK_RIGHT);
		case DPAD_MODE_LEFT_ANALOG:  return pGamepad->state.lx == GAMEPAD_JOYSTICK_MAX;
		case DPAD_MODE_RIGHT_ANALOG: return pGamepad->state.rx == GAMEPAD_JOYSTICK_MAX;
	}

	return false;
}

bool InputHistoryAddon::pressedUpLeft()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((pGamepad->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT));
		case DPAD_MODE_LEFT_ANALOG:  return (pGamepad->state.lx == GAMEPAD_JOYSTICK_MIN) && (pGamepad->state.ly == GAMEPAD_JOYSTICK_MIN);
		case DPAD_MODE_RIGHT_ANALOG: return (pGamepad->state.rx == GAMEPAD_JOYSTICK_MIN) && (pGamepad->state.ry == GAMEPAD_JOYSTICK_MIN);
	}

	return false;
}

bool InputHistoryAddon::pressedUpRight()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((pGamepad->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT));
		case DPAD_MODE_LEFT_ANALOG:  return (pGamepad->state.lx == GAMEPAD_JOYSTICK_MAX) && (pGamepad->state.ly == GAMEPAD_JOYSTICK_MIN);
		case DPAD_MODE_RIGHT_ANALOG: return (pGamepad->state.lx == GAMEPAD_JOYSTICK_MAX) && (pGamepad->state.ly == GAMEPAD_JOYSTICK_MIN);
	}

	return false;
}

bool InputHistoryAddon::pressedDownLeft()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((pGamepad->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT));
		case DPAD_MODE_LEFT_ANALOG:  return (pGamepad->state.lx == GAMEPAD_JOYSTICK_MIN) && (pGamepad->state.ly == GAMEPAD_JOYSTICK_MAX);
		case DPAD_MODE_RIGHT_ANALOG: return (pGamepad->state.lx == GAMEPAD_JOYSTICK_MIN) && (pGamepad->state.ly == GAMEPAD_JOYSTICK_MAX);
	}

	return false;
}

bool InputHistoryAddon::pressedDownRight()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return ((pGamepad->state.dpad & GAMEPAD_MASK_DPAD) == (GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT));
		case DPAD_MODE_LEFT_ANALOG:  return (pGamepad->state.lx == GAMEPAD_JOYSTICK_MAX) && (pGamepad->state.ly == GAMEPAD_JOYSTICK_MAX);
		case DPAD_MODE_RIGHT_ANALOG: return (pGamepad->state.lx == GAMEPAD_JOYSTICK_MAX) && (pGamepad->state.ly == GAMEPAD_JOYSTICK_MAX);
	}

	return false;
}
