#ifndef _InputMacro_H
#define _InputMacro_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef INPUT_MACRO_ENABLED
#define INPUT_MACRO_ENABLED 0
#endif

#ifndef BOARD_LED_PIN
#ifndef PICO_DEFAULT_LED_PIN
#define BOARD_LED_PIN 25
#else
#define BOARD_LED_PIN PICO_DEFAULT_LED_PIN
#endif
#endif

#ifndef INPUT_MACRO_BOARD_LED_ENABLED
#define INPUT_MACRO_BOARD_LED_ENABLED 0
#endif

#ifndef INPUT_MACRO_PIN
#define INPUT_MACRO_PIN -1
#endif

#define MAX_MACRO_INPUT_LIMIT 30
#define MAX_MACRO_LIMIT 6
#define INPUT_HOLD_US 16666

// Input Macro Module Name
#define InputMacroName "Input Macro"

class InputMacro : public GPAddon {
public:
    virtual bool available();   // GPAddon available
    virtual void setup();       // Analog Setup
    virtual void process() {};     // Analog Process
    virtual void preprocess();
    virtual void postprocess(bool sent) {}
    virtual void reinit();
    virtual std::string name() { return InputMacroName; }
private:
    void checkMacroPress();
    void checkMacroAction();
    void runCurrentMacro();
    void reset();
    void restart(Macro& macro);
    bool isMacroRunning;
    bool isMacroTriggerHeld;
    int macroPosition;
    uint32_t macroButtonMask;
    uint32_t macroPinMasks[6];
    uint64_t macroStartTime;
    uint64_t currentMicros;
    int pressedMacro;
    int macroInputPosition;
    uint32_t macroInputHoldTime;
    bool prevMacroInputPressed;
    bool boardLedEnabled;
    MacroOptions * inputMacroOptions;
};

#endif  // _InputMacro_H_