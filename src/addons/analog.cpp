#include "addons/analog.h"
#include "config.pb.h"
#include "enums.pb.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "helper.h"
#include "storagemanager.h"
#include "drivermanager.h"

#include <algorithm>
#include <math.h>

#define ADC_MAX ((1 << 12) - 1) // 4095
#define ADC_PIN_OFFSET ADC_BASE_PIN
#define ANALOG_MAX 1.0f
#define ANALOG_CENTER 0.5f
#define ANALOG_MINIMUM 0.0f
#define ANALOG_MIN_DEADZONE_BAND 0.01f

bool AnalogInput::available() {
    return Storage::getInstance().getAddonOptions().analogOptions.enabled;
}

bool AnalogInput::isAdcPin(Pin_t pin) {
    return pin >= ADC_PIN_OFFSET && pin < ADC_PIN_OFFSET + NUM_ADC_CHANNELS - 1;
}

bool AnalogInput::isAnalogPinUsable(Pin_t pin) {
    return isAnalogMuxPin(pin) || isAdcPin(pin);
}

uint16_t AnalogInput::readCalibrationSample(Pin_t pin) {
    if (!isAdcPin(pin)) {
        return 0;
    }

    adc_gpio_init(pin);
    adc_select_input(pin - ADC_PIN_OFFSET);
    uint32_t sum = 0;
    
    for (int i = 0; i < 16; i++) {
        sum += adc_read();
    }

    return (sum + 8) / 16;
}

