#include "RestartScreen.h"

#include "BitmapScreens.h"

#include "pico/stdlib.h"
#include "system.h"

void RestartScreen::init() {
    getRenderer()->clearScreen();
    //splashStartTime = getMillis();
}

void RestartScreen::shutdown() {
    clearElements();
}

void RestartScreen::drawScreen() {
    getRenderer()->drawSprite((uint8_t *)bitmapGP2040Logo, 128, 35, 10, 0, 2, 1);

    switch ((System::BootMode)this->bootMode) {
        case System::BootMode::USB:
            getRenderer()->drawText(1, 6, "Rebooting to BOOTSEL");
            getRenderer()->drawText(2, 7, "and Mounting Drive");
            break;
        case System::BootMode::WEBCONFIG:
            getRenderer()->drawText(2, 6, "Booting WebConfig");
            getRenderer()->drawText(4, 7, "Please Wait");
            break;
        case System::BootMode::GAMEPAD:
        case System::BootMode::DEFAULT:
            getRenderer()->drawText(4, 6, "Gamepad Mode");
            getRenderer()->drawText(4, 7, "Please Wait");
            break;
    }
}

void RestartScreen::setBootMode(uint32_t mode) {
    this->bootMode = mode;
}

int8_t RestartScreen::update() {
    return -1; // -1 means no change in screen state
}
