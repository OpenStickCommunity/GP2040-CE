#include "GPButton.h"
#include "GPGFX_UI_layouts.h"

void GPButton::draw() {
    // new style button:
    uint16_t baseX = this->x;
    uint16_t baseY = this->y;
    Mask_t values = ~gpio_get_all();

    uint16_t state = (this->_inputMask > -1 ? (this->_inputDirection ? getGamepad()->pressedDpad(this->_inputMask) : (_inputType == GP_ELEMENT_BTN_BUTTON ? getGamepad()->pressedButton(this->_inputMask) : ((values >> this->_inputMask) & 0x01))) : 0);

    // base
    if (this->_shape == GP_BUTTON_ELLIPSE) {
        uint16_t baseRadius = (uint16_t)this->_sizeX;
        uint16_t turboRadius = (uint16_t)this->_sizeX * GP_BUTTON_TURBO_SCALE;

        getRenderer()->drawEllipse(baseX, baseY, baseRadius, baseRadius, this->strokeColor, state);
        if (this->_inputType == GP_ELEMENT_BTN_BUTTON && (getGamepad()->turboState.buttons & this->_inputMask)) {
            getRenderer()->drawEllipse(baseX, baseY, turboRadius, turboRadius, 1, 0);
        }
    } else if (this->_shape == GP_BUTTON_SQUARE) {
        uint16_t width = this->_sizeX - baseX;
        uint16_t height = this->_sizeY - baseY;
        uint16_t turboW = (uint16_t)round(width * GP_BUTTON_TURBO_SCALE);
        uint16_t turboH = (uint16_t)round(height * GP_BUTTON_TURBO_SCALE);
        uint16_t turboX = baseX + (width - turboW) / 2;
        uint16_t turboY = baseY + (height - turboH) / 2;

        getRenderer()->drawRectangle(baseX, baseY, this->_sizeX, this->_sizeY, this->strokeColor, state);
        if (this->_inputType == GP_ELEMENT_BTN_BUTTON && (getGamepad()->turboState.buttons & this->_inputMask)) {
            getRenderer()->drawRectangle(turboX, turboY, turboX+turboW, turboY+turboH, 1, 0);
        }
    } else if (this->_shape == GP_BUTTON_DIAMOND) {
        uint16_t size = this->_sizeX;
        if (state) {
            int i;
            for (i = 0; i < size; i++) {
                getRenderer()->drawLine(baseX - i, baseY - size + i, baseX + i, baseY - size + i, this->strokeColor, 0);
                getRenderer()->drawLine(baseX - i, baseY + size - i, baseX + i, baseY + size - i, this->strokeColor, 0);
            }
            getRenderer()->drawLine(baseX - size, baseY, baseX + size, baseY, this->strokeColor, 0); // Fill in the middle
        }
        getRenderer()->drawLine(baseX - size, baseY, baseX, baseY - size, this->strokeColor, 0);
        getRenderer()->drawLine(baseX, baseY - size, baseX + size, baseY, this->strokeColor, 0);
        getRenderer()->drawLine(baseX + size, baseY, baseX, baseY + size, this->strokeColor, 0);
        getRenderer()->drawLine(baseX, baseY + size, baseX - size, baseY, this->strokeColor, 0);
        if (this->_inputType == GP_ELEMENT_BTN_BUTTON && (getGamepad()->turboState.buttons & this->_inputMask)) {
        }
    } else if (this->_shape == GP_BUTTON_POLYGON) {
        uint16_t baseRadius = (uint16_t)this->_sizeX;
        uint16_t turboRadius = (uint16_t)this->_sizeX * GP_BUTTON_TURBO_SCALE;

        getRenderer()->drawPolygon(baseX, baseY, baseRadius, this->_sizeY, this->strokeColor, state);
        if (this->_inputType == GP_ELEMENT_BTN_BUTTON && (getGamepad()->turboState.buttons & this->_inputMask)) {
            getRenderer()->drawPolygon(baseX, baseY, turboRadius, this->_sizeY, 1, 0);
        }
    }
}
