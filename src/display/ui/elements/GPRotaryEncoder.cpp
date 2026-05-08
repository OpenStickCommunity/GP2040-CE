#include "GPRotaryEncoder.h"
#include <cmath>
#include <algorithm>

void GPRotaryEncoder::draw() {
    double scaleX = this->getScaleX();
    double scaleY = this->getScaleY();

    if ((scaleX > 0.0f) && ((scaleY == 0.0f) || (scaleY == 1.0f))) scaleY = scaleX;
    else if (((scaleX == 0.0f) || (scaleX == 1.0f)) && (scaleY > 0.0f)) scaleX = scaleY;

    uint16_t offsetX = ((getRenderer()->getDriver()->getMetrics()->width -
        (uint16_t)((double)getRenderer()->getDriver()->getMetrics()->width * scaleX)) / 2);

    uint16_t cx = (scaleX > 0.0f)
        ? (uint16_t)(this->x * scaleX + this->getViewport().left + offsetX)
        : this->x;
    uint16_t cy = (scaleY > 0.0f)
        ? (uint16_t)(this->y * scaleY + this->getViewport().top)
        : this->y;

    uint16_t r    = (uint16_t)(_radius * scaleX);
    uint16_t dotR = std::max((uint16_t)1, (uint16_t)(r / 4));

    // outer ring (hollow)
    getRenderer()->drawEllipse(cx, cy, r, r, this->strokeColor, 0);

    // sweeping indicator dot — 0° = 12-o'clock, positive = clockwise
    double rad = (_angle - 90.0) * M_PI / 180.0;
    int dotX = (int)cx + (int)std::round(r * cos(rad));
    int dotY = (int)cy + (int)std::round(r * sin(rad));
    getRenderer()->drawEllipse((uint16_t)dotX, (uint16_t)dotY, dotR, dotR,
                               this->strokeColor, this->strokeColor);
}
