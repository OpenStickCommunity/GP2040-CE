#include "SplashScreen.h"

#include "pico/stdlib.h"

void SplashScreen::init() {
    getRenderer()->clearScreen();
    splashStartTime = getMillis();
    configMode = Storage::getInstance().GetConfigMode();
}

void SplashScreen::shutdown() {
    clearElements();
}

void SplashScreen::drawScreen() {
	if (getDisplayOptions().splashMode == static_cast<SplashMode>(SPLASH_MODE_NONE)) {
		getRenderer()->drawText(0, 4, " Splash NOT enabled.");
    } else {
            // Default, display static or custom image
            getRenderer()->drawSprite((uint8_t*) getDisplayOptions().splashImage.bytes, 128, 64, 16, 0, 0, 1);
	}
}

int8_t SplashScreen::update() {
    uint32_t elapsedDuration = getMillis() - splashStartTime;
    uint32_t splashDuration = getDisplayOptions().splashDuration;
    if (!configMode) {
        // still running
        if (splashDuration != 0 && (elapsedDuration >= splashDuration)) {
            return DisplayMode::BUTTONS;
        }
    } else {
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
