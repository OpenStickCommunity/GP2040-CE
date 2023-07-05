#ifndef _Analog_H
#define _Analog_H

#include "gpaddon.h"

#include "GamepadEnums.h"

#include "BoardConfig.h"

#include "enums.pb.h"

#ifndef ANALOG_INPUT_ENABLED
#define ANALOG_INPUT_ENABLED 0
#endif

#ifndef ANALOG_ADC_1_VRX
#define ANALOG_ADC_1_VRX    -1
#endif

#ifndef ANALOG_ADC_1_VRY
#define ANALOG_ADC_1_VRY    -1
#endif

#ifndef ANALOG_ADC_1_MODE
#define ANALOG_ADC_1_MODE DPAD_MODE_LEFT_ANALOG
#endif

#ifndef ANALOG_ADC_1_INVERT
#define ANALOG_ADC_1_INVERT INVERT_NONE
#endif

#ifndef ANALOG_ADC_2_VRX
#define ANALOG_ADC_2_VRX    -1
#endif

#ifndef ANALOG_ADC_2_VRY
#define ANALOG_ADC_2_VRY    -1
#endif

#ifndef ANALOG_ADC_2_MODE
#define ANALOG_ADC_2_MODE DPAD_MODE_RIGHT_ANALOG
#endif

#ifndef ANALOG_ADC_2_INVERT
#define ANALOG_ADC_2_INVERT INVERT_NONE
#endif

#ifndef FORCED_CIRCULARITY_ENABLED
#define FORCED_CIRCULARITY_ENABLED 0
#endif

#ifndef DEFAULT_ANALOG_DEADZONE
#define DEFAULT_ANALOG_DEADZONE 5
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
};

#endif  // _Analog_H_