void AnalogInput::setup() {
    const AnalogOptions& analogOptions = Storage::getInstance().getAddonOptions().analogOptions;
    
    // Setup our ADC Pair of Sticks
    adc_pairs[0].x_pin = analogOptions.analogAdc1PinX;
    adc_pairs[0].y_pin = analogOptions.analogAdc1PinY;
    adc_pairs[0].analog_invert = analogOptions.analogAdc1Invert;
    adc_pairs[0].analog_dpad = analogOptions.analogAdc1Mode;
    adc_pairs[0].ema_option = analogOptions.analog_smoothing;
    adc_pairs[0].ema_smoothing = analogOptions.smoothing_factor;
    adc_pairs[0].error_rate = analogOptions.analog_error / 1000.0f;
    adc_pairs[0].in_deadzone = analogOptions.inner_deadzone / 100.0f;
    adc_pairs[0].out_deadzone = analogOptions.outer_deadzone / 100.0f;
    adc_pairs[0].auto_calibration = analogOptions.auto_calibrate;
    adc_pairs[0].forced_circularity = analogOptions.forced_circularity;
    adc_pairs[0].joystick_center_x = analogOptions.joystick_center_x;
    adc_pairs[0].joystick_center_y = analogOptions.joystick_center_y;
    adc_pairs[0].x_min = analogOptions.joystick_min_x;
    adc_pairs[0].x_max = analogOptions.joystick_max_x;
    adc_pairs[0].y_min = analogOptions.joystick_min_y;
    adc_pairs[0].y_max = analogOptions.joystick_max_y;
    adc_pairs[1].x_pin = analogOptions.analogAdc2PinX;
    adc_pairs[1].y_pin = analogOptions.analogAdc2PinY;
    adc_pairs[1].analog_invert = analogOptions.analogAdc2Invert;
    adc_pairs[1].analog_dpad = analogOptions.analogAdc2Mode;
    adc_pairs[1].ema_option = analogOptions.analog_smoothing2;
    adc_pairs[1].ema_smoothing = analogOptions.smoothing_factor2;
    adc_pairs[1].error_rate = analogOptions.analog_error2 / 1000.0f;
    adc_pairs[1].in_deadzone = analogOptions.inner_deadzone2 / 100.0f;
    adc_pairs[1].out_deadzone = analogOptions.outer_deadzone2 / 100.0f;
    adc_pairs[1].auto_calibration = analogOptions.auto_calibrate2;
    adc_pairs[1].forced_circularity = analogOptions.forced_circularity2;
    adc_pairs[1].joystick_center_x = analogOptions.joystick_center_x2;
    adc_pairs[1].joystick_center_y = analogOptions.joystick_center_y2;
    adc_pairs[1].x_min = analogOptions.joystick_min_x2;
    adc_pairs[1].x_max = analogOptions.joystick_max_x2;
    adc_pairs[1].y_min = analogOptions.joystick_min_y2;
    adc_pairs[1].y_max = analogOptions.joystick_max_y2;

    // Bring the multiplexer up first: auto-calibration below reads through it
    muxSetup(analogOptions);

    // Setup defaults and helpers
    for (int i = 0; i < ADC_COUNT; i++) {
        adc_pairs[i].x_mux = isAnalogMuxPin(adc_pairs[i].x_pin);
        adc_pairs[i].y_mux = isAnalogMuxPin(adc_pairs[i].y_pin);
        // Mux channels are unusable until S0-S2 and Z are all configured
        if (adc_pairs[i].x_mux && !muxReady) {
            adc_pairs[i].x_pin = -1;
            adc_pairs[i].x_mux = false;
        }
        if (adc_pairs[i].y_mux && !muxReady) {
            adc_pairs[i].y_pin = -1;
            adc_pairs[i].y_mux = false;
        }
        if (!adc_pairs[i].x_mux && !isAdcPin(adc_pairs[i].x_pin)) {
            adc_pairs[i].x_pin = -1;
        }
        if (!adc_pairs[i].y_mux && !isAdcPin(adc_pairs[i].y_pin)) {
            adc_pairs[i].y_pin = -1;
        }

        adc_pairs[i].x_pin_adc = adc_pairs[i].x_mux ? (adc_pairs[i].x_pin - ANALOG_MUX_PIN_BASE) : (adc_pairs[i].x_pin - ADC_PIN_OFFSET);
        adc_pairs[i].y_pin_adc = adc_pairs[i].y_mux ? (adc_pairs[i].y_pin - ANALOG_MUX_PIN_BASE) : (adc_pairs[i].y_pin - ADC_PIN_OFFSET);
        adc_pairs[i].in_deadzone = std::clamp(adc_pairs[i].in_deadzone, ANALOG_MINIMUM, ANALOG_MAX - ANALOG_MIN_DEADZONE_BAND);
        adc_pairs[i].out_deadzone = std::clamp(adc_pairs[i].out_deadzone, adc_pairs[i].in_deadzone + ANALOG_MIN_DEADZONE_BAND, ANALOG_MAX);
        adc_pairs[i].x_value = ANALOG_CENTER;
        adc_pairs[i].y_value = ANALOG_CENTER;
        adc_pairs[i].xy_magnitude = 0.0f;
        adc_pairs[i].x_magnitude = 0.0f;
        adc_pairs[i].y_magnitude = 0.0f;
        adc_pairs[i].x_ema_initialized = false;
        adc_pairs[i].y_ema_initialized = false;
        adc_pairs[i].x_ema = ANALOG_CENTER;
        adc_pairs[i].y_ema = ANALOG_CENTER;
        const float strength = adc_pairs[i].ema_smoothing;
        adc_pairs[i].ema_option &= strength > 0.0f && strength <= 10.0f;
        adc_pairs[i].ema_smoothing = adc_pairs[i].ema_option ? powf(10.0f, -0.5f * strength) : 0.0f;
        
        if (adc_pairs[i].error_rate <= 0.0f || adc_pairs[i].error_rate > 1.0f) {
            adc_pairs[i].error_rate = 1.0f;
        }
    }

    // Intialize and auto center X/Y for each pair
    for (int i = 0; i < ADC_COUNT; i++) {
        if(isAnalogPinUsable(adc_pairs[i].x_pin)) {
            if (!adc_pairs[i].x_mux) {
                adc_gpio_init(adc_pairs[i].x_pin);
            }
            if (adc_pairs[i].auto_calibration) {
                adc_pairs[i].x_center = adc_pairs[i].x_mux ? readRaw(true, adc_pairs[i].x_pin_adc) : readCalibrationSample(adc_pairs[i].x_pin);
            } else {
                // if auto calibration is disabled, attempt to use stored manual calibration value
                adc_pairs[i].x_center = adc_pairs[i].joystick_center_x;
            }
        }
        if(isAnalogPinUsable(adc_pairs[i].y_pin)) {
            if (!adc_pairs[i].y_mux) {
                adc_gpio_init(adc_pairs[i].y_pin);
            }
            if (adc_pairs[i].auto_calibration) {
                adc_pairs[i].y_center = adc_pairs[i].y_mux ? readRaw(true, adc_pairs[i].y_pin_adc) : readCalibrationSample(adc_pairs[i].y_pin);
            } else {
                // if auto calibration is disabled, attempt to use stored manual calibration value
                adc_pairs[i].y_center = adc_pairs[i].joystick_center_y;
            }
        }
    }

    // Analog triggers
    triggers[0].pin = analogOptions.triggerLPin;
    triggers[0].min = (float)analogOptions.triggerLMin;
    triggers[0].max = (float)analogOptions.triggerLMax;
    triggers[1].pin = analogOptions.triggerRPin;
    triggers[1].min = (float)analogOptions.triggerRMin;
    triggers[1].max = (float)analogOptions.triggerRMax;
    for (int i = 0; i < 2; i++) {
        triggers[i].mux = isAnalogMuxPin(triggers[i].pin);
        if (triggers[i].mux && !muxReady) {
            triggers[i].pin = -1;
            triggers[i].mux = false;
        }
        triggers[i].pin_adc = triggers[i].mux ? (triggers[i].pin - ANALOG_MUX_PIN_BASE) : (triggers[i].pin - ADC_PIN_OFFSET);
        triggers[i].primed = false;
        triggers[i].ema = 0.0f;
        if (isAnalogPinUsable(triggers[i].pin) && !triggers[i].mux) {
            adc_gpio_init(triggers[i].pin);
        }
    }
    if (isAnalogPinUsable(triggers[0].pin) || isAnalogPinUsable(triggers[1].pin)) {
        // Drivers only use state.lt / state.rt when this is set
        Storage::getInstance().GetGamepad()->hasAnalogTriggers = true;
    }
}

