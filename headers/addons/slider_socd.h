#ifndef _SliderSOCD_H
#define _SliderSOCD_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef SLIDER_SOCD_ENABLED
#define SLIDER_SOCD_ENABLED 0
#endif

#ifndef PIN_SLIDER_SOCD_UP
#define PIN_SLIDER_SOCD_UP    -1
#endif

#ifndef PIN_SLIDER_SOCD_SECOND
#define PIN_SLIDER_SOCD_SECOND    -1
#endif

// Slider Module Name
#define SliderSOCDName "SliderSOCD"

class SliderSOCDInput : public GPAddon {
public:
    virtual bool available();
	virtual void setup();       // SliderSOCD Button Setup
    virtual void preprocess() {}
	virtual void process();     // SliderSOCD process
    virtual std::string name() { return SliderSOCDName; }
private:
    SOCDMode read();
    void debounce();
    SOCDMode socdState;           // Saved locally for debounce
    SOCDMode dDebState;          // Debounce SliderSOCD State
    uint32_t uDebTime;          // Debounce SliderSOCD Time
    uint8_t pinSliderSOCDUp;
    uint8_t pinSliderSOCDSecond;
};

#endif  // _SliderSOCD_H_