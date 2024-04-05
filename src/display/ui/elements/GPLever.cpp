#include "GPLever.h"

void GPLever::draw() {
    // new style lever:
    // radius defines the base of the lever
    // the lever indicator itself will be sized slightly smaller than the base

    int baseX = this->x;
    int baseY = this->y;

    int leverX = this->x;
    int leverY = this->y;

    // scale to viewport
    double scaleX = this->getScaleX();
    double scaleY = this->getScaleY();

    // set scale on X & Y to be proportionate if either is 0
    if ((scaleX > 0.0f) & ((scaleY == 0.0f) || (scaleY == 1.0f))) {
        scaleY = scaleX;
    } else if (((scaleX == 0.0f) || (scaleX == 1.0f)) & (scaleY > 0.0f)) {
        scaleX = scaleY;
    }

    uint16_t offsetX = ((getRenderer()->getDriver()->getMetrics()->width - (uint16_t)((double)getRenderer()->getDriver()->getMetrics()->width * scaleX)) / 2);

    if (scaleX > 0.0f) {
        baseX = ((this->x) * scaleX + this->getViewport().left) + offsetX;
        leverX = ((this->x) * scaleX + this->getViewport().left) + offsetX;
    }

    if (scaleY > 0.0f) {
        baseY = ((this->y) * scaleY + this->getViewport().top);
        leverY = ((this->y) * scaleY + this->getViewport().top);
    }

    int baseRadius = (int)(((double)this->_radius * 1.00) * scaleX);
    int leverRadius = (int)(((double)this->_radius * 0.75) * scaleY);

    if (this->_inputType == DPAD_MODE_DIGITAL) {
        // dpad
        bool upState    = (this->_upMask > -1 ? getGamepad()->pressedButton((uint16_t)this->_upMask) : getGamepad()->pressedUp());
        bool leftState  = (this->_leftMask > -1 ? getGamepad()->pressedButton((uint16_t)this->_leftMask) : getGamepad()->pressedLeft());
        bool downState  = (this->_downMask > -1 ? getGamepad()->pressedButton((uint16_t)this->_downMask) : getGamepad()->pressedDown());
        bool rightState = (this->_rightMask > -1 ? getGamepad()->pressedButton((uint16_t)this->_rightMask) : getGamepad()->pressedRight());
        if (upState) {
            leverY -= leverRadius;
            if (leftState) {
                leverX -= leverRadius;
            } else if (rightState) {
                leverX += leverRadius;
            }
        } else if (downState) {
            leverY += leverRadius;
            if (leftState) {
                leverX -= leverRadius;
            } else if (rightState) {
                leverX += leverRadius;
            }
        } else if (leftState) {
            leverX -= leverRadius;
        } else if (rightState) {
            leverX += leverRadius;
        }
    } else {
        // analog
        uint16_t analogX = map((this->_inputType == DPAD_MODE_LEFT_ANALOG ? getGamepad()->state.lx : getGamepad()->state.rx), 0, 0xFFFF, 0, 100);
        uint16_t analogY = map((this->_inputType == DPAD_MODE_LEFT_ANALOG ? getGamepad()->state.ly : getGamepad()->state.ry), 0, 0xFFFF, 0, 100);

        uint16_t minX = std::max(0,(baseX - baseRadius));
        uint16_t maxX = std::min((baseX + baseRadius),128);
        uint16_t offsetX = (analogX * 0.01) * (maxX - minX);
        uint16_t minY = std::max(0,(baseY - baseRadius));
        uint16_t maxY = std::min((baseY + baseRadius),64);
        uint16_t offsetY = (analogY * 0.01) * (maxY - minY);

        // move lever around
        leverX = minX + offsetX;
        leverY = minY + offsetY;
    }

    // base
    getRenderer()->drawEllipse(baseX, baseY, baseRadius, baseRadius, this->strokeColor, 0);
    
    if (this->_showCardinal) {
        uint16_t cardinalSize = 3;
        uint16_t cardinalN = std::max(0,(baseY - baseRadius)-cardinalSize);
        uint16_t cardinalS = std::min((baseY + baseRadius),64);
        uint16_t cardinalE = std::max(0,(baseX - baseRadius)-cardinalSize);
        uint16_t cardinalW = std::min((baseX + baseRadius),128);
        getRenderer()->drawLine(baseX, cardinalN, baseX, cardinalN+cardinalSize, this->strokeColor, 1);
        getRenderer()->drawLine(baseX, cardinalS, baseX, cardinalS+cardinalSize, this->strokeColor, 1);
        getRenderer()->drawLine(cardinalE, baseY, cardinalE+cardinalSize, baseY, this->strokeColor, 1);
        getRenderer()->drawLine(cardinalW, baseY, cardinalW+cardinalSize, baseY, this->strokeColor, 1);
    }

    if (this->_showOrdinal) {
        uint16_t ordinalSize = 2;
        for (int angle = 45; angle <= 315; angle += 90) {
            // Convert angle to radians
            double radians = angle * M_PI / 180.0;
    
            // Calculate coordinates of point on ellipse
            int xEllipse = baseX + baseRadius * cos(radians);
            int yEllipse = baseY + baseRadius * sin(radians);
    
            // Calculate coordinates of endpoint of line
            int xEndpoint = xEllipse + ordinalSize * cos(radians);
            int yEndpoint = yEllipse + ordinalSize * sin(radians);
    
            // Draw line from point on ellipse to endpoint
            getRenderer()->drawLine(xEllipse, yEllipse, xEndpoint, yEndpoint, this->strokeColor, 1);
        }
    }

    // lever
    getRenderer()->drawEllipse(leverX, leverY, leverRadius, leverRadius, this->strokeColor, 1);
}

void GPLever::setDirectionMasks(int32_t upMask, int32_t downMask, int32_t leftMask, int32_t rightMask) {
    this->_upMask = upMask;
    this->_downMask = downMask;
    this->_leftMask = leftMask;
    this->_rightMask = rightMask;
}