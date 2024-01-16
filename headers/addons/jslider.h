#ifndef _JSlider_H
#define _JSlider_H

#include "gpaddon.h"

#include "GamepadEnums.h"
#include "types.h"

#ifndef JSLIDER_ENABLED
#define JSLIDER_ENABLED 0
#endif

#ifndef SLIDER_MODE_ONE
#define SLIDER_MODE_ONE DPAD_MODE_LEFT_ANALOG
#endif

#ifndef SLIDER_MODE_TWO
#define SLIDER_MODE_TWO DPAD_MODE_RIGHT_ANALOG
#endif

#ifndef SLIDER_MODE_ZERO
#define SLIDER_MODE_ZERO DPAD_MODE_DIGITAL
#endif

// JSlider Module Name
#define JSliderName "JSlider"

class JSliderInput : public GPAddon {
public:
    virtual bool available();
	virtual void setup();       // JSlider Button Setup
    virtual void reinit();
    virtual void preprocess() {}
	virtual void process();     // JSlider process
    virtual std::string name() { return JSliderName; }
private:
    DpadMode read();
    void debounce();
    DpadMode dpadState;           // Saved locally for debounce
    DpadMode dDebState;          // Debounce JSlider State
    uint32_t uDebTime;          // Debounce JSlider Time

    Mask_t dpModeMask = 0;
    Mask_t lsModeMask = 0;
    Mask_t rsModeMask = 0;
};

#endif  // _JSlider_H_
