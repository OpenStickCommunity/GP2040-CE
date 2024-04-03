#ifndef _GPSHAPE_H_
#define _GPSHAPE_H_

#include "GPWidget.h"
#include "enums.pb.h"

class GPShape : public GPWidget {
    public:
        void draw();
        GPShape* setSize(uint16_t sizeX, uint16_t sizeY) { this->_sizeX = sizeX; this->_sizeY = sizeY; return this; }
        GPShape* setAngle(double angle) { this->_angle = angle; return this; }
        GPShape* setAngleEnd(double angleEnd) { this->_angleEnd = angleEnd; return this; }
        GPShape* setClosed(bool closed) { this->_closed = closed; return this; }
        GPShape* setShape(GPShape_Type shape) { this->_shape = shape; return this; }
    private:
        uint16_t _sizeX = 0;
        uint16_t _sizeY = 0;
        double _angle = 0;
        double _angleEnd = 0;
        bool _closed = false;
        GPShape_Type _shape = GP_SHAPE_ELLIPSE;
};

#endif