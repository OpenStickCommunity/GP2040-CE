#ifndef _InputMacro_H
#define _InputMacro_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef INPUT_MACRO_ENABLED
#define INPUT_MACRO_ENABLED 0
#endif

#ifndef INPUT_MACRO_PIN
#define INPUT_MACRO_PIN -1
#endif

#define INPUT_HOLD_MS 16

// Input Macro Module Name
#define InputMacroName "Input Macro"

class InputMacro : public GPAddon {
public:
	virtual bool available();   // GPAddon available
	virtual void setup();       // Analog Setup
	virtual void process() {};     // Analog Process
	virtual void preprocess();
    virtual std::string name() { return InputMacroName; }
private:
	int macroPosition = -1;
	bool isMacroRunning = false;
	bool isMacroTriggerHeld = false;
	uint32_t macroStartTime = 0;
	uint32_t macroTriggerDebounceStartTime = 0;

	int macroInputPosition = 0;
	bool macroInputPressed = false;
	uint32_t macroInputHoldTime = INPUT_HOLD_MS;
	bool prevMacroInputPressed = false;

	MacroOptions inputMacroOptions;
};

#endif  // _InputMacro_H_