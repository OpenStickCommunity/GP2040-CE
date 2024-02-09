#include "GPButton.h"

void GPButton::draw() {
    // new style button:
    // radius defines the button

    int baseRadius = (int)this->_radius * 1.00;
    int baseX = this->x;
    int baseY = this->y;
    //int state = (this->_inputMask > -1 ? pressedButton(this->_inputMask) : 0);
    int state = 1;

    // base
    getRenderer()->drawEllipse(baseX, baseY, baseRadius, baseRadius, this->strokeColor, state);
}
