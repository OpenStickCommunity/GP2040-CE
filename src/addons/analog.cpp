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

    struct pin_center_pair
    {
        int pin;
        std::uint16_t& center;
    };

    std::array<pin_center_pair, 4> pins =
    {{
        {analogOptions.analogAdc1PinX, adc_1_x_center},
        {analogOptions.analogAdc1PinY, adc_1_y_center},
        {analogOptions.analogAdc2PinX, adc_2_x_center},
        {analogOptions.analogAdc2PinY, adc_2_y_center}
    }};
    
    for(auto& [pin, center]: pins)
    {
        if(isValidPin(pin)) {
            adc_gpio_init(pin);
            if (analogOptions.auto_calibrate) {
                adc_select_input(pin - ADC_PIN_OFFSET);
                center = adc_read();
            }
        }
    }
}

void AnalogInput::process()
{
    const AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
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
        int adc_x_pin;
        int adc_y_pin;
        float& adc_x_value;
        float& adc_y_value;
        std::uint16_t& x_center;
        std::uint16_t& y_center;
        float& adc_magnitude;
        float& adc_x_magnitude;
        float& adc_y_magnitude;
        InvertMode analog_invert;
        DpadMode analog_dpad;
    };

    std::array<adc_pair, 2> adc_pairs =
    {{
        {analogOptions.analogAdc1PinX, analogOptions.analogAdc1PinY, adc_1_x, adc_1_y, 
        adc_1_x_center, adc_1_y_center, magnitude_1, x_magnitude_1, y_magnitude_1, 
        analogOptions.analogAdc1Invert, analogOptions.analogAdc1Mode},
        {analogOptions.analogAdc2PinX, analogOptions.analogAdc2PinY, adc_2_x, adc_2_y, 
        adc_2_x_center, adc_2_y_center, magnitude_2, x_magnitude_2, y_magnitude_2, 
        analogOptions.analogAdc2Invert, analogOptions.analogAdc2Mode}
    }};

    for(auto& [x_pin, y_pin, x_value, y_value, x_center, y_center, 
        xy_magnitude, x_magnitude, y_magnitude, invert, dpad]: adc_pairs) {
        
        if (isValidPin(x_pin)) {
            adc_select_input(x_pin - ADC_PIN_OFFSET);
            x_value = readPin(x_pin, x_center, analogOptions.auto_calibrate);

            if (invert == InvertMode::INVERT_X || invert == InvertMode::INVERT_XY) {
                x_value = ANALOG_MAX - x_value;
            }
        }
        if (isValidPin(y_pin)) {
            adc_select_input(y_pin - ADC_PIN_OFFSET);
            y_value = readPin(y_pin, y_center, analogOptions.auto_calibrate);

            if (invert == InvertMode::INVERT_Y || invert == InvertMode::INVERT_XY) {
                y_value = ANALOG_MAX - y_value;
            }
        }

        if (adc_deadzone >= 0.0f || analogOptions.forced_circularity == true) {
            xy_magnitude = magnitudeCalculation(x_value, y_value, x_magnitude, y_magnitude);

            if (adc_deadzone >= 0.0f) {
                if (xy_magnitude < adc_deadzone) {
                    x_value = ANALOG_CENTER;
                    y_value = ANALOG_CENTER;
                }
                else {
                    radialDeadzone(x_value, y_value, adc_deadzone, x_magnitude, y_magnitude, xy_magnitude);
                }
            }
            if (x_value != ANALOG_CENTER && y_value != ANALOG_CENTER && 
                analogOptions.forced_circularity == true && xy_magnitude > ANALOG_CENTER) {

                adjustCircularity(x_value, y_value, x_magnitude, y_magnitude, xy_magnitude);
            }
        }

        if (dpad == DpadMode::DPAD_MODE_LEFT_ANALOG) {
            gamepad->state.lx = static_cast<uint16_t>(65535.0f * x_value);
            gamepad->state.ly = static_cast<uint16_t>(65535.0f * y_value);
        }
        else if (dpad == DpadMode::DPAD_MODE_RIGHT_ANALOG) {
            gamepad->state.rx = static_cast<uint16_t>(65535.0f * x_value);
            gamepad->state.ry = static_cast<uint16_t>(65535.0f * y_value);
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
    return sqrt((x_magnitude * x_magnitude) + (y_magnitude * y_magnitude));
}

void AnalogInput::radialDeadzone(float& x, float& y, float deadzone, float x_magnitude, float y_magnitude, float magnitude) {
    float scaling_factor = (magnitude - deadzone) / (1.0f - 1.6f * deadzone);
    
    x = ((x_magnitude / magnitude) * scaling_factor) + ANALOG_CENTER;
    y = ((y_magnitude / magnitude) * scaling_factor) + ANALOG_CENTER;

    x = std::fmin(x, 1.0f);
    y = std::fmin(y, 1.0f);
}

void AnalogInput::adjustCircularity(float& x, float& y, float x_magnitude, float y_magnitude, float magnitude) {
    x = ((x_magnitude / magnitude) * ANALOG_CENTER + ANALOG_CENTER);
    y = ((y_magnitude / magnitude) * ANALOG_CENTER + ANALOG_CENTER);
}