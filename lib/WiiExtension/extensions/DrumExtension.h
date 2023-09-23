#ifndef _DRUMEXTENSION_H_
#define _DRUMEXTENSION_H_

#include "ExtensionBase.h"

enum DrumButtons {
    DRUM_ORANGE = WiiButtons::WII_BUTTON_ZL,
    DRUM_RED    = WiiButtons::WII_BUTTON_B,
    DRUM_BLUE   = WiiButtons::WII_BUTTON_Y,
    DRUM_GREEN  = WiiButtons::WII_BUTTON_A,
    DRUM_YELLOW = WiiButtons::WII_BUTTON_X,
    DRUM_PEDAL  = WiiButtons::WII_BUTTON_ZR
};

class DrumExtension : public ExtensionBase {
    public:
        void process(uint8_t *inputData) override;
};

#endif