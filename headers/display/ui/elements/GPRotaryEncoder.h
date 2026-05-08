#ifndef _GPROTARYENCODER_H_
#define _GPROTARYENCODER_H_

#include "GPWidget.h"

class GPRotaryEncoder : public GPWidget {
public:
    GPRotaryEncoder() {}
    virtual ~GPRotaryEncoder() {}
    virtual void draw();

    void setRadius(uint16_t r) { _radius = r; }
    void setEncoderIndex(uint8_t idx) { _encoderIndex = idx; }
    void setAngle(float a) { _angle = a; }
    float getAngle() const { return _angle; }
    uint8_t getEncoderIndex() const { return _encoderIndex; }

private:
    uint16_t _radius = 8;
    uint8_t  _encoderIndex = 0;
    float    _angle = 0.0f;  // degrees; 0 = 12-o'clock
};

#endif
