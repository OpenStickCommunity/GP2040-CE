#include "addons/board_led.h"
#include "usb_driver.h" // Required to check USB state

bool BoardLedAddon::available() {
    const AddonOptions& options = Storage::getInstance().getAddonOptions();
    onBoardLedMode = options.onBoardLedMode;
    return options.BoardLedAddonEnabled &&
        onBoardLedMode != OnBoardLedMode::BOARD_LED_OFF; // Available only when it's not set to off
}

void BoardLedAddon::setup() {
    gpio_init(BOARD_LED_PIN);
    gpio_set_dir(BOARD_LED_PIN, GPIO_OUT);
    isConfigMode = Storage::getInstance().GetConfigMode();
    timeSinceBlink = getMillis();
    prevState = -1;
}

void BoardLedAddon::process() {
    bool state = 0;
    Gamepad * gamepad;
    switch (onBoardLedMode) {
        case OnBoardLedMode::INPUT_TEST: // Blinks on input
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
        case OnBoardLedMode::MODE_INDICATOR: // Blinks based on USB state and config mode
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
        case OnBoardLedMode::BOARD_LED_OFF:
            return;
            break;
    }
}
