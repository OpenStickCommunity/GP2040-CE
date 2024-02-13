#ifndef _GPBUTTON_H_
#define _GPBUTTON_H_

#include "GPWidget.h"
#include "enums.pb.h"

#define GP_BUTTON_TURBO_SCALE 0.70

class GPButton : public GPWidget {
    public:
        void draw();
        GPButton* setSizeX(uint16_t sizeX) { this->_sizeX = sizeX; return this; }
        GPButton* setSizeY(uint16_t sizeY) { this->_sizeY = sizeY; return this; }
        GPButton* setInputMask(int16_t inputMask) { this->_inputMask = inputMask; return this; }
        GPButton* setInputDirection(bool inputDirection) { this->_inputDirection = inputDirection; return this; }
        GPButton* setShape(GPButton_Shape shape) { this->_shape = shape; return this; }
    private:
        uint16_t _sizeX = 0;
        uint16_t _sizeY = 0;
        int16_t _inputMask = -1;
        bool _inputDirection = false;
        GPButton_Shape _shape = GP_BUTTON_ELLIPSE;
};

#endif