#include "addons/analog.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

#include "enums.pb.h"

#include "hardware/adc.h"

#include <math.h>

#define ADC_MAX ((1 << 12) - 1)
#define ANALOG_CENTER 0.5f // 0.5f is center
#define ANALOG_MAX 1.0f    // 1.0f is max

bool AnalogInput::available() {
    return Storage::getInstance().getAddonOptions().analogOptions.enabled;
}

void AnalogInput::setup() {
    const AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;

    // Make sure GPIO is high-impedance, no pullups etc
    if ( isValidPin(analogOptions.analogAdc1PinX) ) {
        adc_gpio_init(analogOptions.analogAdc1PinX);
    }
    if ( isValidPin(analogOptions.analogAdc1PinY) ) {
        adc_gpio_init(analogOptions.analogAdc1PinY);
    }
    if ( isValidPin(analogOptions.analogAdc2PinX) ) {
        adc_gpio_init(analogOptions.analogAdc2PinX);
    }
    if ( isValidPin(analogOptions.analogAdc2PinY) ) {
        adc_gpio_init(analogOptions.analogAdc2PinY);
    }
	
	// Read pins deviation from center for calibration
	if (analogOptions.auto_calibrate) {
		if ( isValidPin(analogOptions.analogAdc1PinX) ) {
			adc_select_input(analogOptions.analogAdc1PinX-26); // ANALOG1-X
			adc_1_x_center = adc_read();
		}
		if ( isValidPin(analogOptions.analogAdc1PinY) ) {
			adc_select_input(analogOptions.analogAdc1PinY-26); // ANALOG1-Y
			adc_1_y_center = adc_read();
		}
		if ( isValidPin(analogOptions.analogAdc2PinX) ) {
			adc_select_input(analogOptions.analogAdc2PinX-26); // ANALOG2-X
			adc_2_x_center = adc_read();
		}
		if ( isValidPin(analogOptions.analogAdc2PinY) ) {
			adc_select_input(analogOptions.analogAdc2PinY-26); // ANALOG2-Y
			adc_2_y_center = adc_read();
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

    if ( isValidPin(analogOptions.analogAdc1PinX) ) {
        adc_1_x = readPin(analogOptions.analogAdc1PinX, adc_1_x_center, analogOptions.auto_calibrate, adc_deadzone);
        
        if ( analogOptions.analogAdc1Invert == InvertMode::INVERT_X ||
            analogOptions.analogAdc1Invert == InvertMode::INVERT_XY) {
            adc_1_x = ANALOG_MAX - adc_1_x;
        }
    }
    if ( isValidPin(analogOptions.analogAdc1PinY) ) {
        adc_1_y = readPin(analogOptions.analogAdc1PinY, adc_1_y_center, analogOptions.auto_calibrate, adc_deadzone);
        
        if ( analogOptions.analogAdc1Invert == InvertMode::INVERT_Y ||
            analogOptions.analogAdc1Invert == InvertMode::INVERT_XY) {
            adc_1_y = ANALOG_MAX - adc_1_y;
        }
    }
    if ( isValidPin(analogOptions.analogAdc2PinX) ) {
        adc_2_x = readPin(analogOptions.analogAdc2PinX, adc_2_x_center, analogOptions.auto_calibrate, adc_deadzone);
        
        if ( analogOptions.analogAdc2Invert == InvertMode::INVERT_X ||
            analogOptions.analogAdc2Invert == InvertMode::INVERT_XY) {
            adc_2_x = ANALOG_MAX - adc_2_x;
        }
    }
    if ( isValidPin(analogOptions.analogAdc2PinY) ) {
        adc_2_y = readPin(analogOptions.analogAdc2PinY, adc_2_y_center, analogOptions.auto_calibrate, adc_deadzone);

        if ( analogOptions.analogAdc2Invert == InvertMode::INVERT_Y ||
            analogOptions.analogAdc2Invert == InvertMode::INVERT_XY) {
            adc_2_y = ANALOG_MAX - adc_2_y;
        }
    }
    
    // Alter coordinates to force perfect circularity
    if (analogOptions.forced_circularity) {
        adjustCircularity(adc_1_x, adc_1_y);
        adjustCircularity(adc_2_x, adc_2_y);
    }

    // Convert to 16-bit value
    if ( analogOptions.analogAdc1Mode == DpadMode::DPAD_MODE_LEFT_ANALOG) {
        gamepad->state.lx = (uint16_t)(65535.0f*adc_1_x);
        gamepad->state.ly = (uint16_t)(65535.0f*adc_1_y);
    } else if ( analogOptions.analogAdc1Mode == DpadMode::DPAD_MODE_RIGHT_ANALOG) {
        gamepad->state.rx = (uint16_t)(65535.0f*adc_1_x);
        gamepad->state.ry = (uint16_t)(65535.0f*adc_1_y);
    }
    if ( analogOptions.analogAdc2Mode == DpadMode::DPAD_MODE_LEFT_ANALOG) {
        gamepad->state.lx = (uint16_t)(65535.0f*adc_2_x);
        gamepad->state.ly = (uint16_t)(65535.0f*adc_2_y);
    } else if ( analogOptions.analogAdc2Mode == DpadMode::DPAD_MODE_RIGHT_ANALOG) {
        gamepad->state.rx = (uint16_t)(65535.0f*adc_2_x);
        gamepad->state.ry = (uint16_t)(65535.0f*adc_2_y);
    }
}

float AnalogInput::readPin(int pin, uint16_t center, bool autoCalibrate, float deadzone) {
	adc_select_input(pin - 26);
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

	float adc_value = ((float)adc_calibrated) / ADC_MAX;

	if (abs(adc_value - ANALOG_CENTER) < deadzone) { // deadzones
		adc_value = ANALOG_CENTER;
	}

	return adc_value;
}

uint16_t AnalogInput::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void AnalogInput::adjustCircularity(float& x, float& y) {
    float x_magnitude = x - ANALOG_CENTER;
    float y_magnitude = y - ANALOG_CENTER;
    float magnitude = sqrt((x_magnitude * x_magnitude) + (y_magnitude * y_magnitude));

    if (magnitude > ANALOG_CENTER) {
        x = ((x_magnitude / magnitude) * ANALOG_CENTER + ANALOG_CENTER);
        y = ((y_magnitude / magnitude) * ANALOG_CENTER + ANALOG_CENTER);
    }
}