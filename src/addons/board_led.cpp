#include "addons/board_led.h"
#include "usb_driver.h" // Required to check USB state
#include "helper.h"
#include "config.pb.h"

bool BoardLedAddon::available() {
    const OnBoardLedOptions& options = Storage::getInstance().getAddonOptions().onBoardLedOptions;
    return options.enabled && options.mode != OnBoardLedMode::ON_BOARD_LED_MODE_OFF; // Available only when it's not set to off
}

void BoardLedAddon::setup() {
    const OnBoardLedOptions& options = Storage::getInstance().getAddonOptions().onBoardLedOptions;
    onBoardLedMode = static_cast<OnBoardLedMode>(options.mode);
    isConfigMode = Storage::getInstance().GetConfigMode();
    timeSinceBlink = getMillis();
    prevState = -1;

    gpio_init(BOARD_LED_PIN);
    gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
}

void BoardLedAddon::process() {
    bool state = 0;
    Gamepad * gamepad;
    switch (onBoardLedMode) {
        case OnBoardLedMode::ON_BOARD_LED_MODE_INPUT_TEST: // Blinks on input
            gamepad = Storage::getInstance().GetGamepad();
            state =    (gamepad->rawState.buttons != 0)
                    || (gamepad->rawState.dpad    != 0)
                    || (gamepad->rawState.lx      != GAMEPAD_JOYSTICK_MID)
                    || (gamepad->rawState.rx      != GAMEPAD_JOYSTICK_MID)
                    || (gamepad->rawState.ly      != GAMEPAD_JOYSTICK_MID)
                    || (gamepad->rawState.ry      != GAMEPAD_JOYSTICK_MID)
                    || (gamepad->rawState.lt      != 0)
                    || (gamepad->rawState.rt      != 0)
                    || (gamepad->rawState.aux     != 0);
            if (prevState != state) {
                gpio_put(BOARD_LED_PIN, state ? 1 : 0);
            }
            prevState = state;
            break;
        case OnBoardLedMode::ON_BOARD_LED_MODE_MODE_INDICATOR: // Blinks based on USB state and config mode
            if (!get_usb_mounted()) { // USB not mounted
                uint32_t millis = getMillis();
                if ((millis - timeSinceBlink) > BLINK_INTERVAL_USB_UNMOUNTED) {
                    gpio_put(BOARD_LED_PIN, prevState ? 1 : 0);
                    timeSinceBlink = millis;
                    prevState = !prevState;
                }
            } else {
                if (isConfigMode) { // Current mode is config
                    uint32_t millis = getMillis();
                    if ((millis - timeSinceBlink) > BLINK_INTERVAL_CONFIG_MODE) {
                        gpio_put(BOARD_LED_PIN, prevState ? 1 : 0);
                        timeSinceBlink = millis;
                        prevState = !prevState;
                    }
                } else { // Regular mode and functional
                    if (prevState != 1) {
                        gpio_put(BOARD_LED_PIN, 1);
                        prevState = 1;
                    }
                }
            }
            break;
        case OnBoardLedMode::ON_BOARD_LED_MODE_OFF:
            return;
            break;
    }
}
