#ifndef _Analog_H
#define _Analog_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#include "BoardConfig.h"

#ifndef ANALOG_INPUT_ENABLED
#define ANALOG_INPUT_ENABLED 0
#endif

#ifndef ANALOG_ADC_VRX
#define ANALOG_ADC_VRX    -1
#endif

#ifndef ANALOG_ADC_VRY
#define ANALOG_ADC_VRY    -1
#endif

// Analog Module Name
#define AnalogName "Analog"

class AnalogInput : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // Analog Setup
	virtual void process();     // Analog Process
	virtual void preprocess() {}
    virtual std::string name() { return AnalogName; }
private:
	uint8_t analogAdcPinX;
	uint8_t analogAdcPinY;
};

#endif  // _Analog_H_