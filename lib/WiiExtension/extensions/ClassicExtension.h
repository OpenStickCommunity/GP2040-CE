#ifndef _CLASSICEXTENSION_H_
#define _CLASSICEXTENSION_H_

#include "ExtensionBase.h"

#define WII_CLASSIC_GATE_SIZE 97
#define WII_CLASSIC_GATE_CENTER 128
#define WII_CLASSIC_TRIGGER_MAX 32
#define WII_CLASSIC_ANALOG_GAP (WII_CLASSIC_GATE_CENTER-WII_CLASSIC_GATE_SIZE)

class ClassicExtension : public ExtensionBase {
    public:
        void init(uint8_t dataType) override;
        bool calibrate(uint8_t *calibrationData) override;
        void process(uint8_t *inputData) override;
};

#endif