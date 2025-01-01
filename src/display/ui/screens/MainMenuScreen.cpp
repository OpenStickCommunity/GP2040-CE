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
        MenuEntry menuEntry = {menuLabel, NULL, nullptr, std::bind(&MainMenuScreen::currentProfile, this), std::bind(&MainMenuScreen::selectProfile, this), profileCtr};
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
}

void MainMenuScreen::shutdown() {
    clearElements();
    exitToScreen = -1;
}

void MainMenuScreen::drawScreen() {
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

    return exitToScreen;
}

void MainMenuScreen::updateMenuNavigation(GpioAction action) {
    bool changeIndex = false;
    uint16_t menuSize = gpMenu->getDataSize();

    switch (action) { 
        case GpioAction::MENU_NAVIGATION_UP:
            if (menuIndex > 0) {
                menuIndex--;
            } else {
                menuIndex = menuSize-1;
            }
            changeIndex = true;
            isPressed = true;
            break;
        case GpioAction::MENU_NAVIGATION_DOWN:
            if (menuIndex < menuSize-1) {
                menuIndex++;
            } else {
                menuIndex = 0;
            }
            changeIndex = true;
            isPressed = true;
            break;
        case GpioAction::MENU_NAVIGATION_LEFT:
            break;
        case GpioAction::MENU_NAVIGATION_RIGHT:
            break;
        case GpioAction::MENU_NAVIGATION_SELECT:
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
            isPressed = true;
            break;
        case GpioAction::MENU_NAVIGATION_BACK:
            if (previousMenu != nullptr) {
                currentMenu = previousMenu;
                previousMenu = nullptr;
                menuIndex = 0;
                changeIndex = true;
                gpMenu->setMenuData(currentMenu);
                gpMenu->setMenuTitle(MAIN_MENU_NAME);
            } else {
                exitToScreen = DisplayMode::BUTTONS;
                isPressed = false;
            }
            isPressed = true;
            break;
        case GpioAction::MENU_NAVIGATION_TOGGLE:
            // when in the menu screen, this exits the menu without confirming changes
            exitToScreen = DisplayMode::BUTTONS;
            isPressed = false;
            break;
        default:
            break;
    }

    if (changeIndex) gpMenu->setIndex(menuIndex);
}

void MainMenuScreen::testMenu() {
    
}

void MainMenuScreen::saveAndExit() {
    saveOptions();
    exitToScreen = DisplayMode::BUTTONS;
}

int32_t MainMenuScreen::modeValue() {
    return -1;
}

void MainMenuScreen::selectInputMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        if (Storage::getInstance().GetGamepad()->getOptions().inputMode != (InputMode)currentMenu->at(menuIndex).optionValue) {
            Storage::getInstance().GetGamepad()->setInputMode((InputMode)currentMenu->at(menuIndex).optionValue);
            saveOptions();
        }
    }
}

int32_t MainMenuScreen::currentInputMode() {
    return Storage::getInstance().getGamepadOptions().inputMode;
}

void MainMenuScreen::selectDPadMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        Storage::getInstance().GetGamepad()->setDpadMode((DpadMode)currentMenu->at(menuIndex).optionValue);
        saveOptions();
    }
}

int32_t MainMenuScreen::currentDpadMode() {
    return Storage::getInstance().getGamepadOptions().dpadMode;
}

void MainMenuScreen::selectSOCDMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        Storage::getInstance().GetGamepad()->setSOCDMode((SOCDMode)currentMenu->at(menuIndex).optionValue);
        saveOptions();
    }
}

int32_t MainMenuScreen::currentSOCDMode() {
    return Storage::getInstance().getGamepadOptions().socdMode;
}

void MainMenuScreen::saveOptions() {
    //Storage::getInstance().save(true);
    //System::reboot(System::BootMode::DEFAULT);
}

void MainMenuScreen::selectProfile() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        Storage::getInstance().setProfile(currentMenu->at(menuIndex).optionValue);
        saveOptions();
    }
}

int32_t MainMenuScreen::currentProfile() {
    return Storage::getInstance().GetGamepad()->getOptions().profileNumber;
}

void MainMenuScreen::selectFocusMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        Storage::getInstance().getAddonOptions().focusModeOptions.enabled = (bool)currentMenu->at(menuIndex).optionValue;
        saveOptions();
    }
}

int32_t MainMenuScreen::currentFocusMode() {
    return Storage::getInstance().getAddonOptions().focusModeOptions.enabled;
}

void MainMenuScreen::selectTurboMode() {
    if (currentMenu->at(menuIndex).optionValue != -1) {
        Storage::getInstance().getAddonOptions().turboOptions.enabled = (bool)currentMenu->at(menuIndex).optionValue;
        saveOptions();
    }
}

int32_t MainMenuScreen::currentTurboMode() {
    return Storage::getInstance().getAddonOptions().turboOptions.enabled;
}
