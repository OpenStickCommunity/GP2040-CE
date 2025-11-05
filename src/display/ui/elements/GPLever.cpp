#include "GPLever.h"

#include "drivermanager.h"

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

    // any zero-defined levers should be forced to dpad to avoid broken functionality. to be fixed.
    if (this->_inputType == GPLever_Mode::GP_LEVER_MODE_NONE) this->_inputType = GPLever_Mode::GP_LEVER_MODE_DPAD;

    bool dpadInput = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_DPAD) == GPLever_Mode::GP_LEVER_MODE_DPAD);
    bool digitalOutput = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_DIGITAL) == GPLever_Mode::GP_LEVER_MODE_DIGITAL);
    bool leftAnalog = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_LEFT_ANALOG) == GPLever_Mode::GP_LEVER_MODE_LEFT_ANALOG);
    bool rightAnalog = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_RIGHT_ANALOG) == GPLever_Mode::GP_LEVER_MODE_RIGHT_ANALOG);
    bool invertX = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_INVERT_X) == GPLever_Mode::GP_LEVER_MODE_INVERT_X);
    bool invertY = ((this->_inputType & GPLever_Mode::GP_LEVER_MODE_INVERT_Y) == GPLever_Mode::GP_LEVER_MODE_INVERT_Y);

    if (digitalOutput) {
        // digital directions regardless of how
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
    } else if (dpadInput) {
        // whatever the switchable dpad input is
        bool upState    = getGamepad()->state.dpadOriginal & GAMEPAD_MASK_UP;
        bool leftState  = getGamepad()->state.dpadOriginal & GAMEPAD_MASK_LEFT;
        bool downState  = getGamepad()->state.dpadOriginal & GAMEPAD_MASK_DOWN;
        bool rightState = getGamepad()->state.dpadOriginal & GAMEPAD_MASK_RIGHT;
        if (upState != downState) {
            leverY -= upState ? (!invertY ? leverRadius : -leverRadius) : (!invertY ? -leverRadius : leverRadius);
        }
        if (leftState != rightState) {
            leverX -= leftState ? (!invertX ? leverRadius : -leverRadius) : (!invertX ? -leverRadius : leverRadius);
        }
    } else if (leftAnalog || rightAnalog) {
        // Get the X/Y of each raw analog
        uint32_t analogX = leftAnalog ? getProcessedGamepad()->state.lx : getProcessedGamepad()->state.rx;
        uint32_t analogY = leftAnalog ? getProcessedGamepad()->state.ly : getProcessedGamepad()->state.ry;

        // Get the midpoint value for the current mode
        GPDriver * gpDriver = DriverManager::getInstance().getDriver();
        uint32_t joystickMid = GAMEPAD_JOYSTICK_MID;
        uint32_t joystickMax = GAMEPAD_JOYSTICK_MAX;
        if ( DriverManager::getInstance().getDriver() != nullptr ) {
            joystickMid = DriverManager::getInstance().getDriver()->GetJoystickMidValue();
            joystickMax = joystickMid * 2; // 0x8000 mid must be 0x10000 max, but we reduce by 1 if we're maxed out
        }

        // Check for inversion, flip with a clamp on 0x10000
        if ( invertX )
            analogX = std::min(joystickMax - analogX, (uint32_t)0xFFFF);
        if ( invertY )
            analogY = std::min(joystickMax - analogY, (uint32_t)0xFFFF);

        // Calculate location based off our driver mid
        leverX = (baseX-baseRadius) + baseRadius * (analogX / (float)joystickMid);
        leverY = (baseY-baseRadius) + baseRadius * (analogY / (float)joystickMid);
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
