#ifndef _GPSLIDER_H_
#define _GPSLIDER_H_

#include "GPWidget.h"

class GPSlider : public GPWidget {
public:
    GPSlider() {}
    virtual ~GPSlider() {}
    virtual void draw();

    void setTrackHeight(uint16_t h) { _trackHeight = h; }
    void setAxisMode(uint16_t mode) { _axisMode = mode; }

private:
    uint16_t _trackHeight = 40;
    uint16_t _axisMode = 0;  // GPLever_Mode bitmask
};

#endif
