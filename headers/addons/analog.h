#ifndef _Analog_H
#define _Analog_H

#include "gpaddon.h"
#include "GamepadEnums.h"
#include "BoardConfig.h"
#include "enums.pb.h"
#include "types.h"

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

#ifndef DEFAULT_INNER_DEADZONE
#define DEFAULT_INNER_DEADZONE 5
#endif

#ifndef DEFAULT_OUTER_DEADZONE
#define DEFAULT_OUTER_DEADZONE 95
#endif

#ifndef AUTO_CALIBRATE_ENABLED
#define AUTO_CALIBRATE_ENABLED 0
#endif

#ifndef ANALOG_SMOOTHING_ENABLED
#define ANALOG_SMOOTHING_ENABLED 0
#endif

#ifndef SMOOTHING_FACTOR
#define SMOOTHING_FACTOR 5
#endif

#ifndef ANALOG_ERROR
#define ANALOG_ERROR 1000
#endif

// Analog Module Name
#define AnalogName "Analog"

#define ADC_COUNT 2

typedef struct
{
    Pin_t x_pin;
    Pin_t y_pin;
    Pin_t x_pin_adc;
    Pin_t y_pin_adc;
    float x_value;
    float y_value;
    uint16_t x_center;
    uint16_t y_center;
    float xy_magnitude;
    float x_magnitude;
    float y_magnitude;
    InvertMode analog_invert;
    DpadMode analog_dpad;
    float x_ema;
    float y_ema;
} adc_instance;

class AnalogInput : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // Analog Setup
    virtual void process();     // Analog Process
    virtual void preprocess() {}
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return AnalogName; }
private:
    float readPin(Pin_t pin, uint16_t center);
    float emaCalculation(float ema_value, float ema_previous);
    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
    float magnitudeCalculation(adc_instance & adc_inst);
    void radialDeadzone(adc_instance & adc_inst);
    adc_instance adc_pairs[ADC_COUNT];
    bool ema_option;
    float ema_smoothing;
    float error_rate;
    float in_deadzone;
    float out_deadzone;
    bool auto_calibration;
    bool forced_circularity;
};

#endif  // _Analog_H_