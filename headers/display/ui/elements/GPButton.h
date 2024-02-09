#ifndef _GPBUTTON_H_
#define _GPBUTTON_H_

#include "GPWidget.h"

class GPButton : public GPWidget {
    public:
        void draw();
        void setRadius(uint16_t radius) { this->_radius = radius; }
        void setInputMask(int16_t inputMask) { this->_inputMask = inputMask; }
    private:
        uint16_t _radius = 0;
        int16_t _inputMask = -1;
};

#endif