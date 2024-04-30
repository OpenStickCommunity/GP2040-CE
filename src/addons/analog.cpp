#include "addons/analog.h"
#include "config.pb.h"
#include "enums.pb.h"
#include "hardware/adc.h"
#include "helper.h"
#include "storagemanager.h"

#include <math.h>

#define ADC_MAX ((1 << 12) - 1) // 4095
#define ADC_PIN_OFFSET 26
#define ANALOG_MAX 1.0f
#define ANALOG_CENTER ANALOG_MAX / 2

bool AnalogInput::available() {
    return Storage::getInstance().getAddonOptions().analogOptions.enabled;
}

void AnalogInput::setup() {
    const AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
    const size_t num_adc_pins = 4;

    struct pin_center_pair
    {
        int pin;
        uint16_t& center;
    };

    pin_center_pair adc_pins[num_adc_pins] =
    {
        {analogOptions.analogAdc1PinX, adc_1_x_center},
        {analogOptions.analogAdc1PinY, adc_1_y_center},
        {analogOptions.analogAdc2PinX, adc_2_x_center},
        {analogOptions.analogAdc2PinY, adc_2_y_center}
    };
    
    for(size_t i = 0; i < num_adc_pins; i++) {
        if(isValidPin(adc_pins[i].pin)) {
            adc_gpio_init(adc_pins[i].pin);
            if (analogOptions.auto_calibrate) {
                adc_select_input(adc_pins[i].pin - ADC_PIN_OFFSET);
                adc_pins[i].center = adc_read();
            }
        }
    }
}

void AnalogInput::process()
{
    const AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
    const size_t num_adc_pairs = 2;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    float adc_1_x = ANALOG_CENTER;
    float adc_1_y = ANALOG_CENTER;
    float adc_2_x = ANALOG_CENTER;
    float adc_2_y = ANALOG_CENTER;
    float adc_deadzone = analogOptions.analog_deadzone / 200.0f;
    float x_magnitude_1 = 0.0f;
    float y_magnitude_1 = 0.0f;
    float x_magnitude_2 = 0.0f;
    float y_magnitude_2 = 0.0f;
    float magnitude_1 = 0.0f;
    float magnitude_2 = 0.0f;

    struct adc_pair
    {
        int x_pin;
        int y_pin;
        float& x_value;
        float& y_value;
        uint16_t& x_center;
        uint16_t& y_center;
        float& xy_magnitude;
        float& x_magnitude;
        float& y_magnitude;
        InvertMode analog_invert;
        DpadMode analog_dpad;
    };

    adc_pair adc_pairs[num_adc_pairs] =
    {
        {analogOptions.analogAdc1PinX, analogOptions.analogAdc1PinY, 
        adc_1_x, adc_1_y, 
        adc_1_x_center, adc_1_y_center, 
        magnitude_1, x_magnitude_1, y_magnitude_1, 
        analogOptions.analogAdc1Invert, 
        analogOptions.analogAdc1Mode},

        {analogOptions.analogAdc2PinX, analogOptions.analogAdc2PinY, 
        adc_2_x, adc_2_y, 
        adc_2_x_center, adc_2_y_center, 
        magnitude_2, x_magnitude_2, y_magnitude_2, 
        analogOptions.analogAdc2Invert, 
        analogOptions.analogAdc2Mode}
    };

    for(size_t i = 0; i < num_adc_pairs; i++) {
        if (isValidPin(adc_pairs[i].x_pin)) {
            adc_select_input(adc_pairs[i].x_pin - ADC_PIN_OFFSET);
            adc_pairs[i].x_value = readPin(adc_pairs[i].x_pin, adc_pairs[i].x_center, analogOptions.auto_calibrate);

            if (adc_pairs[i].analog_invert == InvertMode::INVERT_X || 
                adc_pairs[i].analog_invert == InvertMode::INVERT_XY) {
                
                adc_pairs[i].x_value = ANALOG_MAX - adc_pairs[i].x_value;
            }
        }
        if (isValidPin(adc_pairs[i].y_pin)) {
            adc_select_input(adc_pairs[i].y_pin - ADC_PIN_OFFSET);
            adc_pairs[i].y_value = readPin(adc_pairs[i].y_pin, adc_pairs[i].y_center, analogOptions.auto_calibrate);

            if (adc_pairs[i].analog_invert == InvertMode::INVERT_Y || 
                adc_pairs[i].analog_invert == InvertMode::INVERT_XY) {
                
                adc_pairs[i].y_value = ANALOG_MAX - adc_pairs[i].y_value;
            }
        }

        if (adc_deadzone >= 0.0f || analogOptions.forced_circularity == true) {
            adc_pairs[i].xy_magnitude = magnitudeCalculation(adc_pairs[i].x_value, adc_pairs[i].y_value, 
                                                            adc_pairs[i].x_magnitude, adc_pairs[i].y_magnitude);

            if (adc_deadzone >= 0.0f) {
                if (adc_pairs[i].xy_magnitude < adc_deadzone) {
                    adc_pairs[i].x_value = ANALOG_CENTER;
                    adc_pairs[i].y_value = ANALOG_CENTER;
                }
                else {
                    radialDeadzone(adc_pairs[i].x_value, adc_pairs[i].y_value, adc_deadzone, 
                                    adc_pairs[i].x_magnitude, adc_pairs[i].y_magnitude, adc_pairs[i].xy_magnitude);
                }
            }
            if (adc_pairs[i].x_value != ANALOG_CENTER && adc_pairs[i].y_value != ANALOG_CENTER && 
                analogOptions.forced_circularity == true && adc_pairs[i].xy_magnitude > ANALOG_CENTER) {

                adjustCircularity(adc_pairs[i].x_value, adc_pairs[i].y_value, 
                                adc_pairs[i].x_magnitude, adc_pairs[i].y_magnitude, adc_pairs[i].xy_magnitude);
            }
        }

        if (adc_pairs[i].analog_dpad == DpadMode::DPAD_MODE_LEFT_ANALOG) {
            gamepad->state.lx = static_cast<uint16_t>(65535.0f * adc_pairs[i].x_value);
            gamepad->state.ly = static_cast<uint16_t>(65535.0f * adc_pairs[i].y_value);
        }
        else if (adc_pairs[i].analog_dpad == DpadMode::DPAD_MODE_RIGHT_ANALOG) {
            gamepad->state.rx = static_cast<uint16_t>(65535.0f * adc_pairs[i].x_value);
            gamepad->state.ry = static_cast<uint16_t>(65535.0f * adc_pairs[i].y_value);
        }
    }
}

