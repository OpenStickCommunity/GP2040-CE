#include "addons/rotaryencoder.h"

#include "eventmanager.h"
#include "storagemanager.h"
#include "GPEncoderEvent.h"
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

    encoderMap[0].enabled = options.encoderOne.enabled && ((options.encoderOne.pinA != -1) && (options.encoderOne.pinB != -1));
    if (encoderMap[0].enabled) {
        encoderMap[0].pinA = options.encoderOne.pinA;
        encoderMap[0].pinB = options.encoderOne.pinB;
        encoderMap[0].mode = options.encoderOne.mode;
        encoderMap[0].pulsesPerRevolution = options.encoderOne.pulsesPerRevolution;
        encoderMap[0].resetAfter = options.encoderOne.resetAfter;
        encoderMap[0].allowWrapAround = options.encoderOne.allowWrapAround;
        encoderMap[0].multiplier = options.encoderOne.multiplier;
    }

    encoderMap[1].enabled = options.encoderTwo.enabled && ((options.encoderTwo.pinA != -1) && (options.encoderTwo.pinB != -1));
    if (encoderMap[1].enabled) {
        encoderMap[1].pinA = options.encoderTwo.pinA;
        encoderMap[1].pinB = options.encoderTwo.pinB;
        encoderMap[1].mode = options.encoderTwo.mode;
        encoderMap[1].pulsesPerRevolution = options.encoderTwo.pulsesPerRevolution;
        encoderMap[1].resetAfter = options.encoderTwo.resetAfter;
        encoderMap[1].allowWrapAround = options.encoderTwo.allowWrapAround;
        encoderMap[1].multiplier = options.encoderTwo.multiplier;
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

                uint32_t encoderIncrement = (ENCODER_RADIUS / (encoderMap[i].pulsesPerRevolution / (ENCODER_PRECISION * encoderMap[i].multiplier)));

                if (encoderState[i].pinA != pinAValue || encoderState[i].pinB != pinBValue) {
                    if ((encoderState[i].pinA == encoderState[i].prevA) && (encoderState[i].pinB == encoderState[i].prevB)) {
                        if ((encoderState[i].pinA && !encoderState[i].pinB && pinBValue) || (!encoderState[i].pinA && encoderState[i].pinB && !pinBValue)) {
                            encoderValues[i]+=encoderIncrement;
                        } else if ((!encoderState[i].pinA && encoderState[i].pinB && pinBValue) || (encoderState[i].pinA && !encoderState[i].pinB && !pinBValue)) {
                            encoderValues[i]-=encoderIncrement;
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
            uint32_t lastChange = now - encoderState[i].changeTime;

            if (encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_X) {
                gamepad->state.lx = -mapEncoderValueStick(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_LEFT_ANALOG_Y) {
                gamepad->state.ly = -mapEncoderValueStick(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_X) {
                gamepad->state.rx = -mapEncoderValueStick(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_RIGHT_ANALOG_Y) {
                gamepad->state.ry = -mapEncoderValueStick(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_LEFT_TRIGGER) {
                gamepad->state.lt = mapEncoderValueTrigger(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_RIGHT_TRIGGER) {
                gamepad->state.rt = mapEncoderValueTrigger(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
            } else if (encoderMap[i].mode == ENCODER_MODE_DPAD_X) {
                int8_t axis = mapEncoderValueDPad(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
                dpadLeft = (axis == 1);
                dpadRight = (axis == -1);
            } else if (encoderMap[i].mode == ENCODER_MODE_DPAD_Y) {
                int8_t axis = mapEncoderValueDPad(i, encoderValues[i], encoderMap[i].pulsesPerRevolution);
                dpadUp = (axis == 1);
                dpadDown = (axis == -1);
            } else if (encoderMap[i].mode == ENCODER_MODE_VOLUME) {
                // Prevents NONE kick-out, do nothing for now but rely on GP events
            }

            if ((encoderValues[i] - prevValues[i]) != 0) {
                encoderState[i].changeTime = now;

                if ((encoderValues[i] - prevValues[i]) > 0) {
                    EventManager::getInstance().triggerEvent(new GPEncoderChangeEvent(i, 1));
                } else if ((encoderValues[i] - prevValues[i]) < 0) {
                    EventManager::getInstance().triggerEvent(new GPEncoderChangeEvent(i, -1));
                }
            }

            if ((encoderMap[i].resetAfter > 0) && (lastChange >= encoderMap[i].resetAfter)) {
                encoderValues[i] = 0;
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

uint16_t RotaryEncoderInput::mapEncoderValueStick(int8_t index, int32_t encoderValue, uint16_t ppr) {
    // Calculate total number of positions based on PPR
    int32_t totalPositions = ENCODER_RADIUS * (int32_t)(ppr / (ENCODER_PRECISION * encoderMap[index].multiplier));

    // Calculate range of encoder values corresponding to mapped range
    int32_t minValue = -totalPositions / 2;
    int32_t maxValue = totalPositions / 2;

    if (encoderMap[index].allowWrapAround) {
        return encoderValue;
    } else {
        int32_t mappedValue = map(encoderValue, minValue, maxValue, encoderMap[index].minRange+1, encoderMap[index].maxRange);
        int32_t constrainedValue = mappedValue;
        
        if (constrainedValue < encoderMap[index].minRange+1) constrainedValue = encoderMap[index].minRange+1;
        if (constrainedValue > encoderMap[index].maxRange) constrainedValue = encoderMap[index].maxRange;

        return constrainedValue;
    }
}

uint16_t RotaryEncoderInput::mapEncoderValueTrigger(int8_t index, int32_t encoderValue, uint16_t ppr) {
    // Calculate total number of positions based on PPR
    int32_t totalPositions = ENCODER_RADIUS * (int32_t)(ppr / (ENCODER_PRECISION * encoderMap[index].multiplier));

    // Calculate range of encoder values corresponding to mapped range
    int32_t minValue = 0;
    int32_t maxValue = totalPositions;

    if (encoderMap[index].allowWrapAround) {
        return encoderValue;
    } else {
        int32_t mappedValue = map(encoderValue, minValue, maxValue, encoderMap[index].minRange, encoderMap[index].maxRange);
        int32_t constrainedValue = bounds(mappedValue, encoderMap[index].minRange, encoderMap[index].maxRange-1);

        return constrainedValue;
    }
}

int8_t RotaryEncoderInput::mapEncoderValueDPad(int8_t index, int32_t encoderValue, uint16_t ppr) {
    // Calculate total number of positions based on PPR
    int32_t totalPositions = ENCODER_RADIUS * (int32_t)(ppr / (ENCODER_PRECISION * encoderMap[index].multiplier));

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

int8_t RotaryEncoderInput::getEncoderIndexByPin(uint8_t pin) {
    for (uint8_t i = 0; i < MAX_ENCODERS; i++) {
        if (encoderMap[i].enabled && ((encoderMap[i].pinA != -1) && (encoderMap[i].pinB != -1))) {
            if ((encoderMap[i].pinA == pin) || (encoderMap[i].pinB == pin)) return pin;
        }
    }
    return -1;
}
