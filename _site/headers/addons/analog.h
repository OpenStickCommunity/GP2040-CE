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

#ifndef AUTO_CALIBRATE_ENABLED
#define AUTO_CALIBRATE_ENABLED 0
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
	uint16_t adc_1_x_center = 0;
	uint16_t adc_1_y_center = 0;
	uint16_t adc_2_x_center = 0;
	uint16_t adc_2_y_center = 0;

	static float readPin(int pin, uint16_t center, bool autoCalibrate);
	static uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
	void radialDeadzone(float& x, float& y, float deadzone, float x_magnitude, float y_magnitude, float magnitude);
	void adjustCircularity(float& x, float& y, float deadzone, float x_magnitude, float y_magnitude, float magnitude);
};

#endif  // _Analog_H_