/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "addons/display.h"
#include "GamepadState.h"
#include "enums.h"
#include "storagemanager.h"
#include "pico/stdlib.h"
#include "bitmaps.h"

#include "drivermanager.h"
#include "usbdriver.h"
#include "version.h"
#include "config.pb.h"
#include "class/hid/hid.h"

bool DisplayAddon::available() {
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    return options.enabled && PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock);
}

void DisplayAddon::setup() {
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    PeripheralI2C* i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);

    // Setup GPGFX Options
    GPGFX_DisplayTypeOptions gpOptions;
    if (PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock)) {
        gpOptions.displayType = GPGFX_DisplayType::TYPE_SSD1306;
        gpOptions.i2c = i2c;
        gpOptions.size = options.size;
        gpOptions.address = options.i2cAddress;
        gpOptions.orientation = options.flip;
        gpOptions.inverted = options.invert;
        gpOptions.font.fontData = GP_Font_Standard;
        gpOptions.font.width = 6;
        gpOptions.font.height = 8;
    } else {
        return; // Do not run our display
    }

    // Setup GPGFX
    gpDisplay = new GPGFX();
    gpDisplay->init(gpOptions);

    gamepad = Storage::getInstance().GetGamepad();

    displaySaverTimer = options.displaySaverTimeout;
    displaySaverTimeout = displaySaverTimer;
    configMode = Storage::getInstance().GetConfigMode();
    turnOffWhenSuspended = options.turnOffWhenSuspended;

    // set current display mode
    if (!configMode) {
        if (Storage::getInstance().getDisplayOptions().splashMode != static_cast<SplashMode>(SPLASH_MODE_NONE)) {
            currDisplayMode = DisplayMode::SPLASH;
        } else {
            currDisplayMode = DisplayMode::BUTTONS;
        }
    } else {
        currDisplayMode = DisplayMode::CONFIG_INSTRUCTION;
    }
    gpScreen = nullptr;
    updateDisplayScreen();
}

bool DisplayAddon::updateDisplayScreen() {
    if ( gpScreen != nullptr ) {
        gpScreen->shutdown();
        switch(prevDisplayMode) {
            case CONFIG_INSTRUCTION:
                delete (ConfigScreen*)gpScreen;
                break;
            case SPLASH:
                delete (SplashScreen*)gpScreen;
                break;
            case MAIN_MENU:
                delete (SplashScreen*)gpScreen;
                break;
            case BUTTONS:
                delete (ButtonLayoutScreen*)gpScreen;
                break;
            default:
                break;
        }
        gpScreen = nullptr;
    }
    switch(currDisplayMode) {
        case CONFIG_INSTRUCTION:
            gpScreen = new ConfigScreen(gpDisplay);
            break;
        case SPLASH:
            gpScreen = new SplashScreen(gpDisplay);
            break;
        case MAIN_MENU:
            gpScreen = new MainMenuScreen(gpDisplay);
            break;
        case BUTTONS:
            gpScreen = new ButtonLayoutScreen(gpDisplay);
            break;
        default:
            gpScreen = nullptr;
            break;
    };

    if (gpScreen != nullptr) {
        gpScreen->init();
        prevDisplayMode = currDisplayMode;
        return true;
    }
    return true;
}

bool DisplayAddon::isDisplayPowerOff()
{
    if (turnOffWhenSuspended && get_usb_suspended()) {
        if (displayIsPowerOn) setDisplayPower(0);
        return true;
    } else {
        if (!displayIsPowerOn) setDisplayPower(1);
    }

    if (!displaySaverTimeout) return false;

    float diffTime = getMillis() - prevMillis;
    displaySaverTimer -= diffTime;
    if (!!displaySaverTimeout && (gamepad->state.buttons || gamepad->state.dpad)) {
        displaySaverTimer = displaySaverTimeout;
        setDisplayPower(1);
    } else if (!!displaySaverTimeout && displaySaverTimer <= 0) {
        setDisplayPower(0);
    }

    prevMillis = getMillis();

    return (!!displaySaverTimeout && displaySaverTimer <= 0);
}

void DisplayAddon::setDisplayPower(uint8_t status)
{
    if (displayIsPowerOn != status) {
        displayIsPowerOn = status;
        gpDisplay->getDriver()->setPower(status);
    }
}

void DisplayAddon::process() {
    // If GPDisplay is not loaded or we're in standard mode with display power off enabled
    if (gpDisplay->getDriver() == nullptr ||
        (!configMode && isDisplayPowerOff())) {
        return;
    }

    int8_t screenReturn = gpScreen->update();
    gpScreen->draw();

    // -1 = we do not change state
    if (screenReturn >= 0) {
        // Screen wants to change to something else
        if (screenReturn != currDisplayMode) {
            currDisplayMode = (DisplayMode)screenReturn;
            updateDisplayScreen();
        }
    }
}

const DisplayOptions& DisplayAddon::getDisplayOptions() {
    bool configMode = Storage::getInstance().GetConfigMode();
    return configMode ? Storage::getInstance().getPreviewDisplayOptions() : Storage::getInstance().getDisplayOptions();
}

