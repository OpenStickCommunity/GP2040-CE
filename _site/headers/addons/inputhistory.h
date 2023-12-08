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

#define INPUT_HISTORY_MAX_INPUTS 22

#define CHAR_TRIANGLE "\x80"
#define CHAR_CIRCLE   "\x81"
#define CHAR_CROSS    "\x82"
#define CHAR_SQUARE   "\x83"

#define CHAR_UP       "\x84"
#define CHAR_DOWN     "\x85"
#define CHAR_LEFT     "\x86"
#define CHAR_RIGHT    "\x87"

#define CHAR_UL       "\x88"
#define CHAR_UR       "\x89"
#define CHAR_DL       "\x8A"
#define CHAR_DR       "\x8B"

#define CHAR_HOME_S   "\x8C"
#define CHAR_CAP_S    "\x8D"

#define CHAR_VIEW_X   "\x8E"
#define CHAR_MENU_X   "\x8F"
#define CHAR_HOME_X   "\x90"

#define CHAR_TPAD_P   "\x91"
#define CHAR_HOME_P   "\x92"
#define CHAR_SHARE_P  "\x93"

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
	std::array<bool, INPUT_HISTORY_MAX_INPUTS> lastInput;
	Gamepad* gamepad;
	Gamepad* pGamepad;
	bool pressedUp();
	bool pressedDown();
	bool pressedLeft();
	bool pressedRight();

	bool pressedUpLeft();
	bool pressedUpRight();
	bool pressedDownLeft();
	bool pressedDownRight();
};

#endif
