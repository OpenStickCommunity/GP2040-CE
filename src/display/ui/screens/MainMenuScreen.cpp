#include "MainMenuScreen.h"
#include "hardware/watchdog.h"
#include "system.h"

extern uint32_t getMillis();

void MainMenuScreen::init() {
    getRenderer()->clearScreen();
    currentMenu = &mainMenu;
    previousMenu = nullptr;

    exitToScreen = -1;

    gpMenu = new GPMenu();
    gpMenu->setRenderer(getRenderer());
    gpMenu->setPosition(8, 16);
    gpMenu->setStrokeColor(1);
    gpMenu->setFillColor(1);
    gpMenu->setMenuSize(18, 4);
    gpMenu->setViewport(this->getViewport());
    gpMenu->setShape(GPShape_Type::GP_SHAPE_SQUARE);
    gpMenu->setMenuData(currentMenu);
    gpMenu->setMenuTitle(MAIN_MENU_NAME);
    addElement(gpMenu);

    mapMenuUp = new GamepadButtonMapping(0);
    mapMenuDown = new GamepadButtonMapping(0);
    mapMenuLeft = new GamepadButtonMapping(0);
    mapMenuRight = new GamepadButtonMapping(0);
    mapMenuSelect = new GamepadButtonMapping(0);
    mapMenuBack = new GamepadButtonMapping(0);
    mapMenuToggle = new GamepadButtonMapping(0);

    // populate the profiles menu
    uint8_t profileCount = (sizeof(Storage::getInstance().getProfileOptions().gpioMappingsSets)/sizeof(GpioMappings))+1;
    for (uint8_t profileCtr = 0; profileCtr < profileCount; profileCtr++) {
        std::string menuLabel = "";
        if (profileCtr == 0) {
            menuLabel = Storage::getInstance().getGpioMappings().profileLabel;
        } else {
            menuLabel = Storage::getInstance().getProfileOptions().gpioMappingsSets[profileCtr-1].profileLabel;
        }
        if (menuLabel.empty()) {
            menuLabel = "Profile #" + std::to_string(profileCtr);
        }
        MenuEntry menuEntry = {menuLabel, NULL, nullptr, std::bind(&MainMenuScreen::currentProfile, this), std::bind(&MainMenuScreen::selectProfile, this), profileCtr+1};
        profilesMenu.push_back(menuEntry);
    }

    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        switch (pinMappings[pin].action) {
            case GpioAction::MENU_NAVIGATION_UP: mapMenuUp->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_DOWN: mapMenuDown->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_LEFT: mapMenuLeft->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_RIGHT: mapMenuRight->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_SELECT: mapMenuSelect->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_BACK: mapMenuBack->pinMask |= 1 << pin; break;
            case GpioAction::MENU_NAVIGATION_TOGGLE: mapMenuToggle->pinMask |= 1 << pin; break;
            default:    break;
        }
    }

    changeRequiresReboot = false;
    changeRequiresSave = false;
    prevInputMode = Storage::getInstance().GetGamepad()->getOptions().inputMode;
    updateInputMode = Storage::getInstance().GetGamepad()->getOptions().inputMode;
    
    prevDpadMode = Storage::getInstance().GetGamepad()->getOptions().dpadMode;
    updateDpadMode = Storage::getInstance().GetGamepad()->getOptions().dpadMode;
    
    prevSocdMode = Storage::getInstance().GetGamepad()->getOptions().socdMode;
    updateSocdMode = Storage::getInstance().GetGamepad()->getOptions().socdMode;
    
    prevProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    updateProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    
    prevFocus = Storage::getInstance().getAddonOptions().focusModeOptions.enabled;
    updateFocus = Storage::getInstance().getAddonOptions().focusModeOptions.enabled;
    
    prevTurbo = Storage::getInstance().getAddonOptions().turboOptions.enabled;
    updateTurbo = Storage::getInstance().getAddonOptions().turboOptions.enabled;
}

void MainMenuScreen::shutdown() {
    clearElements();
    exitToScreen = -1;
}

void MainMenuScreen::drawScreen() {
    gpMenu->setVisibility(!screenIsPrompting);

    if (!screenIsPrompting) {

    } else {
        getRenderer()->drawText(1, 1, "Config has changed.");
        if (changeRequiresSave && !changeRequiresReboot) {
            getRenderer()->drawText(3, 3, "Would you like");
            getRenderer()->drawText(6, 4, "to save?");
        } else if (changeRequiresSave && changeRequiresReboot) {
            getRenderer()->drawText(3, 3, "Would you like");
            getRenderer()->drawText(1, 4, "to save & restart?");
        } else {

        }
        
        if (promptChoice) getRenderer()->drawText(5, 6, CHAR_RIGHT);
        getRenderer()->drawText(6, 6, "Yes");
        if (!promptChoice) getRenderer()->drawText(11, 6, CHAR_RIGHT);
        getRenderer()->drawText(12, 6, "No");
    }
}