void AnalogInput::muxSetup(const AnalogOptions& analogOptions) {
    muxSelectPins[0] = analogOptions.muxSelectPin0;
    muxSelectPins[1] = analogOptions.muxSelectPin1;
    muxSelectPins[2] = analogOptions.muxSelectPin2;
    muxZPin = analogOptions.muxZPin;
    muxChannel = -1;

    // Z must be an ADC-capable pin (same range the web config offers as analog pins)
    bool zIsAdc = isAdcPin(muxZPin);
    muxReady = zIsAdc &&
        isValidPin(muxSelectPins[0]) && isValidPin(muxSelectPins[1]) && isValidPin(muxSelectPins[2]);

    if (muxReady) {
        for (int i = 0; i < 3; i++) {
            gpio_init(muxSelectPins[i]);
            gpio_set_dir(muxSelectPins[i], GPIO_OUT);
            gpio_put(muxSelectPins[i], 0);
        }
        adc_gpio_init(muxZPin);
    }
}

void AnalogInput::muxSelect(uint8_t channel) {
    if ((int8_t)channel == muxChannel) {
        return;
    }
    for (int i = 0; i < 3; i++) {
        gpio_put(muxSelectPins[i], (channel >> i) & 0x01);
    }
    muxChannel = (int8_t)channel;
    busy_wait_us(ANALOG_MUX_SETTLE_US);
}

// Read one 12-bit sample either straight from an ADC pin, or from a mux channel via Z.
uint16_t AnalogInput::readRaw(bool mux, Pin_t pin_adc) {
    if (mux) {
        muxSelect((uint8_t)pin_adc);
        adc_select_input(muxZPin - ADC_PIN_OFFSET);
        // Discard one sample: the ADC sample-and-hold still holds charge from the
        // previous channel, which shows up as crosstalk between mux channels.
        (void)adc_read();
        // Average several samples to tame the noise a single conversion picks up
        uint32_t sum = 0;
        for (int i = 0; i < ANALOG_MUX_SAMPLES; i++) {
            sum += adc_read();
        }
        return (uint16_t)(sum / ANALOG_MUX_SAMPLES);
    }
    adc_select_input(pin_adc);
    return adc_read();
}

