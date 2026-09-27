#ifndef _Analog_H
#define _Analog_H

#include "gpaddon.h"
#include "GamepadEnums.h"
#include "BoardConfig.h"
#include "enums.pb.h"
#include "config.pb.h"
#include "helper.h"
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

// 74HC4051 analog multiplexer: select pins S0-S2 and the ADC pin wired to Z
#ifndef ANALOG_MUX_S0_PIN
#define ANALOG_MUX_S0_PIN -1
#endif

#ifndef ANALOG_MUX_S1_PIN
#define ANALOG_MUX_S1_PIN -1
#endif

#ifndef ANALOG_MUX_S2_PIN
#define ANALOG_MUX_S2_PIN -1
#endif

#ifndef ANALOG_MUX_Z_PIN
#define ANALOG_MUX_Z_PIN -1
#endif

// Analog triggers (pin can be an ADC pin or a mux channel), raw 12-bit ADC range.
// Defaults suit an SS49E on 3.3V (~1.6V released, ~2.2V fully pressed).
#ifndef ANALOG_TRIGGER_L_PIN
#define ANALOG_TRIGGER_L_PIN -1
#endif

#ifndef ANALOG_TRIGGER_R_PIN
#define ANALOG_TRIGGER_R_PIN -1
#endif

#ifndef ANALOG_TRIGGER_L_MIN
#define ANALOG_TRIGGER_L_MIN 1985
#endif

#ifndef ANALOG_TRIGGER_L_MAX
#define ANALOG_TRIGGER_L_MAX 2730
#endif

#ifndef ANALOG_TRIGGER_R_MIN
#define ANALOG_TRIGGER_R_MIN 1985
#endif

#ifndef ANALOG_TRIGGER_R_MAX
#define ANALOG_TRIGGER_R_MAX 2730
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
#define SMOOTHING_FACTOR 5
#endif

#ifndef SMOOTHING_FACTOR2
#define SMOOTHING_FACTOR2 5
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

// Mux channels Y0-Y7 are stored in the pin fields as ANALOG_MUX_PIN_BASE + channel.
// They sit above every real GPIO number, so isValidPin() rejects them on purpose.
#define ANALOG_MUX_PIN_BASE 100
#define ANALOG_MUX_CHANNELS 8

// Wait after switching the mux, throw away one sample, then average several (see readRaw)
#define ANALOG_MUX_SETTLE_US 10
#define ANALOG_MUX_SAMPLES 8

// Trigger filtering: EMA weight of the newest sample, and idle dead zone (fraction of range)
#define ANALOG_TRIGGER_EMA 0.3f
#define ANALOG_TRIGGER_DEADZONE 0.02f

static inline bool isAnalogMuxPin(int32_t pin) {
    return pin >= ANALOG_MUX_PIN_BASE && pin < (ANALOG_MUX_PIN_BASE + ANALOG_MUX_CHANNELS);
}

static inline bool isAnalogPinUsable(int32_t pin) {
    return isAnalogMuxPin(pin) || isValidPin(pin);
}

typedef struct
{
    Pin_t x_pin;
    Pin_t y_pin;
    Pin_t x_pin_adc;    // ADC input index, or mux channel when x_mux is set
    Pin_t y_pin_adc;    // ADC input index, or mux channel when y_mux is set
    bool x_mux;
    bool y_mux;
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

typedef struct
{
    Pin_t pin;
    Pin_t pin_adc;      // ADC input index, or mux channel when mux is set
    bool mux;
    bool primed;
    float ema;
    float min;
    float max;
} trigger_instance;

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
    void muxSetup(const AnalogOptions& analogOptions);
    void muxSelect(uint8_t channel);
    uint16_t readRaw(bool mux, Pin_t pin_adc);
    uint8_t readTrigger(trigger_instance & trigger);
    float readPin(int stick_num, Pin_t pin_adc, bool mux, uint16_t center);
    float emaCalculation(int stick_num, float ema_value, float ema_previous);
    uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
    float magnitudeCalculation(int stick_num, adc_instance & adc_inst);
    void radialDeadzone(int stick_num, adc_instance & adc_inst);
    adc_instance adc_pairs[ADC_COUNT];
    trigger_instance triggers[2];   // 0 = left trigger, 1 = right trigger
    bool muxReady;
    Pin_t muxSelectPins[3];
    Pin_t muxZPin;
    int8_t muxChannel;
};

#endif  // _Analog_H_
