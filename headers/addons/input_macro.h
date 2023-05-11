#ifndef _InputMacro_H
#define _InputMacro_H

#include "gpaddon.h"

#include "GamepadEnums.h"

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
};

#endif  // _InputMacro_H_