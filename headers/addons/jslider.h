#ifndef _JSlider_H
#define _JSlider_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef JSLIDER_ENABLED
#define JSLIDER_ENABLED 0
#endif

#ifndef PIN_SLIDER_LS
#define PIN_SLIDER_LS    -1
#endif

#ifndef PIN_SLIDER_RS
#define PIN_SLIDER_RS    -1
#endif

// JSlider Module Name
#define JSliderName "JSlider"

class JSliderInput : public GPAddon {
public:
    virtual bool available();
	virtual void setup();       // JSlider Button Setup
    virtual void preprocess() {}
	virtual void process();     // JSlider process
    virtual std::string name() { return JSliderName; }
private:
    DpadMode read();
    void debounce();
    DpadMode dpadState;           // Saved locally for debounce
    DpadMode dDebState;          // Debounce JSlider State
    uint32_t uDebTime;          // Debounce JSlider Time
};

#endif  // _JSlider_H_