#ifndef _NUNCHUCKEXTENSION_H_
#define _NUNCHUCKEXTENSION_H_

#include "ExtensionBase.h"

class NunchuckExtension : public ExtensionBase {
    public:
        void init(uint8_t dataType) override;
        void calibrate(uint8_t *calibrationData) override;
        void process(uint8_t *inputData) override;
};

#endif