uint8_t AnalogInput::readTrigger(trigger_instance & trigger) {
    float raw = (float)readRaw(trigger.mux, trigger.pin_adc);
    if (!trigger.primed) {
        trigger.ema = raw;
        trigger.primed = true;
    } else {
        trigger.ema += ANALOG_TRIGGER_EMA * (raw - trigger.ema);
    }

    float range = trigger.max - trigger.min;   // negative range = inverted sensor
    if (range == 0.0f) {
        return 0;
    }
    float normalized = std::clamp((trigger.ema - trigger.min) / range, 0.0f, 1.0f);
    if (normalized < ANALOG_TRIGGER_DEADZONE) {
        return 0;
    }
    return (uint8_t)(normalized * GAMEPAD_TRIGGER_MAX + 0.5f);
}

void AnalogInput::process() {
    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    
    uint32_t joystickMid = GAMEPAD_JOYSTICK_MID;
    uint32_t joystickMax = GAMEPAD_JOYSTICK_MAX;
    if ( DriverManager::getInstance().getDriver() != nullptr ) {
        joystickMid = DriverManager::getInstance().getDriver()->GetJoystickMidValue();
        joystickMax = joystickMid * 2; // 0x8000 mid must be 0x10000 max, but we reduce by 1 if we're maxed out
    }

    for(int i = 0; i < ADC_COUNT; i++) {
        // Read X-Axis
        if (isAnalogPinUsable(adc_pairs[i].x_pin)) {
            adc_pairs[i].x_value = readPin(adc_pairs[i].x_pin_adc, adc_pairs[i].x_mux, adc_pairs[i].x_center,
                adc_pairs[i].x_min, adc_pairs[i].x_max);
            if (adc_pairs[i].analog_invert == InvertMode::INVERT_X || 
                adc_pairs[i].analog_invert == InvertMode::INVERT_XY) {
                adc_pairs[i].x_value = ANALOG_MAX - adc_pairs[i].x_value;
            }
            if (adc_pairs[i].ema_option) {
                if (adc_pairs[i].x_ema_initialized)
                    adc_pairs[i].x_value = emaCalculation(i, adc_pairs[i].x_value, adc_pairs[i].x_ema);
                adc_pairs[i].x_ema = adc_pairs[i].x_value;
                adc_pairs[i].x_ema_initialized = true;
            }
        }
        // Read Y-Axis
        if (isAnalogPinUsable(adc_pairs[i].y_pin)) {
            adc_pairs[i].y_value = readPin(adc_pairs[i].y_pin_adc, adc_pairs[i].y_mux, adc_pairs[i].y_center,
                adc_pairs[i].y_min, adc_pairs[i].y_max);
            if (adc_pairs[i].analog_invert == InvertMode::INVERT_Y || 
                adc_pairs[i].analog_invert == InvertMode::INVERT_XY) {
                adc_pairs[i].y_value = ANALOG_MAX - adc_pairs[i].y_value;
            }
            if (adc_pairs[i].ema_option) {
                if (adc_pairs[i].y_ema_initialized)
                    adc_pairs[i].y_value = emaCalculation(i, adc_pairs[i].y_value, adc_pairs[i].y_ema);
                adc_pairs[i].y_ema = adc_pairs[i].y_value;
                adc_pairs[i].y_ema_initialized = true;
            }
        }
        // Look for dead-zones and circularity
        adc_pairs[i].xy_magnitude = magnitudeCalculation(i, adc_pairs[i]);
        if (adc_pairs[i].xy_magnitude <= adc_pairs[i].in_deadzone) {
            adc_pairs[i].x_value = ANALOG_CENTER;
            adc_pairs[i].y_value = ANALOG_CENTER;
        } else {
            radialDeadzone(i, adc_pairs[i]);
        }

        // If MID is 0x8000, clamp our max to 0xFFFF incase we are at 0x10000. 0x7FFF will max at 0xFFFE
        uint16_t clampedX = (uint16_t)std::min((uint32_t)(joystickMax * std::min(adc_pairs[i].x_value, 1.0f)), (uint32_t)0xFFFF);
        uint16_t clampedY = (uint16_t)std::min((uint32_t)(joystickMax * std::min(adc_pairs[i].y_value, 1.0f)), (uint32_t)0xFFFF);

        if (adc_pairs[i].x_value == ANALOG_CENTER && adc_pairs[i].y_value == ANALOG_CENTER) {
            continue;
        }

        if (adc_pairs[i].analog_dpad == DpadMode::DPAD_MODE_LEFT_ANALOG) {
            gamepad->state.lx = clampedX;
            gamepad->state.ly = clampedY;
        } else if (adc_pairs[i].analog_dpad == DpadMode::DPAD_MODE_RIGHT_ANALOG) {
            gamepad->state.rx = clampedX;
            gamepad->state.ry = clampedY;
        }
    }

    // Analog triggers (state.lt / state.rt are cleared every input cycle by Gamepad::read)
    if (isAnalogPinUsable(triggers[0].pin) || isAnalogPinUsable(triggers[1].pin)) {
        gamepad->hasAnalogTriggers = true;
        if (isAnalogPinUsable(triggers[0].pin)) {
            gamepad->state.lt = readTrigger(triggers[0]);
        }
        if (isAnalogPinUsable(triggers[1].pin)) {
            gamepad->state.rt = readTrigger(triggers[1]);
        }
    }
}

