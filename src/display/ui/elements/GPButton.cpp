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

    bool pinState = false;
    bool buttonState = false;
    bool turboState = false;
    uint16_t state = 0;
    int16_t setPin = -1;
    int32_t maskedPins = 0;
    bool useMask = false;
    GamepadButtonMapping *mapMask = NULL;
    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();

    if (_inputType == GP_ELEMENT_BTN_BUTTON) {
        // button mask
        buttonState = getProcessedGamepad()->pressedButton(this->_inputMask);
        useMask = true;

        if ((this->_inputMask & GAMEPAD_MASK_B1) == GAMEPAD_MASK_B1) {
            mapMask = getGamepad()->mapButtonB1;
        } else if ((this->_inputMask & GAMEPAD_MASK_B2) == GAMEPAD_MASK_B2) {
            mapMask = getGamepad()->mapButtonB2;
        } else if ((this->_inputMask & GAMEPAD_MASK_B3) == GAMEPAD_MASK_B3) {
            mapMask = getGamepad()->mapButtonB3;
        } else if ((this->_inputMask & GAMEPAD_MASK_B4) == GAMEPAD_MASK_B4) {
            mapMask = getGamepad()->mapButtonB4;
        } else if ((this->_inputMask & GAMEPAD_MASK_L1) == GAMEPAD_MASK_L1) {
            mapMask = getGamepad()->mapButtonL1;
        } else if ((this->_inputMask & GAMEPAD_MASK_R1) == GAMEPAD_MASK_R1) {
            mapMask = getGamepad()->mapButtonR1;
        } else if ((this->_inputMask & GAMEPAD_MASK_L2) == GAMEPAD_MASK_L2) {
            mapMask = getGamepad()->mapButtonL2;
        } else if ((this->_inputMask & GAMEPAD_MASK_R2) == GAMEPAD_MASK_R2) {
            mapMask = getGamepad()->mapButtonR2;
        } else if ((this->_inputMask & GAMEPAD_MASK_S1) == GAMEPAD_MASK_S1) {
            mapMask = getGamepad()->mapButtonS1;
        } else if ((this->_inputMask & GAMEPAD_MASK_S2) == GAMEPAD_MASK_S2) {
            mapMask = getGamepad()->mapButtonS2;
        } else if ((this->_inputMask & GAMEPAD_MASK_L3) == GAMEPAD_MASK_L3) {
            mapMask = getGamepad()->mapButtonL3;
        } else if ((this->_inputMask & GAMEPAD_MASK_R3) == GAMEPAD_MASK_R3) {
            mapMask = getGamepad()->mapButtonR3;
        } else if ((this->_inputMask & GAMEPAD_MASK_A1) == GAMEPAD_MASK_A1) {
            mapMask = getGamepad()->mapButtonA1;
        } else if ((this->_inputMask & GAMEPAD_MASK_A2) == GAMEPAD_MASK_A2) {
            mapMask = getGamepad()->mapButtonA2;
        }
        turboState = (getGamepad()->turboState.buttons & this->_inputMask);
    } else if (_inputType == GP_ELEMENT_DIR_BUTTON) {
        // direction button mask
        buttonState = getProcessedGamepad()->pressedDpad(this->_inputMask);
        useMask = true;

        if ((this->_inputMask & GAMEPAD_MASK_UP) == GAMEPAD_MASK_UP) {
            mapMask = getGamepad()->mapDpadUp;
        } else if ((this->_inputMask & GAMEPAD_MASK_DOWN) == GAMEPAD_MASK_DOWN) {
            mapMask = getGamepad()->mapDpadDown;
        } else if ((this->_inputMask & GAMEPAD_MASK_LEFT) == GAMEPAD_MASK_LEFT) {
            mapMask = getGamepad()->mapDpadLeft;
        } else if ((this->_inputMask & GAMEPAD_MASK_RIGHT) == GAMEPAD_MASK_RIGHT) {
            mapMask = getGamepad()->mapDpadRight;
        }
    } else if (_inputType == GP_ELEMENT_PIN_BUTTON) {
        // physical pin
        pinState = ((pinValues >> this->_inputMask) & 0x01);
        buttonState = true;

        switch (pinMappings[this->_inputMask].action) {
            case GpioAction::BUTTON_PRESS_B1: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_B1); break;
            case GpioAction::BUTTON_PRESS_B2: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_B2); break;
            case GpioAction::BUTTON_PRESS_B3: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_B3); break;
            case GpioAction::BUTTON_PRESS_B4: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_B4); break;
            case GpioAction::BUTTON_PRESS_L1: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_L1); break;
            case GpioAction::BUTTON_PRESS_R1: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_R1); break;
            case GpioAction::BUTTON_PRESS_L2: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_L2); break;
            case GpioAction::BUTTON_PRESS_R2: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_R2); break;
            case GpioAction::BUTTON_PRESS_S1: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_S1); break;
            case GpioAction::BUTTON_PRESS_S2: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_S2); break;
            case GpioAction::BUTTON_PRESS_A1: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_A1); break;
            case GpioAction::BUTTON_PRESS_A2: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_A2); break;
            case GpioAction::BUTTON_PRESS_L3: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_L3); break;
            case GpioAction::BUTTON_PRESS_R3: turboState |= (getGamepad()->turboState.buttons & GAMEPAD_MASK_R3); break;
            default: break;
        }
    }

    if (useMask && mapMask != NULL) {
        maskedPins = (pinValues & mapMask->pinMask);
        for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
            if ((maskedPins & (1 << pin)) == (1 << pin)) {
                setPin = pin;
                break;
            }
        }

        if (setPin > -1) {
            pinState = ((pinValues >> setPin) & 0x01);
        }
    }

    state = (buttonState ? pinState : 0);

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
    }
}
