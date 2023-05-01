#ifndef _SliderSOCD_H
#define _SliderSOCD_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef SLIDER_SOCD_ENABLED
#define SLIDER_SOCD_ENABLED 0
#endif

#ifndef PIN_SLIDER_SOCD_ONE
#define PIN_SLIDER_SOCD_ONE    -1
#endif

#ifndef PIN_SLIDER_SOCD_TWO
#define PIN_SLIDER_SOCD_TWO    -1
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
    virtual void preprocess() {}
	virtual void process();     // SliderSOCD process
    virtual std::string name() { return SliderSOCDName; }
private:
    SOCDMode read();
    void debounce();
    SOCDMode socdState;           // Saved locally for debounce
    SOCDMode dDebState;          // Debounce SliderSOCD State
    uint32_t uDebTime;          // Debounce SliderSOCD Time
    SOCDMode sliderSOCDModeOne;
    SOCDMode sliderSOCDModeTwo;
    SOCDMode sliderSOCDModeDefault;
    uint8_t pinSliderSOCDOne;
    uint8_t pinSliderSOCDTwo;
};

#endif  // _SliderSOCD_H_