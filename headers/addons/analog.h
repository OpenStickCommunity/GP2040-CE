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

#ifndef FORCED_CIRCULARITY2_ENABLED
#define FORCED_CIRCULARITY2_ENABLED 0
#endif

#ifndef DEFAULT_INNER_DEADZONE
#define DEFAULT_INNER_DEADZONE 5
#endif

#ifndef DEFAULT_INNER_DEADZONE2
#define DEFAULT_INNER_DEADZONE2 5
#endif

#ifndef DEFAULT_OUTER_DEADZONE
#define DEFAULT_OUTER_DEADZONE 95
#endif

#ifndef DEFAULT_OUTER_DEADZONE2
#define DEFAULT_OUTER_DEADZONE2 95
#endif

#ifndef AUTO_CALIBRATE_ENABLED
#define AUTO_CALIBRATE_ENABLED 0
#endif

#ifndef AUTO_CALIBRATE2_ENABLED
#define AUTO_CALIBRATE2_ENABLED 0
#endif

#ifndef ANALOG_SMOOTHING_ENABLED
#define ANALOG_SMOOTHING_ENABLED 0
#endif

#ifndef ANALOG_SMOOTHING2_ENABLED
#define ANALOG_SMOOTHING2_ENABLED 0
#endif

#ifndef SMOOTHING_FACTOR
#define SMOOTHING_FACTOR 2
#endif

#ifndef SMOOTHING_FACTOR2
#define SMOOTHING_FACTOR2 2
#endif

#ifndef ANALOG_ERROR
#define ANALOG_ERROR 1000
#endif

#ifndef ANALOG_ERROR2
#define ANALOG_ERROR2 1000
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
    uint32_t x_center;
    uint32_t y_center;
    uint32_t x_min;
    uint32_t x_max;
    uint32_t y_min;
    uint32_t y_max;
    float xy_magnitude;
    float x_magnitude;
    float y_magnitude;
    InvertMode analog_invert;
    DpadMode analog_dpad;
    float x_ema;
    float y_ema;
    bool x_ema_initialized;
    bool y_ema_initialized;
    bool ema_option;
    float ema_smoothing;
    float error_rate;
    float in_deadzone;
    float out_deadzone;
    bool auto_calibration;
    bool forced_circularity;
    uint32_t joystick_center_x;
    uint32_t joystick_center_y;
} adc_instance;

class AnalogInput : public GPAddon {
public:
    static bool isAdcPin(Pin_t pin);
    static uint16_t readCalibrationSample(Pin_t pin);
    virtual bool available();
    virtual void setup();       // Analog Setup
    virtual void process();     // Analog Process
    virtual void preprocess() {}
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return AnalogName; }
private:
    float readPin(Pin_t pin, uint32_t center, uint32_t minimum, uint32_t maximum);
    float emaCalculation(int stick_num, float ema_value, float ema_previous);
    float magnitudeCalculation(int stick_num, adc_instance & adc_inst);
    void radialDeadzone(int stick_num, adc_instance & adc_inst);
    adc_instance adc_pairs[ADC_COUNT];
};

#endif  // _Analog_H_
