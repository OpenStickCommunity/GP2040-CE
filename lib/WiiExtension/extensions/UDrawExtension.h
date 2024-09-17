#ifndef _UDRAWEXTENSION_H_
#define _UDRAWEXTENSION_H_

#include "ExtensionBase.h"

class UDrawExtension : public ExtensionBase {
    public:
        void process(uint8_t *inputData) override;
    private:
        const uint16_t UDRAW_MIN_X = 80;
        const uint16_t UDRAW_MAX_X = 1955;
        const uint16_t UDRAW_MIN_Y = 95;
        const uint16_t UDRAW_MAX_Y = 1450;
};

#endif