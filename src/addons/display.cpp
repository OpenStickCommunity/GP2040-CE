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
#include "drivers/ps4/PS4Driver.h"
#include "drivers/xbone/XBOneDriver.h"
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

    loadedScreens.insert({CONFIG_INSTRUCTION, {new ConfigScreen()}});
    loadedScreens.insert({SPLASH, {new SplashScreen()}});
    loadedScreens.insert({MAIN_MENU, {new MainMenuScreen()}});
    loadedScreens.insert({BUTTONS, {new ButtonLayoutScreen()}});

    gpDisplay = new GPGFX();

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
    }

    gpDisplay->init(gpOptions);

    map<DisplayMode, GPScreen*>::iterator screenIterator;
    for (screenIterator = loadedScreens.begin(); screenIterator != loadedScreens.end(); screenIterator++) {
        screenIterator->second->setRenderer(gpDisplay);
    }

    gamepad = Storage::getInstance().GetGamepad();
    pGamepad = Storage::getInstance().GetProcessedGamepad();

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
    if (!configMode && isDisplayPowerOff()) return;

    gpScreen = loadedScreens.find(currDisplayMode)->second;

    drawStatusBar(gamepad);

    int8_t screenReturn = gpScreen->update();

    gpScreen->draw();

    if (screenReturn >= 0) {
        if (screenReturn != currDisplayMode) {
            // screen changes
            currDisplayMode = (DisplayMode)screenReturn;
        } else {
            // no change
        }
    } else {
        // screen exited
        if (!configMode) {
            currDisplayMode = DisplayMode::BUTTONS;
        } else {
            currDisplayMode = DisplayMode::CONFIG_INSTRUCTION;
        }
    }
}

const DisplayOptions& DisplayAddon::getDisplayOptions() {
    bool configMode = Storage::getInstance().GetConfigMode();
    return configMode ? Storage::getInstance().getPreviewDisplayOptions() : Storage::getInstance().getDisplayOptions();
}

void DisplayAddon::drawStatusBar(Gamepad * gamepad)
{
    const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;

    // Limit to 21 chars with 6x8 font for now
    statusBar.clear();

    switch (gamepad->getOptions().inputMode)
    {
        case INPUT_MODE_HID:    statusBar += "DINPUT"; break;
        case INPUT_MODE_SWITCH: statusBar += "SWITCH"; break;
        case INPUT_MODE_XINPUT: statusBar += "XINPUT"; break;
        case INPUT_MODE_MDMINI: statusBar += "GEN/MD"; break;
        case INPUT_MODE_NEOGEO: statusBar += "NGMINI"; break;
        case INPUT_MODE_PCEMINI: statusBar += "PCE/TG"; break;
        case INPUT_MODE_EGRET: statusBar += "EGRET"; break;
        case INPUT_MODE_ASTRO: statusBar += "ASTRO"; break;
        case INPUT_MODE_PSCLASSIC: statusBar += "PSC"; break;
        case INPUT_MODE_XBOXORIGINAL: statusBar += "OGXBOX"; break;
        case INPUT_MODE_PS4:
            statusBar += "PS4";
            if(((PS4Driver*)DriverManager::getInstance().getDriver())->getAuthSent() == true )
                statusBar += ":AS";
            else
                statusBar += "   ";
            break;
        case INPUT_MODE_PS5:
            statusBar += "PS5";
            if(((PS4Driver*)DriverManager::getInstance().getDriver())->getAuthSent() == true )
                statusBar += ":AS";
            else
                statusBar += "   ";
            break;
        case INPUT_MODE_XBONE:
            statusBar += "XBON";
            if(((XBOneDriver*)DriverManager::getInstance().getDriver())->getAuthSent() == true )
                statusBar += "E";
            else
                statusBar += "*";
            break;
        case INPUT_MODE_KEYBOARD: statusBar += "HID-KB"; break;
        case INPUT_MODE_CONFIG: statusBar += "CONFIG"; break;
    }

    if ( turboOptions.enabled ) {
        statusBar += " T";
        if ( turboOptions.shotCount < 10 ) // padding
            statusBar += "0";
        statusBar += std::to_string(turboOptions.shotCount);
    } else {
        statusBar += "    "; // no turbo, don't show Txx setting
    }
    switch (gamepad->getOptions().dpadMode)
    {
        case DPAD_MODE_DIGITAL:      statusBar += " D"; break;
        case DPAD_MODE_LEFT_ANALOG:  statusBar += " L"; break;
        case DPAD_MODE_RIGHT_ANALOG: statusBar += " R"; break;
    }

    switch (Gamepad::resolveSOCDMode(gamepad->getOptions()))
    {
        case SOCD_MODE_NEUTRAL:               statusBar += " SOCD-N"; break;
        case SOCD_MODE_UP_PRIORITY:           statusBar += " SOCD-U"; break;
        case SOCD_MODE_SECOND_INPUT_PRIORITY: statusBar += " SOCD-L"; break;
        case SOCD_MODE_FIRST_INPUT_PRIORITY:  statusBar += " SOCD-F"; break;
        case SOCD_MODE_BYPASS:                statusBar += " SOCD-X"; break;
    }
    if (Storage::getInstance().getAddonOptions().macroOptions.enabled)
        statusBar += " M";

    gpScreen->header = statusBar;
}
