#include "ConfigScreen.h"
#include "version.h"

void ConfigScreen::init() {
    version = "GP2040-CE : ";
    version += GP2040VERSION;
    getRenderer()->clearScreen();
}

void ConfigScreen::shutdown() {
    clearElements();
}

int8_t ConfigScreen::update() {
    uint16_t buttonState = getGamepad()->state.buttons;
    if (prevButtonState && !buttonState) {
        switch (prevButtonState) {
            case (GAMEPAD_MASK_B1):
                prevButtonState = 0;
                return DisplayMode::BUTTONS;
            case (GAMEPAD_MASK_B2):
                prevButtonState = 0;
                return DisplayMode::SPLASH;
            case (GAMEPAD_MASK_R2):
                prevButtonState = 0;
                return DisplayMode::PIN_VIEWER;
            case (GAMEPAD_MASK_L2):
                prevButtonState = 0;
                return DisplayMode::STATS;
            default:
                break;
        }
    }
    prevButtonState = buttonState;
    return -1;
}

void ConfigScreen::drawScreen() {
    getRenderer()->drawText(2, 0, "[Web Config Mode]");
    getRenderer()->drawText(0, 1, version);
    getRenderer()->drawText(0, 2, "[http://192.168.7.1]");
    getRenderer()->drawText(0, 3, "Preview:");
    getRenderer()->drawText(3, 4, "  B1 > Button");
    getRenderer()->drawText(3, 5, "  B2 > Splash");
    getRenderer()->drawText(3, 6, "  R2 > Pin Viewer");
    getRenderer()->drawText(3, 7, "  L2 > Stats");
}