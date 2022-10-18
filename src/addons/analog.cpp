#include "addons/analog.h"
#include "storagemanager.h"

#include "hardware/adc.h"

#define ADC_MAX ((1 << 12) - 1)
#define ANALOG_CENTER 0.5f // 0.5f is center
#define ANALOG_DEADZONE 0.05f // move to config (future release)

bool AnalogInput::available() {
	return (ANALOG_ADC_VRX != -1 && ANALOG_ADC_VRY != -1);
}

void AnalogInput::setup() {
    adc_init();
    // Make sure GPIO is high-impedance, no pullups etc
    adc_gpio_init(ANALOG_ADC_VRX);
    adc_gpio_init(ANALOG_ADC_VRY);
}

void AnalogInput::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    adc_select_input(0); // ANALOG-X
    float adc_x = ((float)adc_read())/ADC_MAX;
    adc_select_input(1); // ANALOG-Y
    float adc_y = ((float)adc_read())/ADC_MAX;
    if ( abs(adc_x - ANALOG_CENTER) < ANALOG_DEADZONE ) // deadzones
        adc_x = ANALOG_CENTER;
    if ( abs(adc_y - ANALOG_CENTER) < ANALOG_DEADZONE ) // deadzones
        adc_y = ANALOG_CENTER;

    // Convert to 16-bit value
    gamepad->state.lx = (uint16_t)(65535.0f*adc_x);
    gamepad->state.ly = (uint16_t)(65535.0f*adc_y);
}
