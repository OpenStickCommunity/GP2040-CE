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

    // Center within the viewport band, not the full screen (see GPButton.cpp).
    uint16_t offsetX = (((this->getViewport().right - this->getViewport().left) - (uint16_t)((double)(this->getViewport().right - this->getViewport().left) * scaleX)) / 2);
    uint16_t offsetY = (((this->getViewport().bottom - this->getViewport().top) - (uint16_t)((double)(this->getViewport().bottom - this->getViewport().top) * scaleY)) / 2);

    // Match GPButton's input-history lift so shapes move with the buttons.
    const uint16_t INPUT_HISTORY_LAYOUT_LIFT = 6;
    if (this->getViewport().top > 0) {
        // Clamp so an over-large lift can never wrap the unsigned offset.
        offsetY -= (offsetY < INPUT_HISTORY_LAYOUT_LIFT) ? offsetY : INPUT_HISTORY_LAYOUT_LIFT;
    }

    if (scaleX > 0.0f) {
        baseX = ((this->x) * scaleX + this->getViewport().left) + offsetX;
    }

    if (scaleY > 0.0f) {
        baseY = (this->y) * scaleY + this->getViewport().top + offsetY;
    }

    // base
    if (this->_shape == GP_SHAPE_ELLIPSE) {
        uint16_t scaledSize = (uint16_t)((double)this->_sizeX * scaleX);
        uint16_t baseRadius = (uint16_t)scaledSize;

        getRenderer()->drawEllipse(baseX, baseY, baseRadius, baseRadius, this->strokeColor, this->fillColor);
    } else if (this->_shape == GP_SHAPE_SQUARE) {
        uint16_t sizeX = (this->_sizeX) * scaleX + this->getViewport().left;
        uint16_t sizeY = (this->_sizeY) * scaleY + this->getViewport().top;

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
    } else if (this->_shape == GP_SHAPE_PILL) {
        uint16_t sizeX = (this->_sizeX) * scaleX + this->getViewport().left;
        uint16_t sizeY = (this->_sizeY) * scaleY + this->getViewport().top;

        getRenderer()->drawPill(baseX, baseY, sizeX+offsetX, sizeY, this->strokeColor, this->fillColor, this->_angle);
    }
}
