#ifndef _CLASSICEXTENSION_H_
#define _CLASSICEXTENSION_H_

#include "ExtensionBase.h"

class ClassicExtension : public ExtensionBase {
    public:
        void init(uint8_t dataType) override;
        void calibrate(uint8_t *calibrationData) override;
        void process(uint8_t *inputData) override;
};

#endif