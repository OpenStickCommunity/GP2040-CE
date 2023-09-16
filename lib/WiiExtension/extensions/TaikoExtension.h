#ifndef _TAIKOEXTENSION_H_
#define _TAIKOEXTENSION_H_

#include "ExtensionBase.h"

enum TaikoButtons {
    TATA_DON_LEFT    = WiiButtons::WII_BUTTON_B,
    TATA_KAT_LEFT    = WiiButtons::WII_BUTTON_Y,
    TATA_DON_RIGHT   = WiiButtons::WII_BUTTON_A,
    TATA_KAT_RIGHT   = WiiButtons::WII_BUTTON_X
};

class TaikoExtension : public ExtensionBase {
    public:
        void process(uint8_t *inputData) override;
};

#endif