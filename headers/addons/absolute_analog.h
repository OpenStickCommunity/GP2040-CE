#ifndef _AbsoluteAnalog_H
#define _AbsoluteAnalog_H

#include "gpaddon.h"
#include "gamepad.h"

#ifndef ABSOLUTE_ANALOG_ENABLED
#define ABSOLUTE_ANALOG_ENABLED 0
#endif

#define ABSOLUTE_ANALOG_COUNT 9

// Axis values are stored as -255 to 255
#define ABSOLUTE_ANALOG_AXIS_MAX 255

#define AbsoluteAnalogName "AbsoluteAnalog"

// Also used by the hotkey handler in gamepad.cpp
void applyAbsoluteAnalogValue(GamepadState& state, uint8_t index);

class AbsoluteAnalogAddon : public GPAddon {
public:
    virtual bool available();
    virtual void setup();
    virtual void process();
    virtual void preprocess() {}
    virtual void postprocess(bool sent) {}
    virtual void reinit();
    virtual std::string name() { return AbsoluteAnalogName; }
private:
    Mask_t pinMasks[ABSOLUTE_ANALOG_COUNT];
};

#endif  // _AbsoluteAnalog_H_
