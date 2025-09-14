#include "SystemErrorScreen.h"

#include "BitmapScreens.h"

#include "pico/stdlib.h"
#include "system.h"

void SystemErrorScreen::init() {
    getRenderer()->clearScreen();
}

void SystemErrorScreen::shutdown() {
    clearElements();
}

void SystemErrorScreen::drawScreen() {
    getRenderer()->drawSprite((uint8_t *)bitmapGP2040Logo, 128, 35, 10, 0, 2, 1);
    getRenderer()->drawText(1, 6, errorMessage.c_str());
    //getRenderer()->drawText(1, 6, "No USB Data Detected");
    //getRenderer()->drawText(1, 7, "Check Cable and Port");
}

int8_t SystemErrorScreen::update() {
    return -1; // -1 means no change in screen state
}
