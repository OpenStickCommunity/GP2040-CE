#include "GPSlider.h"
#include "enums.pb.h"
#include "gamepad/GamepadState.h"
#include <algorithm>

void GPSlider::draw() {
    double scaleX = this->getScaleX();
    double scaleY = this->getScaleY();

    if ((scaleX > 0.0f) && ((scaleY == 0.0f) || (scaleY == 1.0f))) scaleY = scaleX;
    else if (((scaleX == 0.0f) || (scaleX == 1.0f)) && (scaleY > 0.0f)) scaleX = scaleY;

    uint16_t offsetX = ((getRenderer()->getDriver()->getMetrics()->width -
        (uint16_t)((double)getRenderer()->getDriver()->getMetrics()->width * scaleX)) / 2);

    uint16_t tx  = (scaleX > 0.0f)
        ? (uint16_t)(this->x * scaleX + this->getViewport().left + offsetX)
        : this->x;
    uint16_t top = (scaleY > 0.0f)
        ? (uint16_t)(this->y * scaleY + this->getViewport().top)
        : this->y;
    uint16_t h   = (uint16_t)(_trackHeight * scaleY);
    uint16_t bot = top + h;

    // vertical track line
    getRenderer()->drawLine(tx, top, tx, bot, this->strokeColor, 0);

    // position the dot based on analog input
    bool rightAnalog = (_axisMode & GP_LEVER_MODE_RIGHT_ANALOG) == GP_LEVER_MODE_RIGHT_ANALOG;
    bool leftAnalog  = (_axisMode & GP_LEVER_MODE_LEFT_ANALOG)  == GP_LEVER_MODE_LEFT_ANALOG;

    uint32_t analogY = rightAnalog ? getProcessedGamepad()->state.ry
                     : leftAnalog  ? getProcessedGamepad()->state.ly
                     : GAMEPAD_JOYSTICK_MID;

    uint16_t dotY = top + (uint16_t)((double)h * analogY / (double)GAMEPAD_JOYSTICK_MAX);
    dotY = (uint16_t)std::max((int)top, std::min((int)bot, (int)dotY));

    getRenderer()->drawEllipse(tx, dotY, 2, 2, this->strokeColor, this->strokeColor);
}
