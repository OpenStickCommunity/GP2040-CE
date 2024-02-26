#include "GPShape.h"

void GPShape::draw() {
    uint16_t baseX = this->x;
    uint16_t baseY = this->y;

    // base
    if (this->_shape == GP_SHAPE_ELLIPSE) {
        uint16_t baseRadius = (uint16_t)this->_sizeX;

        getRenderer()->drawEllipse(baseX, baseY, baseRadius, baseRadius, this->strokeColor, this->fillColor);
    } else if (this->_shape == GP_SHAPE_SQUARE) {
        uint16_t width = this->_sizeX - baseX;
        uint16_t height = this->_sizeY - baseY;

        getRenderer()->drawRectangle(baseX, baseY, this->_sizeX, this->_sizeY, this->strokeColor, this->fillColor);
    } else if (this->_shape == GP_SHAPE_DIAMOND) {
        uint16_t size = this->_sizeX;
        getRenderer()->drawLine(baseX - size, baseY, baseX, baseY - size, this->strokeColor, 0);
        getRenderer()->drawLine(baseX, baseY - size, baseX + size, baseY, this->strokeColor, 0);
        getRenderer()->drawLine(baseX + size, baseY, baseX, baseY + size, this->strokeColor, 0);
        getRenderer()->drawLine(baseX, baseY + size, baseX - size, baseY, this->strokeColor, 0);
    } else if (this->_shape == GP_SHAPE_POLYGON) {
        uint16_t baseRadius = (uint16_t)this->_sizeX;

        getRenderer()->drawPolygon(baseX, baseY, baseRadius, this->_sizeY, this->strokeColor, this->fillColor, this->_angle);
    } else if (this->_shape == GP_SHAPE_ARC) {
        uint16_t baseRadius = (uint16_t)this->_sizeX;

        getRenderer()->drawArc(baseX, baseY, baseRadius, baseRadius, this->strokeColor, this->fillColor, this->_angle, this->_angleEnd, this->_closed);
    }
}
