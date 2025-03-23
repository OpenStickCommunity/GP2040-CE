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

    bool dpadInput = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_DIGITAL) == GPLever_Mode::GP_LEVER_MODE_DIGITAL);
    bool leftAnalog = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_LEFT_ANALOG) == GPLever_Mode::GP_LEVER_MODE_LEFT_ANALOG);
    bool rightAnalog = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_RIGHT_ANALOG) == GPLever_Mode::GP_LEVER_MODE_RIGHT_ANALOG);
    bool invertX = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_INVERT_X) == GPLever_Mode::GP_LEVER_MODE_INVERT_X);
    bool invertY = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_INVERT_Y) == GPLever_Mode::GP_LEVER_MODE_INVERT_Y);

    if (dpadInput) {
        // dpad
        bool upState    = (this->_upMask > -1 ? getProcessedGamepad()->pressedButton((uint16_t)this->_upMask) : getProcessedGamepad()->pressedUp());
        bool leftState  = (this->_leftMask > -1 ? getProcessedGamepad()->pressedButton((uint16_t)this->_leftMask) : getProcessedGamepad()->pressedLeft());
        bool downState  = (this->_downMask > -1 ? getProcessedGamepad()->pressedButton((uint16_t)this->_downMask) : getProcessedGamepad()->pressedDown());
        bool rightState = (this->_rightMask > -1 ? getProcessedGamepad()->pressedButton((uint16_t)this->_rightMask) : getProcessedGamepad()->pressedRight());
        if (upState != downState) {
            leverY -= upState ? (!invertY ? leverRadius : -leverRadius) : (!invertY ? -leverRadius : leverRadius);
        }
        if (leftState != rightState) {
            leverX -= leftState ? (!invertX ? leverRadius : -leverRadius) : (!invertX ? -leverRadius : leverRadius);
        }
    } else if (leftAnalog || rightAnalog) {
        // analog
        uint16_t analogX = map((leftAnalog ? getProcessedGamepad()->state.lx : getProcessedGamepad()->state.rx), (!invertX ? 0 : 0xFFFF), (!invertX ? 0xFFFF : 0), 0, 100);
        uint16_t analogY = map((leftAnalog ? getProcessedGamepad()->state.ly : getProcessedGamepad()->state.ry), (!invertY ? 0 : 0xFFFF), (!invertY ? 0xFFFF : 0), 0, 100);

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
