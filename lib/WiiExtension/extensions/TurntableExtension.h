#ifndef _TURNTABLEEXTENSION_H_
#define _TURNTABLEEXTENSION_H_

#include "ExtensionBase.h"

enum TurntableButtons {
    TURNTABLE_LEFT_RED    = WiiButtons::WII_BUTTON_L,
    TURNTABLE_RIGHT_RED   = WiiButtons::WII_BUTTON_R,
    TURNTABLE_LEFT_GREEN  = WiiButtons::WII_BUTTON_X,
    TURNTABLE_LEFT_BLUE   = WiiButtons::WII_BUTTON_ZL,
    TURNTABLE_RIGHT_GREEN = WiiButtons::WII_BUTTON_Y,
    TURNTABLE_RIGHT_BLUE  = WiiButtons::WII_BUTTON_ZR,
    TURNTABLE_EUPHORIA    = WiiButtons::WII_BUTTON_A
};

enum TurntableAnalogs {
    TURNTABLE_LEFT        = WiiAnalogs::WII_ANALOG_TRIGGER_RIGHT,
    TURNTABLE_RIGHT       = WiiAnalogs::WII_ANALOG_RIGHT_X,
    TURNTABLE_EFFECTS     = WiiAnalogs::WII_ANALOG_TRIGGER_LEFT,
    TURNTABLE_CROSSFADE   = WiiAnalogs::WII_ANALOG_RIGHT_Y
};

class TurntableExtension : public ExtensionBase {
    public:
        void process(uint8_t *inputData) override;
};

#endif