void MainMenuScreen::setMenu(std::vector<MenuEntry>* menu) {
    currentMenu = menu;
}

int8_t MainMenuScreen::update() {
	Gamepad * gamepad = Storage::getInstance().GetGamepad();
    Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;

    uint16_t buttonState = getGamepad()->state.buttons;

    if (!isPressed && prevValues != values) {
        if (values & mapMenuUp->pinMask) {
            updateMenuNavigation(GpioAction::MENU_NAVIGATION_UP);
        } else if (values & mapMenuDown->pinMask) {
            updateMenuNavigation(GpioAction::MENU_NAVIGATION_DOWN);
        } else if (values & mapMenuSelect->pinMask) {
            updateMenuNavigation(GpioAction::MENU_NAVIGATION_SELECT);
        } else if (values & mapMenuBack->pinMask) {
            updateMenuNavigation(GpioAction::MENU_NAVIGATION_BACK);
        }
    } else {
        isPressed = false;
    }

    prevButtonState = buttonState;
    prevValues = values;

    if ((exitToScreen != -1) && ((changeRequiresSave) || (changeRequiresReboot))) {
        // trying to exit menu but a change requires a save/reboot
        exitToScreenBeforePrompt = exitToScreen;
        exitToScreen = -1;
        screenIsPrompting = true;
    }

    return exitToScreen;
}

void MainMenuScreen::updateMenuNavigation(GpioAction action) {
    bool changeIndex = false;
    uint16_t menuSize = gpMenu->getDataSize();

    switch (action) { 
        case GpioAction::MENU_NAVIGATION_UP:
            if (!screenIsPrompting) {
                if (menuIndex > 0) {
                    menuIndex--;
                } else {
                    menuIndex = menuSize-1;
                }
                changeIndex = true;
            } else {
                promptChoice = !promptChoice;
            }
            isPressed = true;
            break;
        case GpioAction::MENU_NAVIGATION_DOWN:
            if (!screenIsPrompting) {
                if (menuIndex < menuSize-1) {
                    menuIndex++;
                } else {
                    menuIndex = 0;
                }
                changeIndex = true;
            } else {
                promptChoice = !promptChoice;
            }
            isPressed = true;
            break;
        case GpioAction::MENU_NAVIGATION_LEFT:
            if (screenIsPrompting) {
                promptChoice = !promptChoice;
            }
            isPressed = true;
            break;
        case GpioAction::MENU_NAVIGATION_RIGHT:
            if (screenIsPrompting) {
                promptChoice = !promptChoice;
            }
            isPressed = true;
            break;
        case GpioAction::MENU_NAVIGATION_SELECT:
            if (!screenIsPrompting) {
                if (currentMenu->at(menuIndex).submenu != nullptr) {
                    previousMenu = currentMenu;
                    currentMenu = currentMenu->at(menuIndex).submenu;
                    gpMenu->setMenuData(currentMenu);
                    gpMenu->setMenuTitle(previousMenu->at(menuIndex).label);
                    menuIndex = 0;
                    changeIndex = true;
                } else {
                    currentMenu->at(menuIndex).action();
                }
            } else {
                if (promptChoice) {
                    saveOptions();
                } else {
                    resetOptions();
                    exitToScreen = DisplayMode::BUTTONS;
                    exitToScreenBeforePrompt = DisplayMode::BUTTONS;
                    isPressed = false;
                }
            }
            isPressed = true;
            break;
        case GpioAction::MENU_NAVIGATION_BACK:
            if (!screenIsPrompting) {
                if (previousMenu != nullptr) {
                    currentMenu = previousMenu;
                    previousMenu = nullptr;
                    menuIndex = 0;
                    changeIndex = true;
                    gpMenu->setMenuData(currentMenu);
                    gpMenu->setMenuTitle(MAIN_MENU_NAME);
                } else {
                    exitToScreen = DisplayMode::BUTTONS;
                    exitToScreenBeforePrompt = DisplayMode::BUTTONS;
                    isPressed = false;
                }
            } else {
                // back again goes back to the menu
                screenIsPrompting = false;
                isPressed = false;
            }
            isPressed = true;
            break;
        default:
            break;
    }

    if (changeIndex) gpMenu->setIndex(menuIndex);
}

