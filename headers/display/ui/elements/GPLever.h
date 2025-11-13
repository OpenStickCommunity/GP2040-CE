#ifndef _GPLEVER_H_
#define _GPLEVER_H_

#include "GPWidget.h"

class GPLever : public GPWidget {
    public:
        void draw();
        void setRadius(uint16_t radius) { this->_radius = radius; }
        void setInputType(uint16_t inputType) { this->_inputType = inputType; }
        void setShowCardinal(bool show) { this->_showCardinal = show; }
        void setShowOrdinal(bool show) { this->_showOrdinal = show; }
        void setDirectionMasks(int32_t upMask, int32_t downMask, int32_t leftMask, int32_t rightMask);
    private:
        uint16_t _radius = 0;
        uint16_t _inputType = 0;
        bool _showCardinal = false;
        bool _showOrdinal = false;
        int32_t _upMask = -1;
        int32_t _downMask = -1;
        int32_t _leftMask = -1;
        int32_t _rightMask = -1;
};

#endif