#include "addons/analog.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

#include "enums.pb.h"

#include "hardware/adc.h"

#include <math.h>

#define ADC_MAX ((1 << 12) - 1)
#define ANALOG_CENTER 0.5f // 0.5f is center

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

    // Analog ADC 1
    if ( isValidPin(analogOptions.analogAdc1PinX) ) {
        adc_select_input(analogOptions.analogAdc1PinX-26); // ANALOG-X
        adc_1_x = ((float)adc_read())/ADC_MAX;
        if ( abs(adc_1_x - ANALOG_CENTER) < deadzone ) // deadzones
            adc_1_x = ANALOG_CENTER;
    }
    if ( isValidPin(analogOptions.analogAdc1PinY) ) {
        adc_select_input(analogOptions.analogAdc1PinY-26); // ANALOG-Y
        adc_1_y = ((float)adc_read())/ADC_MAX;
        if ( abs(adc_1_y - ANALOG_CENTER) < deadzone ) // deadzones
            adc_1_y = ANALOG_CENTER;
    }
    if ( isValidPin(analogOptions.analogAdc2PinX) ) {
        adc_select_input(analogOptions.analogAdc2PinX-26); // ANALOG-X
        adc_2_x = ((float)adc_read())/ADC_MAX;
        if ( abs(adc_2_x - ANALOG_CENTER) < deadzone ) // deadzones
            adc_2_x = ANALOG_CENTER;
    }
    if ( isValidPin(analogOptions.analogAdc2PinY) ) {
        adc_select_input(analogOptions.analogAdc2PinY-26); // ANALOG-Y
        adc_2_y = ((float)adc_read())/ADC_MAX;
        if ( abs(adc_2_y - ANALOG_CENTER) < deadzone ) // deadzones
            adc_2_y = ANALOG_CENTER;
    }
    
    // Alter coordinates to force perfect circularity
    if (analogOptions.forced_circularity){
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
