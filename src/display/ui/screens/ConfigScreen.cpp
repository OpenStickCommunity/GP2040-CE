#include "ConfigScreen.h"

#include "version.h"

void ConfigScreen::drawScreen() {
    getRenderer()->drawText(0, 1, "[Web Config Mode]");
    getRenderer()->drawText(0, 2, std::string("GP2040-CE : ") + std::string(GP2040VERSION));
    getRenderer()->drawText(0, 3, "[http://192.168.7.1]");
    getRenderer()->drawText(0, 4, "Preview:");
    getRenderer()->drawText(5, 5, "B1 > Button");
    getRenderer()->drawText(5, 6, "B2 > Splash");

    getRenderer()->drawText(0, 0, header);
    getRenderer()->drawText(0, 50, footer);
}

int8_t ConfigScreen::update() {
    uint16_t buttonState = getGamepad()->state.buttons;

    if (prevButtonState && !buttonState) {
        switch (prevButtonState) {
            case (GAMEPAD_MASK_B1):
                //prevDisplayMode = prevDisplayMode == DisplayMode::BUTTONS ? DisplayMode::CONFIG_INSTRUCTION : DisplayMode::BUTTONS;
                prevButtonState = 0;
                return DisplayMode::BUTTONS;
            case (GAMEPAD_MASK_B2):
                //prevDisplayMode = prevDisplayMode == DisplayMode::SPLASH ? DisplayMode::CONFIG_INSTRUCTION : DisplayMode::SPLASH;
                prevButtonState = 0;
                return DisplayMode::SPLASH;
            default:
                //prevDisplayMode = DisplayMode::CONFIG_INSTRUCTION;
                break;
        }
    }

    prevButtonState = buttonState;

    return DisplayMode::CONFIG_INSTRUCTION;
}
