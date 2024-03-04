#include "addons/rotaryencoder.h"

#include "storagemanager.h"
#include "types.h"

#include "GamepadEnums.h"
#include "helper.h"
#include "config.pb.h"

bool RotaryEncoderInput::available() {
    const RotaryOptions& options = Storage::getInstance().getAddonOptions().rotaryOptions;
    return options.enabled;
}

void RotaryEncoderInput::setup()
{
    const RotaryOptions& options = Storage::getInstance().getAddonOptions().rotaryOptions;
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    stdio_init_all();

    encoderMap[0].enabled = options.encoderOne.enabled;
    if (encoderMap[0].enabled) {
        encoderMap[0].pinA = options.encoderOne.pinA;
        encoderMap[0].pinB = options.encoderOne.pinB;
        encoderMap[0].mode = options.encoderOne.mode;
        encoderMap[0].pulsesPerRevolution = options.encoderOne.pulsesPerRevolution;
    }

    encoderMap[1].enabled = options.encoderTwo.enabled;
    if (encoderMap[1].enabled) {
        encoderMap[1].pinA = options.encoderTwo.pinA;
        encoderMap[1].pinB = options.encoderTwo.pinB;
        encoderMap[1].mode = options.encoderTwo.mode;
        encoderMap[1].pulsesPerRevolution = options.encoderTwo.pulsesPerRevolution;
    }

    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        encoderValues[i] = 0;
    
        if (encoderMap[i].enabled) {
            gpio_init(encoderMap[i].pinA);             // Initialize pin
            gpio_set_dir(encoderMap[i].pinA, GPIO_IN); // Set as INPUT
            gpio_pull_up(encoderMap[i].pinA);          // Set as PULLUP
            
            gpio_init(encoderMap[i].pinB);             // Initialize pin
            gpio_set_dir(encoderMap[i].pinB, GPIO_IN); // Set as INPUT
            gpio_pull_up(encoderMap[i].pinB);          // Set as PULLUP
        }
    
        if ((encoderMap[i].mode == ENCODER_MODE_LEFT_TRIGGER) || (encoderMap[i].mode == ENCODER_MODE_RIGHT_TRIGGER)) {
            gamepad->hasAnalogTriggers = true;
            encoderMap[i].minRange = GAMEPAD_TRIGGER_MIN;
            encoderMap[i].maxRange = GAMEPAD_TRIGGER_MAX;
        } else if ((encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_X) || (encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_Y) || (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_X) || (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_Y)) {
            encoderMap[i].minRange = GAMEPAD_JOYSTICK_MIN;
            encoderMap[i].maxRange = GAMEPAD_JOYSTICK_MAX;
        }
    }
}