float AnalogInput::readPin(Pin_t pin_adc, bool mux, uint32_t center, uint32_t minimum, uint32_t maximum) {
    uint16_t adc_value = readRaw(mux, pin_adc);
    // Only a valid calibration changes the raw ADC scaling
    if (minimum < center && center < maximum && maximum <= ADC_MAX) {
        const float delta = (float)adc_value - center;
        const float span = adc_value < center ? center - minimum : maximum - center;
        return std::clamp(ANALOG_CENTER + ANALOG_CENTER * (delta / span), ANALOG_MINIMUM, ANALOG_MAX);
    }
    return ((float)adc_value) / ADC_MAX;
}

float AnalogInput::emaCalculation(int stick_num, float ema_value, float ema_previous) {
    return ema_previous + adc_pairs[stick_num].ema_smoothing * (ema_value - ema_previous);
}

float AnalogInput::magnitudeCalculation(int stick_num, adc_instance & adc_inst) {
    adc_inst.x_magnitude = adc_inst.x_value - ANALOG_CENTER;
    adc_inst.y_magnitude = adc_inst.y_value - ANALOG_CENTER;
    return adc_pairs[stick_num].error_rate * std::sqrt((adc_inst.x_magnitude * adc_inst.x_magnitude) + (adc_inst.y_magnitude * adc_inst.y_magnitude));
}

void AnalogInput::radialDeadzone(int stick_num, adc_instance & adc_inst) {
    float scaling_factor = (adc_inst.xy_magnitude - adc_pairs[stick_num].in_deadzone) / (adc_pairs[stick_num].out_deadzone - adc_pairs[stick_num].in_deadzone);
    if (adc_pairs[stick_num].forced_circularity == true) {
        scaling_factor = std::fmin(scaling_factor, ANALOG_CENTER * adc_inst.error_rate);
    }
    adc_inst.x_value = ((adc_inst.x_magnitude / adc_inst.xy_magnitude) * scaling_factor) + ANALOG_CENTER;
    adc_inst.y_value = ((adc_inst.y_magnitude / adc_inst.xy_magnitude) * scaling_factor) + ANALOG_CENTER;
    adc_inst.x_value = std::clamp(adc_inst.x_value, ANALOG_MINIMUM, ANALOG_MAX);
    adc_inst.y_value = std::clamp(adc_inst.y_value, ANALOG_MINIMUM, ANALOG_MAX);
}
