#include "GPButton.h"
#include "GPGFX_UI_layouts.h"

void GPButton::draw() {
    // new style button:
    uint16_t baseX = this->x;
    uint16_t baseY = this->y;
    Mask_t pinValues = ~gpio_get_all();

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
        baseY = ((this->y) * scaleY + this->getViewport().top);
    }

    bool buttonState = false;
    bool turboState = false;
    uint16_t state = 0;
    int32_t maskedPins = 0;
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();

    if (_inputType == GP_ELEMENT_BTN_BUTTON) {
        // button mask
        buttonState = getProcessedGamepad()->pressedButton(this->_inputMask);
        turboState = (getGamepad()->turboState.buttons & this->_inputMask);
    } else if (_inputType == GP_ELEMENT_DIR_BUTTON) {
        // direction button mask
        buttonState = getProcessedGamepad()->pressedDpad(this->_inputMask);
    }

    state = buttonState

    // base
    if (this->_shape == GP_SHAPE_ELLIPSE) {
        uint16_t scaledSize = (uint16_t)((double)this->_sizeX * scaleX);
        uint16_t baseRadius = (uint16_t)scaledSize;
        uint16_t turboRadius = (uint16_t)scaledSize * GP_BUTTON_TURBO_SCALE;

        getRenderer()->drawEllipse(baseX, baseY, baseRadius, baseRadius, this->strokeColor, state);
        if (turboState) getRenderer()->drawEllipse(baseX, baseY, turboRadius, turboRadius, 1, 0);
    } else if (this->_shape == GP_SHAPE_SQUARE) {
        uint16_t sizeX = (this->_sizeX) * scaleX + this->getViewport().left;
        uint16_t sizeY = (this->_sizeY) * scaleY + this->getViewport().top;
        uint16_t width = sizeX - baseX;
        uint16_t height = sizeY - baseY;
        uint16_t turboW = (uint16_t)round(width * GP_BUTTON_TURBO_SCALE);
        uint16_t turboH = (uint16_t)round(height * GP_BUTTON_TURBO_SCALE);
        uint16_t turboX = baseX + (width - turboW) / 2;
        uint16_t turboY = baseY + (height - turboH) / 2;

        getRenderer()->drawRectangle(baseX, baseY, sizeX+offsetX, sizeY, this->strokeColor, state, this->_angle);
        if (turboState) getRenderer()->drawRectangle(turboX, turboY, turboX+turboW, turboY+turboH, 1, 0, this->_angle);
    } else if (this->_shape == GP_SHAPE_LINE) {
        getRenderer()->drawLine(baseX, baseY, this->_sizeX, this->_sizeY, this->strokeColor, 0);
    } else if (this->_shape == GP_SHAPE_POLYGON) {
        uint16_t scaledSize = (uint16_t)((double)this->_sizeX * scaleX);
        uint16_t baseRadius = (uint16_t)scaledSize;
        uint16_t turboRadius = (uint16_t)scaledSize * GP_BUTTON_TURBO_SCALE;

        getRenderer()->drawPolygon(baseX, baseY, baseRadius, this->_sizeY, this->strokeColor, state, this->_angle);
        if (turboState) getRenderer()->drawPolygon(baseX, baseY, turboRadius, this->_sizeY, 1, 0, this->_angle);
    } else if (this->_shape == GP_SHAPE_ARC) {
        uint16_t scaledSize = (uint16_t)((double)this->_sizeX * scaleX);
        uint16_t baseRadius = (uint16_t)scaledSize;
        uint16_t turboRadius = (uint16_t)scaledSize * GP_BUTTON_TURBO_SCALE;

        getRenderer()->drawArc(baseX, baseY, baseRadius, baseRadius, this->strokeColor, state, this->_angle, this->_angleEnd, this->_closed);
        if (turboState) getRenderer()->drawArc(baseX, baseY, turboRadius, turboRadius, 1, 0, this->_angle, this->_angleEnd, this->_closed);
    } else if (this->_shape == GP_SHAPE_PILL) {
        uint16_t sizeX = (this->_sizeX) * scaleX + this->getViewport().left;
        uint16_t sizeY = (this->_sizeY) * scaleY + this->getViewport().top;
        uint16_t width = sizeX - baseX;
        uint16_t height = sizeY - baseY;
        uint16_t turboW = (uint16_t)round(width * GP_BUTTON_TURBO_SCALE);
        uint16_t turboH = (uint16_t)round(height * GP_BUTTON_TURBO_SCALE);
        uint16_t turboX = baseX + (width - turboW) / 2;
        uint16_t turboY = baseY + (height - turboH) / 2;

        getRenderer()->drawPill(baseX, baseY, sizeX+offsetX, sizeY, this->strokeColor, state, this->_angle);
        if (turboState) getRenderer()->drawPill(turboX, turboY, turboX+turboW, turboY+turboH, 1, 0, this->_angle);
    }
}
