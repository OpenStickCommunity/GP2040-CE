#ifndef INPUT_HISTORY_H_
#define INPUT_HISTORY_H_

#include "OneBitDisplay.h"

#include <vector>
#include <string>
#include "gpaddon.h"
#include <deque>
#include <array>

#ifndef INPUT_HISTORY_ENABLED
#define INPUT_HISTORY_ENABLED 0
#endif

#ifndef INPUT_HISTORY_LENGTH
#define INPUT_HISTORY_LENGTH 21
#endif

#ifndef INPUT_HISTORY_COL
#define INPUT_HISTORY_COL 0
#endif

#ifndef INPUT_HISTORY_ROW
#define INPUT_HISTORY_ROW 7
#endif

#define InputHistoryName "InputHistory"

class InputHistoryAddon : public GPAddon
{
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return InputHistoryName; }
	virtual void drawHistory(OBDISP *pOBD);
private:
	uint32_t historyLength;
	uint32_t col;
	uint32_t row;
	std::string historyString;
	std::deque<std::string> inputHistory;
	std::array<bool, 18> lastInput;
	Gamepad* gamepad;
	Gamepad* pGamepad;
	bool pressedUp();
	bool pressedDown();
	bool pressedLeft();
	bool pressedRight();
};

#endif
