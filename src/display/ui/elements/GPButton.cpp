#include "GPButton.h"

void GPButton::draw() {
    // new style button:
    // radius defines the button

    int baseRadius = (int)this->_radius * 1.00;
    int turboRadius = (int)this->_radius * 0.70;
    int baseX = this->x;
    int baseY = this->y;
    int state = (this->_inputMask > -1 ? getGamepad()->pressedButton(this->_inputMask) : 0);

    // base
    getRenderer()->drawEllipse(baseX, baseY, baseRadius, baseRadius, this->strokeColor, state);
    if (getGamepad()->turboState.buttons & this->_inputMask) {
        getRenderer()->drawEllipse(baseX, baseY, turboRadius, turboRadius, 1, 0);
    }
}
