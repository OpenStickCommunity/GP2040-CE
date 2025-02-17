#ifndef _SliderSOCD_H
#define _SliderSOCD_H

#include "gpaddon.h"
#include "types.h"

#include "GamepadEnums.h"

#ifndef SLIDER_SOCD_ENABLED
#define SLIDER_SOCD_ENABLED 0
#endif

#ifndef SLIDER_SOCD_SLOT_ONE
#define SLIDER_SOCD_SLOT_ONE SOCD_MODE_UP_PRIORITY
#endif

#ifndef SLIDER_SOCD_SLOT_TWO 
#define SLIDER_SOCD_SLOT_TWO  SOCD_MODE_SECOND_INPUT_PRIORITY
#endif

#ifndef SLIDER_SOCD_SLOT_DEFAULT
#define SLIDER_SOCD_SLOT_DEFAULT SOCD_MODE_NEUTRAL
#endif

// Slider Module Name
#define SliderSOCDName "SliderSOCD"

class SliderSOCDInput : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // SliderSOCD Button Setup
    virtual void reinit();
    virtual void preprocess() {}
    virtual void process();     // SliderSOCD process
    virtual void postprocess(bool sent) {}
    virtual std::string name() { return SliderSOCDName; }
private:
    SOCDMode read();

    Mask_t upPrioModeMask = 0;
    Mask_t neutralModeMask = 0;
    Mask_t secondInputModeMask = 0;
    Mask_t firstInputModeMask = 0;
    Mask_t bypassModeMask = 0;
};

#endif  // _SliderSOCD_H_
