#include "hardware/pwm.h"
#include "addons/reactiveleds.h"
#include "storagemanager.h"
#include "usbdriver.h"
#include "helper.h"
#include "config.pb.h"

bool ReactiveLEDAddon::available() {
    bool pinsEnabled = false;
    const ReactiveLEDOptions& options = Storage::getInstance().getAddonOptions().reactiveLEDOptions;
    for (uint8_t led = 0; led < REACTIVE_LED_COUNT; led++) {
        if (isValidPin(options.leds[led].pin)) {
            pinsEnabled = true;
            break;
        }
    }
    return options.enabled && pinsEnabled;
}

void ReactiveLEDAddon::setup() {
    const ReactiveLEDOptions& options = Storage::getInstance().getAddonOptions().reactiveLEDOptions;

    for (uint8_t led = 0; led < REACTIVE_LED_COUNT; led++) {
        ReactiveLEDInfo ledInfo = options.leds[led];

        ledPins[led].pinNumber = ledInfo.pin;
        ledPins[led].action = ledInfo.action;
        ledPins[led].modeDown = ledInfo.modeDown;
        ledPins[led].modeUp = ledInfo.modeUp;

        if (isValidPin(ledPins[led].pinNumber)) {
            gpio_init(ledPins[led].pinNumber);
            gpio_set_dir(ledPins[led].pinNumber, GPIO_OUT);
            gpio_set_function(ledPins[led].pinNumber, GPIO_FUNC_PWM);

            pwm_set_wrap(pwm_gpio_to_slice_num(ledPins[led].pinNumber), REACTIVE_LED_MAX_BRIGHTNESS);
            pwm_set_enabled(pwm_gpio_to_slice_num(ledPins[led].pinNumber), true);

            ledPins[led].lastUpdate = to_ms_since_boot(get_absolute_time());

            setLEDByMode(ledPins[led], false);
        }
    }
}

void ReactiveLEDAddon::process() {
    Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

    uint32_t currUpdate = to_ms_since_boot(get_absolute_time());

    for (uint8_t led = 0; led < REACTIVE_LED_COUNT; led++) {
        if (isValidPin(ledPins[led].pinNumber) && ledPins[led].action != GpioAction::NONE) {
            ledPins[led].currUpdate = currUpdate;
            switch (ledPins[led].action) {
                case BUTTON_PRESS_UP: setLEDByMode(ledPins[led], gamepad->pressedDpad(GAMEPAD_MASK_UP)); break;
                case BUTTON_PRESS_DOWN: setLEDByMode(ledPins[led], gamepad->pressedDpad(GAMEPAD_MASK_DOWN)); break;
                case BUTTON_PRESS_LEFT: setLEDByMode(ledPins[led], gamepad->pressedDpad(GAMEPAD_MASK_LEFT)); break;
                case BUTTON_PRESS_RIGHT: setLEDByMode(ledPins[led], gamepad->pressedDpad(GAMEPAD_MASK_RIGHT)); break;
                case BUTTON_PRESS_B1: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_B1)); break;
                case BUTTON_PRESS_B2: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_B2)); break;
                case BUTTON_PRESS_B3: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_B3)); break;
                case BUTTON_PRESS_B4: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_B4)); break;
                case BUTTON_PRESS_L1: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_L1)); break;
                case BUTTON_PRESS_R1: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_R1)); break;
                case BUTTON_PRESS_L2: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_L2)); break;
                case BUTTON_PRESS_R2: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_R2)); break;
                case BUTTON_PRESS_S1: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_S1)); break;
                case BUTTON_PRESS_S2: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_S2)); break;
                case BUTTON_PRESS_A1: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_A1)); break;
                case BUTTON_PRESS_A2: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_A2)); break;
                case BUTTON_PRESS_L3: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_L3)); break;
                case BUTTON_PRESS_R3: setLEDByMode(ledPins[led], gamepad->pressedButton(GAMEPAD_MASK_R3)); break;
                default: break;
            }
        }
    }
}

void ReactiveLEDAddon::setLEDByMode(ReactiveLEDPinState &ledState, bool pressed) {
    ledState.currState = pressed;

    switch (pressed ? ledState.modeDown : ledState.modeUp) {
        case ReactiveLEDMode::REACTIVE_LED_STATIC_OFF:
            ledState.value = 0;
            break;
        case ReactiveLEDMode::REACTIVE_LED_STATIC_ON:
            ledState.value = 255;
            break;
        case ReactiveLEDMode::REACTIVE_LED_FADE_IN:
            if (ledState.currUpdate - ledState.lastUpdate >= REACTIVE_LED_DELAY) {
                if (ledState.prevState != pressed) ledState.value = 0;
                if (ledState.value < REACTIVE_LED_MAX_BRIGHTNESS) {
                    ledState.value+=REACTIVE_LED_FADE_INC;
                }

                ledState.lastUpdate = ledState.currUpdate;
            }
            break;
        case ReactiveLEDMode::REACTIVE_LED_FADE_OUT:
            if (ledState.currUpdate - ledState.lastUpdate >= REACTIVE_LED_DELAY) {
                if (ledState.prevState != pressed) ledState.value = REACTIVE_LED_MAX_BRIGHTNESS;
                if (ledState.value > 0) {
                    ledState.value-=REACTIVE_LED_FADE_INC;
                }

                ledState.lastUpdate = ledState.currUpdate;
            }
            break;
    }

    pwm_set_gpio_level(ledState.pinNumber, ledState.value);

    ledState.prevState = pressed;
}
