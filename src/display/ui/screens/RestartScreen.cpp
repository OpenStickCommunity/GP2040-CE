#include "RestartScreen.h"

#include "pico/stdlib.h"
#include "system.h"

void RestartScreen::init() {
    getRenderer()->clearScreen();
    //splashStartTime = getMillis();
    //configMode = Storage::getInstance().GetConfigMode();
}

void RestartScreen::shutdown() {
    clearElements();
}

void RestartScreen::drawScreen() {
    getRenderer()->drawSprite((uint8_t *)bootLogoBottom, 128, 35, 10, 0, 2, 1);
    
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

	//if (getDisplayOptions().splashMode == static_cast<SplashMode>(SPLASH_MODE_NONE)) {
	//	getRenderer()->drawText(0, 4, " Splash NOT enabled.");
    //} else {
    //    int splashMode = getDisplayOptions().splashMode;
    //    int splashSpeed = 40;
    //    if (splashMode == SPLASH_MODE_STATIC) {
    //        // Default, display static or custom image
    //        getRenderer()->drawSprite((uint8_t*) getDisplayOptions().splashImage.bytes, 128, 64, 16, 0, 0, 1);
    //    } else if (splashMode == SPLASH_MODE_CLOSEIN) {
    //        // Close-in. Animate the GP2040 logo
    //        int timeMS = getMillis();
    //        getRenderer()->drawSprite((uint8_t *)bootLogoTop, 43, 39, 6, 43, std::min<int>((timeMS / splashSpeed) - 39, 0), 1);
    //        getRenderer()->drawSprite((uint8_t *)bootLogoBottom, 128, 35, 10, 0, std::max<int>(64 - (timeMS / (splashSpeed * 2)), 30), 1);
    //    } else if (splashMode == SPLASH_MODE_CLOSEINCUSTOM) {
    //        // Close-in on custom image or delayed close-in if custom image does not exist
    //        getRenderer()->drawSprite((uint8_t*) getDisplayOptions().splashImage.bytes, 128, 64, 16, 0, 0, 1);
    //        int timeMS = getMillis();
    //        if (timeMS > 1000) {
    //            int offsetMS = timeMS - 1000;
    //            getRenderer()->drawRectangle(0, 63, 127, 62 - (offsetMS / (splashSpeed * 2)), 0, 1);
    //            getRenderer()->drawSprite((uint8_t *)bootLogoBottom, 128, 35, 10, 0, std::max<int>(64 - (timeMS / splashSpeed), 20), 1);
    //        }
    //    }
	//}
}

void RestartScreen::setBootMode(uint32_t mode) {
    this->bootMode = mode;
}

int8_t RestartScreen::update() {
    //uint32_t elapsedDuration = getMillis() - splashStartTime;
    //uint32_t splashDuration = getDisplayOptions().splashDuration;
    //if (!configMode) {
    //    // still running
    //    if (splashDuration != 0 && (elapsedDuration >= splashDuration)) {
    //        return DisplayMode::BUTTONS;
    //    }
    //} else {
    //    uint16_t buttonState = getGamepad()->state.buttons;
    //    if (prevButtonState && !buttonState) {
    //        if (prevButtonState == GAMEPAD_MASK_B2) {
    //            prevButtonState = 0;
    //            return DisplayMode::CONFIG_INSTRUCTION;
    //        }
    //    }
    //    prevButtonState = buttonState;
    //}
    return -1; // -1 means no change in screen state
}
