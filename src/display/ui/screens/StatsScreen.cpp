#include "StatsScreen.h"

#include "pico/stdlib.h"
#include "version.h"

void StatsScreen::init() {
    getRenderer()->clearScreen();
}

void StatsScreen::shutdown() {
    clearElements();
}

void StatsScreen::drawScreen() {
    getRenderer()->drawText(2, 0, "[GP2040-CE Stats]");
    getRenderer()->drawText(0, 1, "Version: " GP2040VERSIONID);
    getRenderer()->drawText(0, 2, "Build: " GP2040BUILD);
    getRenderer()->drawText(0, 3, "Board: " GP2040_BOARDCONFIG);
    getRenderer()->drawText(0, 4, "Type: " GP2040CONFIG);
    getRenderer()->drawText(0, 5, "Arch: " GP2040PLATFORM);

    getRenderer()->drawText(5, 7, "B2 to Return");
}

int8_t StatsScreen::update() {
    if (Storage::getInstance().GetConfigMode()) {
        uint16_t buttonState = getGamepad()->state.buttons;
        if (prevButtonState && !buttonState) {
            if (prevButtonState == GAMEPAD_MASK_B2) {
                prevButtonState = 0;
                return DisplayMode::CONFIG_INSTRUCTION;
            }
        }
        prevButtonState = buttonState;
    }

    return -1; // -1 means no change in screen state
}
