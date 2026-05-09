#include "GPRotaryEncoder.h"
#include <cmath>

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

    uint16_t r = (uint16_t)(_radius * scaleX);
    if (r == 0) r = _radius;

    // 0-360 dial gauge: outer ring + cardinal tick marks + needle indicator.
    // 0 = 12-o'clock, positive angles sweep clockwise.
    getRenderer()->drawEllipse(cx, cy, r, r, this->strokeColor, 0);

    // Cardinal tick marks just outside the ring (N/E/S/W) so the dial
    // reads as a 0/90/180/270 gauge regardless of where the needle points.
    for (int i = 0; i < 4; i++) {
        double tickRad = (i * 90.0 - 90.0) * M_PI / 180.0;
        int tx = (int)cx + (int)std::round((r + 1) * cos(tickRad));
        int ty = (int)cy + (int)std::round((r + 1) * sin(tickRad));
        getRenderer()->drawPixel((uint16_t)tx, (uint16_t)ty, this->strokeColor);
    }

    // Needle from center to ring edge representing the current dial angle.
    double rad = (_angle - 90.0) * M_PI / 180.0;
    int handX = (int)cx + (int)std::round(r * cos(rad));
    int handY = (int)cy + (int)std::round(r * sin(rad));
    getRenderer()->drawLine(cx, cy, (uint16_t)handX, (uint16_t)handY, this->strokeColor, 0);
}
