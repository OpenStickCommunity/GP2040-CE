#include "addons/analog_triggers.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

#include "hardware/adc.h"
#include <algorithm>

#define AT_ADC_PIN_OFFSET 26
#define AT_ADC_MAX ((1 << 12) - 1) // 4095, resolucion del ADC del RP2040

bool AnalogTriggerInput::available() {
    const AnalogTriggerOptions& options = Storage::getInstance().getAddonOptions().analogTriggerOptions;
    return options.enabled && (isValidPin(options.leftTriggerPin) || isValidPin(options.rightTriggerPin));
}

void AnalogTriggerInput::setup() {
    const AnalogTriggerOptions& options = Storage::getInstance().getAddonOptions().analogTriggerOptions;

    leftTriggerPin = options.leftTriggerPin;
    rightTriggerPin = options.rightTriggerPin;
    leftTriggerInvert = options.leftTriggerInvert;
    rightTriggerInvert = options.rightTriggerInvert;
    innerDeadzone = options.innerDeadzone / 100.0f;
    outerDeadzone = options.outerDeadzone / 100.0f;

    if (isValidPin(leftTriggerPin)) {
        adc_gpio_init(leftTriggerPin);
        leftTriggerPinAdc = leftTriggerPin - AT_ADC_PIN_OFFSET;
    }

    if (isValidPin(rightTriggerPin)) {
        adc_gpio_init(rightTriggerPin);
        rightTriggerPinAdc = rightTriggerPin - AT_ADC_PIN_OFFSET;
    }
}

// 
uint8_t AnalogTriggerInput::readTriggerByte(uint8_t pinAdc, bool invert, float innerDz, float outerDz) {
    adc_select_input(pinAdc);
    uint16_t raw = adc_read();
    float rawFloat = (float)raw / (float)AT_ADC_MAX; // 0.0 - 1.0

    if (invert) {
        rawFloat = 1.0f - rawFloat;
    }

    float range = outerDz - innerDz;
    float scaled = (rawFloat - innerDz) / (range > 0.0001f ? range : 1.0f);
    scaled = std::clamp(scaled, 0.0f, 1.0f);

    return (uint8_t)(scaled * 255.0f);
}

void AnalogTriggerInput::process() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    if (isValidPin(leftTriggerPin)) {
        gamepad->hasAnalogTriggers = true;
        gamepad->state.lt = readTriggerByte(leftTriggerPinAdc, leftTriggerInvert, innerDeadzone, outerDeadzone);
    }

    if (isValidPin(rightTriggerPin)) {
        gamepad->hasAnalogTriggers = true;
        gamepad->state.rt = readTriggerByte(rightTriggerPinAdc, rightTriggerInvert, innerDeadzone, outerDeadzone);
    }
}
