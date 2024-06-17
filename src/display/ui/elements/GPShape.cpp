#include "GPShape.h"

void GPShape::draw() {
    uint16_t baseX = this->x;
    uint16_t baseY = this->y;

    // scale to viewport
    double scaleX = this->getScaleX();
    double scaleY = this->getScaleY();

    // set scale on X & Y to be proportionate if either is 0
    if ((scaleX > 0.0f) & ((scaleY == 0.0f) || (scaleY == 1.0f))) {
        scaleY = scaleX;
    } else if (((scaleX == 0.0f) || (scaleX == 1.0f)) & (scaleY > 0.0f)) {
        scaleX = scaleY;
    }

    uint16_t offsetX = ((getRenderer()->getDriver()->getMetrics()->width - (uint16_t)((double)(this->getViewport().right - this->getViewport().left) * scaleX)) / 2);
    uint16_t offsetY = ((getRenderer()->getDriver()->getMetrics()->height - (uint16_t)((double)(this->getViewport().bottom - this->getViewport().top) * scaleY)) / 2);

    if (scaleX > 0.0f) {
        baseX = ((this->x) * scaleX + this->getViewport().left) + offsetX;
    }

    if (scaleY > 0.0f) {
        baseY = (this->y) * scaleY + this->getViewport().top;
    }

    // base
    if (this->_shape == GP_SHAPE_ELLIPSE) {
        uint16_t scaledSize = (uint16_t)((double)this->_sizeX * scaleX);
        uint16_t baseRadius = (uint16_t)scaledSize;

        getRenderer()->drawEllipse(baseX, baseY, baseRadius, baseRadius, this->strokeColor, this->fillColor);
    } else if (this->_shape == GP_SHAPE_SQUARE) {
        uint16_t sizeX = (this->_sizeX) * scaleX + this->getViewport().left;
        uint16_t sizeY = (this->_sizeY) * scaleY + this->getViewport().top;
        uint16_t width = this->_sizeX - baseX;
        uint16_t height = this->_sizeY - baseY;

        getRenderer()->drawRectangle(baseX, baseY, sizeX+offsetX, sizeY, this->strokeColor, this->fillColor, this->_angle);
    } else if (this->_shape == GP_SHAPE_LINE) {
        getRenderer()->drawLine(baseX, baseY, this->_sizeX, this->_sizeY, this->strokeColor, 0);
    } else if (this->_shape == GP_SHAPE_POLYGON) {
        uint16_t scaledSize = (uint16_t)((double)this->_sizeX * scaleX);
        uint16_t baseRadius = (uint16_t)scaledSize;

        getRenderer()->drawPolygon(baseX, baseY, baseRadius, this->_sizeY, this->strokeColor, this->fillColor, this->_angle);
    } else if (this->_shape == GP_SHAPE_ARC) {
        uint16_t scaledSize = (uint16_t)((double)this->_sizeX * scaleX);
        uint16_t baseRadius = (uint16_t)scaledSize;

        getRenderer()->drawArc(baseX, baseY, baseRadius, baseRadius, this->strokeColor, this->fillColor, this->_angle, this->_angleEnd, this->_closed);
    }
}
