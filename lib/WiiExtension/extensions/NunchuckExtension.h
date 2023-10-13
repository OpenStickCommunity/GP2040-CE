#ifndef _NUNCHUCKEXTENSION_H_
#define _NUNCHUCKEXTENSION_H_

#include "ExtensionBase.h"

#define WII_NUNCHUCK_GATE_SIZE 96
#define WII_NUNCHUCK_GATE_CENTER 128

class NunchuckExtension : public ExtensionBase {
    public:
        void init(uint8_t dataType) override;
        bool calibrate(uint8_t *calibrationData) override;
        void process(uint8_t *inputData) override;
};

#endif