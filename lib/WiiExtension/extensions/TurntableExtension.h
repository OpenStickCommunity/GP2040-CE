#ifndef _TURNTABLEEXTENSION_H_
#define _TURNTABLEEXTENSION_H_

#include "ExtensionBase.h"

enum TurntableButtons {
    TURNTABLE_RIGHT_GREEN = WiiButtons::WII_BUTTON_Y,
    TURNTABLE_RIGHT_RED   = WiiButtons::WII_BUTTON_X,
    TURNTABLE_RIGHT_BLUE  = WiiButtons::WII_BUTTON_A,
    TURNTABLE_EUPHORIA    = WiiButtons::WII_BUTTON_ZR,
    TURNTABLE_LEFT_GREEN  = WiiButtons::WII_BUTTON_LEFT,
    TURNTABLE_LEFT_RED    = WiiButtons::WII_BUTTON_UP,
    TURNTABLE_LEFT_BLUE   = WiiButtons::WII_BUTTON_RIGHT
};

enum TurntableAnalogs {
    TURNTABLE_LEFT        = WiiAnalogs::WII_ANALOG_RIGHT_Y,
    TURNTABLE_RIGHT       = WiiAnalogs::WII_ANALOG_RIGHT_X,
    TURNTABLE_EFFECTS     = WiiAnalogs::WII_ANALOG_LEFT_TRIGGER,
    TURNTABLE_CROSSFADE   = WiiAnalogs::WII_ANALOG_RIGHT_TRIGGER
};

#define WII_TURNTABLE_GATE_SIZE 97
#define WII_TURNTABLE_GATE_CENTER 128
#define WII_TURNTABLE_TRIGGER_MAX 64
#define WII_TURNTABLE_ANALOG_GAP (WII_TURNTABLE_GATE_CENTER-WII_TURNTABLE_GATE_SIZE)
#define WII_TURNTABLE_MAX_PRECISION WII_ANALOG_PRECISION_0

class TurntableExtension : public ExtensionBase {
    private:
        bool _ledState = false;
    public:
        void init(uint8_t dataType) override;
        void process(uint8_t *inputData) override;

        void setLED(bool ledOn) {_ledState = ledOn;};
        bool getLED() {return _ledState;};
};

#endif