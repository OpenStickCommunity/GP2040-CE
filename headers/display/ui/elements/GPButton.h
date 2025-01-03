#ifndef _GPBUTTON_H_
#define _GPBUTTON_H_

#include "GPWidget.h"
#include "enums.pb.h"

#define GP_BUTTON_TURBO_SCALE 0.70

class GPButton : public GPWidget {
    public:
        void draw();
        GPButton* setSize(uint16_t sizeX, uint16_t sizeY) { this->_sizeX = sizeX; this->_sizeY = sizeY; return this; }
        GPButton* setInputMask(int16_t inputMask) { this->_inputMask = inputMask; return this; }
        GPButton* setInputDirection(bool inputDirection) { this->_inputDirection = inputDirection; return this; }
        GPButton* setInputType(GPElement inputType) { this->_inputType = inputType; return this; }
        GPButton* setAngle(double angle) { this->_angle = angle; return this; }
        GPButton* setAngleEnd(double angleEnd) { this->_angleEnd = angleEnd; return this; }
        GPButton* setClosed(bool closed) { this->_closed = closed; return this; }
        GPButton* setShape(GPShape_Type shape) { this->_shape = shape; return this; }
    private:
        uint16_t _sizeX = 0;
        uint16_t _sizeY = 0;
        double _angle = 0;
        double _angleEnd = 0;
        bool _closed = false;
        int32_t _inputMask = -1;
        bool _inputDirection = false;
        GPElement _inputType = GP_ELEMENT_BTN_BUTTON;
        GPShape_Type _shape = GP_SHAPE_ELLIPSE;
};

#endif