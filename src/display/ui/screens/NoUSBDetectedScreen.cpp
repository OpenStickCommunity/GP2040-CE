#include "NoUSBDetectedScreen.h"

#include "BitmapScreens.h"

#include "pico/stdlib.h"
#include "system.h"

void NoUSBDetectedScreen::init() {
    getRenderer()->clearScreen();
}

void NoUSBDetectedScreen::shutdown() {
    clearElements();
}

void NoUSBDetectedScreen::drawScreen() {
    getRenderer()->drawSprite((uint8_t *)bitmapGP2040Logo, 128, 35, 10, 0, 2, 1);
    getRenderer()->drawText(1, 6, "No USB Data Detected");
    getRenderer()->drawText(1, 7, "Check Cable and Port");
}

int8_t NoUSBDetectedScreen::update() {
    return -1; // -1 means no change in screen state
}
