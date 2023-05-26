#ifndef _GUITAREXTENSION_H_
#define _GUITAREXTENSION_H_

#include "ExtensionBase.h"

#define WII_GUITAR_GATE_SIZE 97
#define WII_GUITAR_GATE_CENTER 128
#define WII_GUITAR_TRIGGER_MAX 32
#define WII_GUITAR_ANALOG_GAP (WII_GUITAR_GATE_CENTER-WII_GUITAR_GATE_SIZE)

enum GuitarButtons {
    GUITAR_ORANGE = WiiButtons::WII_BUTTON_ZL,
    GUITAR_RED    = WiiButtons::WII_BUTTON_B,
    GUITAR_BLUE   = WiiButtons::WII_BUTTON_Y,
    GUITAR_GREEN  = WiiButtons::WII_BUTTON_A,
    GUITAR_YELLOW = WiiButtons::WII_BUTTON_X,
    GUITAR_PEDAL  = WiiButtons::WII_BUTTON_ZR
};

class GuitarExtension : public ExtensionBase {
    public:
        void init(uint8_t dataType) override;
        void process(uint8_t *inputData) override;
    private:
        uint8_t _guitarType   = 0;
        uint16_t whammyBar    = 0;
        int8_t touchBar       = 0;
        bool isTouched        = 0;
};

#endif