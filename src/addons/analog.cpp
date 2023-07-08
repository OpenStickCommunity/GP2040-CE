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

uint16_t adc_1_x_center = 0;
uint16_t adc_1_y_center = 0;
uint16_t adc_2_x_center = 0;
uint16_t adc_2_y_center = 0;

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
    float deadzone = analogOptions.analog_deadzone / 200.0f;

    if ( isValidPin(analogOptions.analogAdc1PinX) ) {
        adc_select_input(analogOptions.analogAdc1PinX-26); // ANALOG1-X

		if (analogOptions.auto_calibrate) {
			uint16_t adc_1_x_hold = adc_read();

			// Calibrate axis based on off-center
			uint16_t adc_1_x_calibrated;

			if (adc_1_x_hold > adc_1_x_center) {
				adc_1_x_calibrated = map(adc_1_x_hold, adc_1_x_center, ADC_MAX, ADC_MAX/2, ADC_MAX);
			}
			else if (adc_1_x_hold == adc_1_x_center) {
				adc_1_x_calibrated = ADC_MAX/2;
			}
			else{
				adc_1_x_calibrated = map(adc_1_x_hold, 0, adc_1_x_center, 0, ADC_MAX/2);
			}
			
			adc_1_x = ((float)adc_1_x_calibrated)/ADC_MAX;
		}
		else {
			adc_1_x = ((float)adc_read())/ADC_MAX;
		}
        
        if ( abs(adc_1_x - ANALOG_CENTER) < deadzone ) { // deadzones
            adc_1_x = ANALOG_CENTER;
        } else if ( analogOptions.analogAdc1Invert == InvertMode::INVERT_X ||
            analogOptions.analogAdc1Invert == InvertMode::INVERT_XY) {
            adc_1_x = ANALOG_MAX - adc_1_x;
        }
    }
    if ( isValidPin(analogOptions.analogAdc1PinY) ) {
        adc_select_input(analogOptions.analogAdc1PinY-26); // ANALOG1-Y
        
		if (analogOptions.auto_calibrate) {
			uint16_t adc_1_y_hold = adc_read();

			// Calibrate axis based on off-center
			uint16_t adc_1_y_calibrated;
			if (adc_1_y_hold > adc_1_y_center) {
				adc_1_y_calibrated = map(adc_1_y_hold, adc_1_y_center, ADC_MAX, ADC_MAX/2, ADC_MAX);
			}
			else if (adc_1_y_hold == adc_1_y_center) {
				adc_1_y_calibrated = ADC_MAX/2;
			}
			else{
				adc_1_y_calibrated = map(adc_1_y_hold, 0, adc_1_y_center, 0, ADC_MAX/2);
			}
			
			adc_1_y = ((float)adc_1_y_calibrated)/ADC_MAX;
		}
		else {
			adc_1_y = ((float)adc_read())/ADC_MAX;
		}
        
        if ( abs(adc_1_y - ANALOG_CENTER) < deadzone ) { // deadzones
            adc_1_y = ANALOG_CENTER;
        } else if ( analogOptions.analogAdc1Invert == InvertMode::INVERT_Y ||
            analogOptions.analogAdc1Invert == InvertMode::INVERT_XY) {
            adc_1_y = ANALOG_MAX - adc_1_y;
        }
    }
    if ( isValidPin(analogOptions.analogAdc2PinX) ) {
        adc_select_input(analogOptions.analogAdc2PinX-26); // ANALOG2-X
        
		if (analogOptions.auto_calibrate) {
			uint16_t adc_2_x_hold = adc_read();

			// Calibrate axis based on off-center
			uint16_t adc_2_x_calibrated;
			if (adc_2_x_hold >= adc_2_x_center) {
				adc_2_x_calibrated = map(adc_2_x_hold, adc_2_x_center, ADC_MAX, ADC_MAX/2, ADC_MAX);
			}
			else{
				adc_2_x_calibrated = map(adc_2_x_hold, 0, adc_2_x_center, 0, ADC_MAX/2);
			}
			
			adc_2_x = ((float)adc_2_x_calibrated)/ADC_MAX;
		}
		else {
			adc_2_x = ((float)adc_read())/ADC_MAX;
		}

        if ( abs(adc_2_x - ANALOG_CENTER) < deadzone ) { // deadzones
            adc_2_x = ANALOG_CENTER;
        } else if ( analogOptions.analogAdc2Invert == InvertMode::INVERT_X ||
            analogOptions.analogAdc2Invert == InvertMode::INVERT_XY) {
            adc_2_x = ANALOG_MAX - adc_2_x;
        }
    }
    if ( isValidPin(analogOptions.analogAdc2PinY) ) {
        adc_select_input(analogOptions.analogAdc2PinY-26); // ANALOG2-Y
        
		if (analogOptions.auto_calibrate) {
			uint16_t adc_2_y_hold = adc_read();

			// Calibrate axis based on off-center
			uint16_t adc_2_y_calibrated;
			if (adc_2_y_hold >= adc_2_y_center) {
				adc_2_y_calibrated = map(adc_2_y_hold, adc_2_y_center, ADC_MAX, ADC_MAX/2, ADC_MAX);
			}
			else{
				adc_2_y_calibrated = map(adc_2_y_hold, 0, adc_2_y_center, 0, ADC_MAX/2);
			}
			
			adc_2_y = ((float)adc_2_y_calibrated)/ADC_MAX;
		}
		else {
			adc_2_y = ((float)adc_read())/ADC_MAX;
		}

        if ( abs(adc_2_y - ANALOG_CENTER) < deadzone ) { // deadzones
            adc_2_y = ANALOG_CENTER;
        } else if ( analogOptions.analogAdc2Invert == InvertMode::INVERT_Y ||
            analogOptions.analogAdc2Invert == InvertMode::INVERT_XY) {
            adc_2_y = ANALOG_MAX - adc_2_y;
        }
    }
    
    // Alter coordinates to force perfect circularity
    if (analogOptions.forced_circularity) {
        float adc_1_x_magnitude = adc_1_x-(ANALOG_CENTER);
        float adc_1_y_magnitude = adc_1_y-(ANALOG_CENTER);
        float adc_1_magnitude = sqrt((adc_1_x_magnitude * adc_1_x_magnitude) + (adc_1_y_magnitude * adc_1_y_magnitude));
        
        if ( adc_1_magnitude > ANALOG_CENTER) {
            adc_1_x = ((adc_1_x_magnitude / adc_1_magnitude) * ANALOG_CENTER + ANALOG_CENTER);
            adc_1_y = ((adc_1_y_magnitude / adc_1_magnitude) * ANALOG_CENTER + ANALOG_CENTER);
        }

        float adc_2_x_magnitude = adc_2_x-(ANALOG_CENTER);
        float adc_2_y_magnitude = adc_2_y-(ANALOG_CENTER);
        float adc_2_magnitude = sqrt((adc_2_x_magnitude * adc_2_x_magnitude) + (adc_2_y_magnitude * adc_2_y_magnitude));
        
        if ( adc_2_magnitude > ANALOG_CENTER) {
            adc_2_x = ((adc_2_x_magnitude / adc_2_magnitude) * ANALOG_CENTER + ANALOG_CENTER);
            adc_2_y = ((adc_2_y_magnitude / adc_2_magnitude) * ANALOG_CENTER + ANALOG_CENTER);
        }
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

uint16_t AnalogInput::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}