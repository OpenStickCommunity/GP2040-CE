#ifndef _SliderInputMode_H
#define _SliderInputMode_H

#include "gpaddon.h"

#ifndef SLIDER_INPUT_MODE_ENABLED
#define SLIDER_INPUT_MODE_ENABLED   0
#endif

#ifndef PIN_SLIDER_INPUT_MODE_ONE
#define PIN_SLIDER_INPUT_MODE_ONE  -1
#endif

#ifndef PIN_SLIDER_INPUT_MODE_TWO
#define PIN_SLIDER_INPUT_MODE_TWO  -1
#endif

#ifndef SLIDER_INPUT_MODE_SLOT_ONE
#define SLIDER_INPUT_MODE_SLOT_ONE INPUT_MODE_SWITCH
#endif

#ifndef SLIDER_INPUT_MODE_SLOT_TWO
#define SLIDER_INPUT_MODE_SLOT_TWO INPUT_MODE_HID
#endif

// Slider Module Name
#define SliderInputModeName "SliderInputMode"

class SliderInputMode : public GPAddon {
public:
  virtual bool available();
	virtual void setup();       					// Pin Setup
  virtual void preprocess() {}
	virtual void process();
  virtual std::string name() { return SliderInputModeName; }
private:
		InputMode read();
		void debounce();
		InputMode simState;           			// for debouncing
		InputMode dDebState;          			// Debounce Slider State
		uint32_t uDebTime;          				// Debounce Slider Time
		InputMode sliderInputModeOne;
		InputMode sliderInputModeTwo;
		InputMode sliderInputModeDefault;
		uint8_t pinSliderInputModeOne;
		uint8_t pinSliderInputModeTwo;
};

#endif // _SliderInputMode_H