void MainMenuScreen::saveAndExit() {
    saveOptions();
}

int32_t MainMenuScreen::modeValue() {
    return -1;
}

void MainMenuScreen::selectInputMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        InputMode valueToSave = (InputMode)currentMenu->at(menuIndex).optionValue;
        prevInputMode = Storage::getInstance().GetGamepad()->getOptions().inputMode;
        updateInputMode = valueToSave;

        if (prevInputMode != valueToSave) {
            // input mode requires a save and reboot
            changeRequiresReboot = true;
            changeRequiresSave = true;
        }
    }
}

int32_t MainMenuScreen::currentInputMode() {
    return updateInputMode;
}

void MainMenuScreen::selectDPadMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        DpadMode valueToSave = (DpadMode)currentMenu->at(menuIndex).optionValue;
        prevDpadMode = Storage::getInstance().GetGamepad()->getOptions().dpadMode;
        updateDpadMode = valueToSave;

        if (prevDpadMode != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentDpadMode() {
    return updateDpadMode;
}

void MainMenuScreen::selectSOCDMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        SOCDMode valueToSave = (SOCDMode)currentMenu->at(menuIndex).optionValue;
        prevSocdMode = Storage::getInstance().GetGamepad()->getOptions().socdMode;
        updateSocdMode = valueToSave;

        if (prevDpadMode != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentSOCDMode() {
    return updateSocdMode;
}

void MainMenuScreen::resetOptions() {
    if (changeRequiresSave) {
        if (prevInputMode != updateInputMode) updateInputMode = prevInputMode;
        if (prevDpadMode != updateDpadMode) updateDpadMode = prevDpadMode;
        if (prevSocdMode != updateSocdMode) updateSocdMode = prevSocdMode;
        if (prevProfile != updateProfile) updateProfile = prevProfile;
        if (prevFocus != updateFocus) updateFocus = prevFocus;
        if (prevTurbo != updateTurbo) updateTurbo = prevTurbo;
    }

    changeRequiresSave = false;
    changeRequiresReboot = false;
    screenIsPrompting = false;
}

void MainMenuScreen::saveOptions() {
    GamepadOptions& options = Storage::getInstance().getGamepadOptions();

    if (changeRequiresSave) {
        bool saveHasChanged = false;
        if (prevInputMode != updateInputMode) {
            options.inputMode = updateInputMode;
            saveHasChanged = true;
        }
        if (prevDpadMode != updateDpadMode) {
            options.dpadMode = updateDpadMode;
            saveHasChanged = true;
        }
        if (prevSocdMode != updateSocdMode) {
            options.socdMode = updateSocdMode;
            saveHasChanged = true;
        }
        if (prevProfile != updateProfile) {
            options.profileNumber = updateProfile;
            saveHasChanged = true;
        }
        if (prevFocus != updateFocus) {
            Storage::getInstance().getAddonOptions().focusModeOptions.enabled = updateFocus;
            saveHasChanged = true;
        }
        if (prevTurbo != updateTurbo) {
            Storage::getInstance().getAddonOptions().turboOptions.enabled = updateTurbo;
            saveHasChanged = true;
        }

        if (saveHasChanged) {
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true, changeRequiresReboot));
            screenIsPrompting = false;
        }
        changeRequiresSave = false;
        changeRequiresReboot = false;
    }

    if (exitToScreenBeforePrompt != -1) {
        exitToScreen = exitToScreenBeforePrompt;
        exitToScreenBeforePrompt = -1;
    }
}

void MainMenuScreen::selectProfile() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        uint8_t valueToSave = currentMenu->at(menuIndex).optionValue;
        prevProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;
        updateProfile = valueToSave;

        if (prevProfile != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentProfile() {
    return updateProfile;
}

void MainMenuScreen::selectFocusMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        uint8_t valueToSave = currentMenu->at(menuIndex).optionValue;
        prevFocus = Storage::getInstance().getAddonOptions().focusModeOptions.enabled;
        updateFocus = valueToSave;

        if (prevFocus != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentFocusMode() {
    return updateFocus;
}

void MainMenuScreen::selectTurboMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        uint8_t valueToSave = currentMenu->at(menuIndex).optionValue;
        prevTurbo = Storage::getInstance().getAddonOptions().turboOptions.enabled;
        updateTurbo = valueToSave;

        if (updateTurbo != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentTurboMode() {
    return updateTurbo;
}