void RotaryEncoderInput::process()
{
    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    uint32_t now = getMillis();

    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        if (encoderMap[i].enabled) {
            uint32_t lastUpdate = now - encoderState[i].updateTime;

            if (lastUpdate >= encoderState[i].delay) {
                bool pinAValue = gpio_get(encoderMap[i].pinA);
                bool pinBValue = gpio_get(encoderMap[i].pinB);

                if (encoderState[i].pinA != pinAValue || encoderState[i].pinB != pinBValue) {
                    if ((encoderState[i].pinA == encoderState[i].prevA) && (encoderState[i].pinB == encoderState[i].prevB)) {
                        if ((encoderState[i].pinA && !encoderState[i].pinB && pinBValue) || (!encoderState[i].pinA && encoderState[i].pinB && !pinBValue)) {
                            encoderValues[i]+=(360 / (encoderMap[i].pulsesPerRevolution / 4));
                        } else if ((!encoderState[i].pinA && encoderState[i].pinB && pinBValue) || (encoderState[i].pinA && !encoderState[i].pinB && !pinBValue)) {
                            encoderValues[i]-=(360 / (encoderMap[i].pulsesPerRevolution / 4));
                        }
                    }
                }

                encoderState[i].pinA = pinAValue;
                encoderState[i].pinB = pinBValue;
                encoderState[i].prevA = pinAValue;
                encoderState[i].prevB = pinBValue;
                encoderState[i].updateTime = now;
            }
        }
    }

    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        if (encoderMap[i].enabled && (encoderMap[i].mode != ENCODER_MODE_NONE)) {
            if (encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_X) {
                gamepad->state.lx = mapEncoderValueStick(encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_Y) {
                gamepad->state.ly = mapEncoderValueStick(encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_X) {
                gamepad->state.rx = mapEncoderValueStick(encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_Y) {
                gamepad->state.ry = mapEncoderValueStick(encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_LEFT_TRIGGER) {
                gamepad->state.lt = mapEncoderValueTrigger(encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_RIGHT_TRIGGER) {
                gamepad->state.rt = mapEncoderValueTrigger(encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_DPAD_X) {
                int8_t axis = mapEncoderValueDPad(encoderValues[i], encoderMap[i].pulsesPerRevolution);
                dpadLeft = (axis == -1);
                dpadRight = (axis == 1);
            } else if (encoderMap[i].mode == ENCODER_MODE_DPAD_Y) {
                int8_t axis = mapEncoderValueDPad(encoderValues[i], encoderMap[i].pulsesPerRevolution);
                dpadUp = (axis == -1);
                dpadDown = (axis == 1);
            }
        }

        prevValues[i] = encoderValues[i];
    }

    // only modify the dpad if needed to avoid flicker
    if (dpadUp) gamepad->state.dpad |= GAMEPAD_MASK_UP;
    if (dpadDown) gamepad->state.dpad |= GAMEPAD_MASK_DOWN;
    if (dpadLeft) gamepad->state.dpad |= GAMEPAD_MASK_LEFT;
    if (dpadRight) gamepad->state.dpad |= GAMEPAD_MASK_RIGHT;
}

uint16_t RotaryEncoderInput::mapEncoderValueStick(int32_t encoderValue, uint16_t ppr) {
    // Calculate total number of positions based on PPR
    int32_t totalPositions = 360 * (int32_t)(ppr / 4);

    // Calculate range of encoder values corresponding to mapped range
    int32_t minValue = -totalPositions / 2;
    int32_t maxValue = totalPositions / 2;

    int32_t mappedValue = map(encoderValue, minValue, maxValue, encoderMap[0].minRange, encoderMap[0].maxRange);
    int32_t constrainedValue = bounds(mappedValue, encoderMap[0].minRange, encoderMap[0].maxRange);

    return constrainedValue;
}

uint16_t RotaryEncoderInput::mapEncoderValueTrigger(int32_t encoderValue, uint16_t ppr) {
    // Calculate total number of positions based on PPR
    int32_t totalPositions = 360 * (int32_t)(ppr / 4);

    // Calculate range of encoder values corresponding to mapped range
    int32_t minValue = 0;
    int32_t maxValue = totalPositions;

    int32_t mappedValue = map(encoderValue, minValue, maxValue, encoderMap[0].minRange, encoderMap[0].maxRange);
    int32_t constrainedValue = bounds(mappedValue, encoderMap[0].minRange, encoderMap[0].maxRange);

    return constrainedValue;
}

int8_t RotaryEncoderInput::mapEncoderValueDPad(int32_t encoderValue, uint16_t ppr) {
    // Calculate total number of positions based on PPR
    int32_t totalPositions = 360 * (int32_t)(ppr / 4);

    // Calculate range of encoder values corresponding to mapped range
    int32_t minValue = -totalPositions / 2;
    int32_t maxValue = totalPositions / 2;

    // Calculate thresholds for left and right turns
    int32_t leftThreshold = minValue + (maxValue - minValue) / 3;
    int32_t rightThreshold = maxValue - (maxValue - minValue) / 3;

    if (encoderValue < leftThreshold) {
        return -1;
    } else if (encoderValue > rightThreshold) {
        return 1;
    } else {
        return 0;
    }
}

int32_t RotaryEncoderInput::map(int32_t x, int32_t in_min, int32_t in_max, int32_t out_min, int32_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int32_t RotaryEncoderInput::bounds(int32_t x, int32_t out_min, int32_t out_max) {
    return (x < out_min) ? out_min : ((x > out_max) ? out_max : x);
}
