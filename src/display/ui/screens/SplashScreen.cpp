#include "SplashScreen.h"

#include "pico/stdlib.h"

void SplashScreen::drawScreen() {
    int mils = getMillis();

	if (getDisplayOptions().splashMode == static_cast<SplashMode>(SPLASH_MODE_NONE)) {
		getRenderer()->drawText(0, 4, " Splash NOT enabled.");
    } else {
        int splashMode = getDisplayOptions().splashMode;
        int splashSpeed = 40;
        if (splashMode == SPLASH_MODE_STATIC) {
            // Default, display static or custom image
            getRenderer()->drawSprite((uint8_t*) getDisplayOptions().splashImage.bytes, 128, 64, 16, 0, 0, 1);
        } else if (splashMode == SPLASH_MODE_CLOSEIN) {
            // Close-in. Animate the GP2040 logo
            getRenderer()->drawSprite((uint8_t *)bootLogoTop, 43, 39, 6, 43, std::min<int>((mils / splashSpeed) - 39, 0), 1);
            getRenderer()->drawSprite((uint8_t *)bootLogoBottom, 128, 35, 10, 0, std::max<int>(64 - (mils / (splashSpeed * 2)), 30), 1);
        } else if (splashMode == SPLASH_MODE_CLOSEINCUSTOM) {
            // Close-in on custom image or delayed close-in if custom image does not exist
            getRenderer()->drawSprite((uint8_t*) getDisplayOptions().splashImage.bytes, 128, 64, 16, 0, 0, 1);
            if (mils > 1000) {
                int milss = mils - 1000;
                getRenderer()->drawRectangle(0, 63, 127, 62 - (milss / (splashSpeed * 2)), 0, 1);
                getRenderer()->drawSprite((uint8_t *)bootLogoBottom, 128, 35, 10, 0, std::max<int>(64 - (mils / splashSpeed), 20), 1);
            }
        }
	}
}

int8_t SplashScreen::update() {
    uint32_t splashDuration = getDisplayOptions().splashDuration;
    bool configMode = getConfigMode();

    if (!configMode) {
        // still running
        if (splashDuration == 0 || getMillis() < splashDuration) {
            return DisplayMode::SPLASH;
        }
        return -1;
    } else {
        uint16_t buttonState = getGamepad()->state.buttons;

        if (prevButtonState && !buttonState) {
            if (prevButtonState == GAMEPAD_MASK_B2) {
                prevButtonState = 0;
                return -1;
            }
        }

        prevButtonState = buttonState;
  
        return DisplayMode::SPLASH;
    }
}
