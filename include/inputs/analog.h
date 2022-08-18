#ifndef _Analog_H
#define _Analog_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef ANALOG_ADC_VRX
#define ANALOG_ADC_VRX    -1
#endif

#ifndef ANALOG_ADC_VRY
#define ANALOG_ADC_VRX    -1
#endif

// Analog Module Name
#define AnalogName "Analog"

class AnalogInput : public GPAddon {
public:
	virtual bool available();   // GPAddon available
	virtual void setup();       // Analog Setup
	virtual void process();     // Analog Process
    virtual std::string name() { return AnalogName; }
private:
};

#endif  // _Analog_H_