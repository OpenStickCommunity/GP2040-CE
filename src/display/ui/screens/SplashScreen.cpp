#include "SplashScreen.h"

#include <algorithm>
#include <cstring>

#include "pico/stdlib.h"
#include "drivermanager.h"

void SplashScreen::init() {
    getRenderer()->clearScreen();
    splashStartTime = getMillis();
    configMode = DriverManager::getInstance().isConfigMode();

    const DisplayOptions& options = getDisplayOptions();
    showingText = options.splashTextEnabled
        && (options.splashMode == static_cast<SplashMode>(SPLASH_MODE_NONE) || options.splashTextFirst);
    onSecondStage = false;
}

void SplashScreen::shutdown() {
    clearElements();
}

void SplashScreen::drawScreen() {
    if (showingText) {
        drawSplashText();
    } else if (getDisplayOptions().splashMode == static_cast<SplashMode>(SPLASH_MODE_NONE)) {
        getRenderer()->drawText(0, 4, " Splash NOT enabled.");
    } else {
        // Default, display static or custom image
        getRenderer()->drawSprite((uint8_t*) getDisplayOptions().splashImage.bytes, 128, 64, 16, 0, 0, 1);
    }
}

void SplashScreen::drawSplashText() {
    const DisplayOptions& options = getDisplayOptions();
    const char* lines[] = { options.splashTextLine1, options.splashTextLine2, options.splashTextLine3 };
    uint8_t lineCount = std::min(std::max(options.splashTextLineCount, (uint32_t)1), (uint32_t)3);

    uint16_t blockHeight = (lineCount * SPLASH_TEXT_LINE_HEIGHT) + ((lineCount - 1) * SPLASH_TEXT_LINE_GAP);
    uint16_t y = (SPLASH_SCREEN_HEIGHT - blockHeight) / 2;
    for (uint8_t i = 0; i < lineCount; i++) {
        size_t length = strlen(lines[i]);
        if (length > 0) {
            uint16_t textWidth = (length * SPLASH_TEXT_CHAR_WIDTH) - 1;
            uint16_t x = (textWidth < SPLASH_SCREEN_WIDTH) ? (SPLASH_SCREEN_WIDTH - textWidth) / 2 : 0;
            getRenderer()->drawTextAt(x, y, lines[i]);
        }
        y += SPLASH_TEXT_LINE_HEIGHT + SPLASH_TEXT_LINE_GAP;
    }
}

bool SplashScreen::hasSecondStage() {
    const DisplayOptions& options = getDisplayOptions();
    return options.splashTextEnabled && options.splashMode != static_cast<SplashMode>(SPLASH_MODE_NONE);
}

void SplashScreen::showSecondStage() {
    onSecondStage = true;
    showingText = !showingText;
    splashStartTime = getMillis();
    getRenderer()->clearScreen();
}

int8_t SplashScreen::update() {
    const DisplayOptions& options = getDisplayOptions();
    if (!configMode) {
        uint32_t elapsedDuration = getMillis() - splashStartTime;
        uint32_t duration = showingText ? options.splashTextDuration : options.splashDuration;
        // still running
        if (duration != 0 && (elapsedDuration >= duration)) {
            if (!onSecondStage && hasSecondStage()) {
                showSecondStage();
            } else {
                return DisplayMode::BUTTONS;
            }
        }
    } else {
        uint16_t buttonState = getGamepad()->state.buttons;
        if (prevButtonState && !buttonState) {
            if (prevButtonState == GAMEPAD_MASK_B2) {
                prevButtonState = 0;
                if (!onSecondStage && hasSecondStage()) {
                    showSecondStage();
                } else {
                    return DisplayMode::CONFIG_INSTRUCTION;
                }
            }
        }
        prevButtonState = buttonState;
    }
    return -1; // -1 means no change in screen state
}