float AnalogInput::readPin(int pin, uint16_t center, bool autoCalibrate) {
	adc_select_input(pin - ADC_PIN_OFFSET);
	uint16_t adc_hold = adc_read();

	// Calibrate axis based on off-center
	uint16_t adc_calibrated;

	if (autoCalibrate) {
		if (adc_hold > center) {
			adc_calibrated = map(adc_hold, center, ADC_MAX, ADC_MAX / 2, ADC_MAX);
		}
		else if (adc_hold == center) {
			adc_calibrated = ADC_MAX / 2;
		}
		else {
			adc_calibrated = map(adc_hold, 0, center, 0, ADC_MAX / 2);
		}
	}
	else {
		adc_calibrated = adc_hold;
	}

	return ((float)adc_calibrated) / ADC_MAX;
}

uint16_t AnalogInput::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

float AnalogInput::magnitudeCalculation(float x, float y, float& x_magnitude, float& y_magnitude) {
    x_magnitude = x - ANALOG_CENTER;
    y_magnitude = y - ANALOG_CENTER;
    
    return std::sqrt((x_magnitude * x_magnitude) + (y_magnitude * y_magnitude));
}

void AnalogInput::radialDeadzone(float& x, float& y, float deadzone, float x_magnitude, float y_magnitude, float xy_magnitude) {
    float scaling_factor = (xy_magnitude - deadzone) / (1.0f - 1.6f * deadzone);
    
    x = ((x_magnitude / xy_magnitude) * scaling_factor) + ANALOG_CENTER;
    y = ((y_magnitude / xy_magnitude) * scaling_factor) + ANALOG_CENTER;

    x = std::fmin(x, 1.0f);
    y = std::fmin(y, 1.0f);
}

void AnalogInput::adjustCircularity(float& x, float& y, float x_magnitude, float y_magnitude, float xy_magnitude) {
    x = ((x_magnitude / xy_magnitude) * ANALOG_CENTER + ANALOG_CENTER);
    y = ((y_magnitude / xy_magnitude) * ANALOG_CENTER + ANALOG_CENTER);
}