#ifndef _TRAINEXTENSION_H_
#define _TRAINEXTENSION_H_

#include "ExtensionBase.h"

enum TrainButtons {
    TRAIN_BUTTON_C   = WiiButtons::WII_BUTTON_A,
    TRAIN_BUTTON_B   = WiiButtons::WII_BUTTON_B,
    TRAIN_BUTTON_D   = WiiButtons::WII_BUTTON_X,
    TRAIN_BUTTON_A   = WiiButtons::WII_BUTTON_Y
};

class TrainExtension : public ExtensionBase {
    public:
        void process(uint8_t *inputData) override;
        uint8_t prepareOutput() override;
};

#endif