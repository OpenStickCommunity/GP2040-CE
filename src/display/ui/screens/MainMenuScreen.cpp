#include "MainMenuScreen.h"
#include "hardware/watchdog.h"
#include "system.h"
#include "GPRestartEvent.h"

// matches TURBO_SHOT_MIN/MAX in turbo.cpp
#define MENU_TURBO_SHOT_MIN 2
#define MENU_TURBO_SHOT_MAX 30

extern uint32_t getMillis();

void MainMenuScreen::init() {
    getRenderer()->clearScreen();
    currentMenu = &mainMenu;
    menuStack.clear();
    currentTitle = MAIN_MENU_NAME;

    exitToScreen = -1;

    gpMenu = new GPMenu();
    gpMenu->setRenderer(getRenderer());
    gpMenu->setPosition(8, 16);
    gpMenu->setStrokeColor(1);
    gpMenu->setFillColor(1);
    gpMenu->setMenuSize(18, menuLineSize);
    gpMenu->setViewport(this->getViewport());
    gpMenu->setShape(GPShape_Type::GP_SHAPE_SQUARE);
    gpMenu->setMenuData(currentMenu);
    gpMenu->setMenuTitle(MAIN_MENU_NAME);
    addElement(gpMenu);

    mapMenuUp = new GamepadButtonMapping(GAMEPAD_MASK_UP);
    mapMenuDown = new GamepadButtonMapping(GAMEPAD_MASK_DOWN);
    mapMenuLeft = new GamepadButtonMapping(GAMEPAD_MASK_LEFT);
    mapMenuRight = new GamepadButtonMapping(GAMEPAD_MASK_RIGHT);
    mapMenuSelect = new GamepadButtonMapping(GAMEPAD_MASK_B1);
    mapMenuBack = new GamepadButtonMapping(GAMEPAD_MASK_B2);
    mapMenuToggle = new GamepadButtonMapping(0);

    // populate the profiles menu with profile 1 plus every enabled extra profile
    const ProfileOptions& profileOptions = Storage::getInstance().getProfileOptions();
    uint8_t profileCount = profileOptions.gpioMappingsSets_count + 1;
    for (uint8_t profileCtr = 0; profileCtr < profileCount; profileCtr++) {
        std::string menuLabel = "";
        if (profileCtr == 0) {
            menuLabel = Storage::getInstance().getGpioMappings().profileLabel;
        } else {
            if (!profileOptions.gpioMappingsSets[profileCtr-1].enabled) {
                continue;
            }
            menuLabel = profileOptions.gpioMappingsSets[profileCtr-1].profileLabel;
        }
        if (menuLabel.empty()) {
            menuLabel = "Profile #" + std::to_string(profileCtr+1);
        }

        MenuEntry menuEntry = {menuLabel, NULL, nullptr, std::bind(&MainMenuScreen::currentProfile, this), std::bind(&MainMenuScreen::selectProfile, this), profileCtr+1};
        profilesMenu.push_back(menuEntry);
    }

    socdModeMenu.clear();
    const GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();
    for (MenuEntry menuEntry : std::vector<MenuEntry>{ SOCDMode_VALUELIST(SOCD_MODE_ENTRIES) }) {
        if (Gamepad::isAdvancedSOCDMode((SOCDMode)menuEntry.optionValue)) {
            const uint8_t slotIndex = menuEntry.optionValue - SOCD_MODE_ADVANCED_1;
            if (!gamepadOptions.advancedSOCDEnabled || slotIndex >= gamepadOptions.advancedSOCDSlots_count ||
                    !gamepadOptions.advancedSOCDSlots[slotIndex].enabled)
                continue;
            if (gamepadOptions.advancedSOCDSlots[slotIndex].name[0] != '\0')
                menuEntry.label = gamepadOptions.advancedSOCDSlots[slotIndex].name;
        }
        socdModeMenu.push_back(menuEntry);
    }

    GpioMappingInfo* pinMappings = Storage::getInstance().getProfilePinMappings();
    for (Pin_t pin = 0; pin < (Pin_t)NUM_BANK0_GPIOS; pin++) {
        switch (pinMappings[pin].action) {
            case GpioAction::MENU_NAVIGATION_UP: mapMenuUp->pinMask |= Mask_t{1} << pin; break;
            case GpioAction::MENU_NAVIGATION_DOWN: mapMenuDown->pinMask |= Mask_t{1} << pin; break;
            case GpioAction::MENU_NAVIGATION_LEFT: mapMenuLeft->pinMask |= Mask_t{1} << pin; break;
            case GpioAction::MENU_NAVIGATION_RIGHT: mapMenuRight->pinMask |= Mask_t{1} << pin; break;
            case GpioAction::MENU_NAVIGATION_SELECT: mapMenuSelect->pinMask |= Mask_t{1} << pin; break;
            case GpioAction::MENU_NAVIGATION_BACK: mapMenuBack->pinMask |= Mask_t{1} << pin; break;
            case GpioAction::MENU_NAVIGATION_TOGGLE: mapMenuToggle->pinMask |= Mask_t{1} << pin; break;
            default:    break;
        }
    }

    changeRequiresReboot = false;
    changeRequiresSave = false;
    prevInputMode = Storage::getInstance().GetGamepad()->getOptions().inputMode;
    updateInputMode = Storage::getInstance().GetGamepad()->getOptions().inputMode;
    
    prevDpadMode = Storage::getInstance().GetGamepad()->getOptions().dpadMode;
    updateDpadMode = Storage::getInstance().GetGamepad()->getOptions().dpadMode;
    
    prevSocdMode = Storage::getInstance().GetGamepad()->getStoredSOCDMode();
    updateSocdMode = Storage::getInstance().GetGamepad()->getStoredSOCDMode();
    
    prevProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    updateProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;
    
    GamepadOptions& options = Storage::getInstance().getGamepadOptions();
    AddonOptions& addonOptions = Storage::getInstance().getAddonOptions();

    prevSwitchProType = options.switchProControllerType;
    updateSwitchProType = options.switchProControllerType;
    prevSwitch2ProIdentity = options.switch2ProIdentity;
    updateSwitch2ProIdentity = options.switch2ProIdentity;

    // Focus mode on/off is the override (same as the hotkey), it takes effect without a reboot.
    // The add-on itself only loads at boot, so only offer it when the add-on is enabled.
    setupToggle(focusToggle, &addonOptions.focusModeOptions.overrideEnabled, false);
    // The turbo add-on only loads at boot, so turning it on or off needs a restart
    setupToggle(turboToggle, &addonOptions.turboOptions.enabled, true);
    setupToggle(fourWayToggle, &options.fourWayMode, false);
    setupToggle(ddiFourWayToggle, &addonOptions.dualDirectionalOptions.fourWayMode, false);
    setupToggle(invertXToggle, &options.invertXAxis, false);
    setupToggle(invertYToggle, &options.invertYAxis, false);

    focusModeMenu = makeToggleMenu(&focusToggle);
    turboModeMenu = makeToggleMenu(&turboToggle);
    fourWayMenu = makeToggleMenu(&fourWayToggle);
    ddiFourWayMenu = makeToggleMenu(&ddiFourWayToggle);
    invertXMenu = makeToggleMenu(&invertXToggle);
    invertYMenu = makeToggleMenu(&invertYToggle);

    prevTurboSpeed = addonOptions.turboOptions.shotCount;
    updateTurboSpeed = addonOptions.turboOptions.shotCount;
    turboSpeedMenu.clear();
    for (int32_t shots = MENU_TURBO_SHOT_MIN; shots <= MENU_TURBO_SHOT_MAX; shots++) {
        turboSpeedMenu.push_back({std::to_string(shots) + " shots/sec", NULL, nullptr, [this] { return currentTurboSpeed(); }, [this] { selectTurboSpeed(); }, shots});
    }

    rebootMenu = {
        {"Web Config", NULL, nullptr, [this] { return modeValue(); }, [this] { rebootTo(System::BootMode::WEBCONFIG); }},
        {"BOOTSEL",    NULL, nullptr, [this] { return modeValue(); }, [this] { rebootTo(System::BootMode::USB); }},
    };

    buildInputModeMenus();

    mainMenu.clear();
    addSubmenu("Input Mode", &inputModeMenu);
    addSubmenu("D-Pad Mode", &dpadModeMenu);
    addSubmenu("SOCD Mode", &socdModeMenu);
    addSubmenu("Profile", &profilesMenu);
    addSubmenu("4-Way Mode", &fourWayMenu);
    if (addonOptions.dualDirectionalOptions.enabled)
        addSubmenu("DDI 4-Way Mode", &ddiFourWayMenu);
    addSubmenu("Invert X Axis", &invertXMenu);
    addSubmenu("Invert Y Axis", &invertYMenu);
    if (addonOptions.focusModeOptions.enabled)
        addSubmenu("Focus Mode", &focusModeMenu);
    addSubmenu("Turbo", &turboModeMenu);
    if (addonOptions.turboOptions.enabled)
        addSubmenu("Turbo Speed", &turboSpeedMenu);
    addSubmenu("Reboot", &rebootMenu);
    addSubmenu("Exit", &saveMenu);

    setMenuHome();
}

void MainMenuScreen::shutdown() {
    clearElements();
    delete mapMenuUp;
    delete mapMenuDown;
    delete mapMenuLeft;
    delete mapMenuRight;
    delete mapMenuSelect;
    delete mapMenuBack;
    delete mapMenuToggle;
    exitToScreen = -1;
}

void MainMenuScreen::drawScreen() {
    if (gpMenu == nullptr) return;
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

void MainMenuScreen::setMenuHome() {
    currentMenu = &mainMenu;
    menuStack.clear();
    currentTitle = MAIN_MENU_NAME;

    exitToScreen = -1;
    prevValues = Storage::getInstance().GetGamepad()->debouncedGpio;
    isMenuReady = true;
}

int8_t MainMenuScreen::update() {
    if (isMenuReady) {
        GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
        Mask_t values = Storage::getInstance().GetGamepad()->debouncedGpio;
        uint16_t buttonState = getGamepad()->state.buttons;
        uint8_t dpadState = getGamepad()->state.dpad;

        if (prevValues != values) {
            if (values & mapMenuUp->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_UP);
            else if (values & mapMenuDown->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_DOWN);
            else if (values & mapMenuLeft->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_LEFT);
            else if (values & mapMenuRight->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_RIGHT);
            else if (values & mapMenuSelect->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_SELECT);
            else if (values & mapMenuBack->pinMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_BACK);
            else if (values & mapMenuToggle->pinMask) {
                // Menu toggle will always exit out of main menu
                exitToScreen = DisplayMode::BUTTONS;
                exitToScreenBeforePrompt = DisplayMode::BUTTONS;
            }
        }
        if (gamepadOptions.miniMenuGamepadInput == true ) {
            if (prevDpadState != dpadState ) {
                if (dpadState == mapMenuUp->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_UP);
                else if (dpadState == mapMenuDown->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_DOWN);
                else if (dpadState == mapMenuLeft->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_LEFT);
                else if (dpadState == mapMenuRight->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_RIGHT);
            }
            if ( prevButtonState != buttonState ) {
                if (buttonState == mapMenuSelect->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_SELECT);
                else if (buttonState == mapMenuBack->buttonMask) updateMenuNavigation(GpioAction::MENU_NAVIGATION_BACK);
            }
        }

        prevButtonState = buttonState;
        prevDpadState = dpadState;
        prevValues = values;

        // Core0 Event Navigations
        if (eventAction != GpioAction::NONE) {
            updateMenuNavigation(eventAction);
            eventAction = GpioAction::NONE;
        }

        if ((exitToScreen != -1) && ((changeRequiresSave) || (changeRequiresReboot))) {
            // trying to exit menu but a change requires a save/reboot
            exitToScreenBeforePrompt = exitToScreen;
            exitToScreen = -1;
            screenIsPrompting = true;
        }

        return exitToScreen;
    } else {
        return -1;
    }
}

void MainMenuScreen::updateEventMenuNavigation(GpioAction action) {
    eventAction = action;
}

void MainMenuScreen::updateMenuNavigation(GpioAction action) {
    if (gpMenu == nullptr)
        return;

    uint16_t menuIndex = gpMenu->getIndex();
    uint16_t menuSize = gpMenu->getDataSize();

    if (isMenuReady) {
        if (screenIsPrompting) {
            switch(action) {
                case GpioAction::MENU_NAVIGATION_UP:
                case GpioAction::MENU_NAVIGATION_DOWN:
                case GpioAction::MENU_NAVIGATION_LEFT:
                case GpioAction::MENU_NAVIGATION_RIGHT:
                    promptChoice = !promptChoice;
                    break;
                case GpioAction::MENU_NAVIGATION_SELECT:
                    if (promptChoice) {
                        saveOptions();
                    } else {
                        resetOptions();
                        exitToScreen = DisplayMode::BUTTONS;
                        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_BACK:
                    // back again goes back to the menu
                    screenIsPrompting = false;
                    break;
                default:
                    break;
            }
        } else {
            switch (action) { 
                case GpioAction::MENU_NAVIGATION_UP:
                    if ( menuIndex == 0 ) {
                        gpMenu->setIndex(menuSize-1);
                    } else {
                        gpMenu->setIndex(menuIndex-1);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_DOWN:
                    if (menuIndex < menuSize-1) {
                        gpMenu->setIndex(menuIndex+1);
                    } else {
                        gpMenu->setIndex(0);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_LEFT:
                    if ((menuIndex-menuLineSize) > 0) {
                        gpMenu->setIndex(menuIndex - menuLineSize);
                    } else {
                        gpMenu->setIndex(0);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_RIGHT:
                    if ((menuIndex+menuLineSize) < (menuSize-1)) {
                        gpMenu->setIndex(menuIndex + menuLineSize);
                    } else {
                        gpMenu->setIndex(menuSize-1);
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_SELECT:
                    if (currentMenu->at(menuIndex).submenu != nullptr) {
                        menuStack.push_back({currentMenu, currentTitle, menuIndex});
                        showMenu(currentMenu->at(menuIndex).submenu, currentMenu->at(menuIndex).label, 0);
                    } else {
                        currentMenu->at(menuIndex).action();
                    }
                    break;
                case GpioAction::MENU_NAVIGATION_BACK:
                    if (!menuStack.empty()) {
                        MenuFrame frame = menuStack.back();
                        menuStack.pop_back();
                        showMenu(frame.menu, frame.title, frame.index);
                    } else {
                        exitToScreen = DisplayMode::BUTTONS;
                        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
                    }
                    break;
                default:
                    break;
            }
        }
    }
}

void MainMenuScreen::showMenu(std::vector<MenuEntry>* menu, const std::string& title, uint16_t index) {
    currentMenu = menu;
    currentTitle = title;
    gpMenu->setMenuData(currentMenu);
    gpMenu->setMenuTitle(currentTitle);
    gpMenu->setIndex(index);
}

void MainMenuScreen::chooseAndReturn() {
    // after picking an option, go back to the main menu on the entry we came from
    if (!menuStack.empty()) {
        MenuFrame frame = menuStack.front();
        menuStack.clear();
        showMenu(frame.menu, frame.title, frame.index);
    } else {
        exitToScreen = DisplayMode::BUTTONS;
        exitToScreenBeforePrompt = DisplayMode::BUTTONS;
        isPressed = false;
    }
}

void MainMenuScreen::saveAndExit() {
    saveOptions();
    exitToScreen = DisplayMode::BUTTONS;
}

void MainMenuScreen::exitOnly() {
    exitToScreen = DisplayMode::BUTTONS;
}

int32_t MainMenuScreen::modeValue() {
    return -1;
}

void MainMenuScreen::selectInputMode() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        InputMode valueToSave = (InputMode)currentMenu->at(gpMenu->getIndex()).optionValue;
        prevInputMode = Storage::getInstance().GetGamepad()->getOptions().inputMode;
        updateInputMode = valueToSave;

        chooseAndReturn();

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
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        DpadMode valueToSave = (DpadMode)currentMenu->at(gpMenu->getIndex()).optionValue;
        prevDpadMode = Storage::getInstance().GetGamepad()->getOptions().dpadMode;
        updateDpadMode = valueToSave;

        chooseAndReturn();

        if (prevDpadMode != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentDpadMode() {
    return updateDpadMode;
}

void MainMenuScreen::selectSOCDMode() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        SOCDMode valueToSave = (SOCDMode)currentMenu->at(gpMenu->getIndex()).optionValue;
        prevSocdMode = Storage::getInstance().GetGamepad()->getStoredSOCDMode();
        updateSocdMode = valueToSave;

        chooseAndReturn();

        if (prevSocdMode != valueToSave) changeRequiresSave = true;
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
        for (PendingToggle* toggle : toggles) toggle->update = toggle->prev;
        updateTurboSpeed = prevTurboSpeed;
        updateSwitchProType = prevSwitchProType;
        updateSwitch2ProIdentity = prevSwitch2ProIdentity;
    }

    changeRequiresSave = false;
    changeRequiresReboot = false;
    screenIsPrompting = false;
}

void MainMenuScreen::saveOptions() {
    GamepadOptions& options = Storage::getInstance().getGamepadOptions();
    AddonOptions& addonOptions = Storage::getInstance().getAddonOptions();

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
            Storage::getInstance().GetGamepad()->setSOCDMode(updateSocdMode);
            saveHasChanged = true;
        }
        if (prevProfile != updateProfile) {
            options.profileNumber = updateProfile;
            saveHasChanged = true;
        }
        for (PendingToggle* toggle : toggles) {
            if (toggle->target != nullptr && toggle->prev != toggle->update) {
                *toggle->target = toggle->update;
                saveHasChanged = true;
            }
        }
        if (prevSwitchProType != updateSwitchProType) {
            options.switchProControllerType = updateSwitchProType;
            saveHasChanged = true;
        }
        if (prevSwitch2ProIdentity != updateSwitch2ProIdentity) {
            options.switch2ProIdentity = updateSwitch2ProIdentity;
            saveHasChanged = true;
        }
        if (prevTurboSpeed != updateTurboSpeed) {
            addonOptions.turboOptions.shotCount = updateTurboSpeed;
            saveHasChanged = true;
        }

        if (saveHasChanged) {
            EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true, changeRequiresReboot));
        }
        changeRequiresSave = false;
        changeRequiresReboot = false;
    }

    screenIsPrompting = false;

    if (exitToScreenBeforePrompt != -1) {
        exitToScreen = exitToScreenBeforePrompt;
        exitToScreenBeforePrompt = -1;
    }
}

void MainMenuScreen::selectProfile() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        uint8_t valueToSave = currentMenu->at(gpMenu->getIndex()).optionValue;
        prevProfile = Storage::getInstance().GetGamepad()->getOptions().profileNumber;
        updateProfile = valueToSave;

        chooseAndReturn();

        if (prevProfile != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentProfile() {
    return updateProfile;
}

// every input mode the menu can offer, minus the ones excluded in the header
static const struct InputModeEntry { const char* label; int32_t value; } inputModeEntries[] = {
    InputMode_VALUELIST(INPUT_MODE_ENTRIES)
};

// same groups and order as the web configurator (www/src/Data/InputBootModes.ts)
static const struct InputModeGroup { const char* label; std::vector<InputMode> modes; } inputModeGroups[] = {
    {"Primary",      {INPUT_MODE_XINPUT, INPUT_MODE_KEYBOARD, INPUT_MODE_GENERIC, INPUT_MODE_SINPUT}},
    {"Sony",         {INPUT_MODE_PS3, INPUT_MODE_PS4, INPUT_MODE_PS5, INPUT_MODE_P5GENERAL}},
    {"Nintendo",     {INPUT_MODE_SWITCH, INPUT_MODE_SWITCH_PRO, INPUT_MODE_SWITCH2_PRO}},
    {"Microsoft",    {INPUT_MODE_XBONE, INPUT_MODE_XBOXORIGINAL}},
    {"Mini/Classic", {INPUT_MODE_MDMINI, INPUT_MODE_NEOGEO, INPUT_MODE_PCEMINI, INPUT_MODE_EGRET, INPUT_MODE_ASTRO, INPUT_MODE_PSCLASSIC}},
};
static_assert(sizeof(inputModeGroups) / sizeof(inputModeGroups[0]) == MainMenuScreen::INPUT_MODE_GROUP_COUNT, "input mode group count mismatch");

static int8_t findInputModeGroup(int32_t mode) {
    for (uint8_t group = 0; group < MainMenuScreen::INPUT_MODE_GROUP_COUNT; group++) {
        for (InputMode groupMode : inputModeGroups[group].modes) {
            if (groupMode == mode) return group;
        }
    }
    return -1;
}

// same names and order as the web configurator
static const struct { const char* label; SwitchProControllerType value; } switchProTypes[] = {
    {"Pro Controller",    SWITCH_PRO_CONTROLLER_TYPE_PRO},
    {"NSO SNES",          SWITCH_PRO_CONTROLLER_TYPE_SNES},
    {"NSO Super Famicom", SWITCH_PRO_CONTROLLER_TYPE_SUPER_FAMICOM},
    {"NSO N64",           SWITCH_PRO_CONTROLLER_TYPE_N64},
    {"NSO Genesis",       SWITCH_PRO_CONTROLLER_TYPE_GENESIS},
    {"NSO Mega Drive",    SWITCH_PRO_CONTROLLER_TYPE_MEGA_DRIVE},
    {"NSO NES",           SWITCH_PRO_CONTROLLER_TYPE_NES},
    {"NSO Famicom I",     SWITCH_PRO_CONTROLLER_TYPE_FAMICOM_I},
    {"NSO Famicom II",    SWITCH_PRO_CONTROLLER_TYPE_FAMICOM_II},
    {"Joy-Con (L)",       SWITCH_PRO_CONTROLLER_TYPE_JOYCON_LEFT},
    {"Joy-Con (R)",       SWITCH_PRO_CONTROLLER_TYPE_JOYCON_RIGHT},
    {"Joy-Con (L + R)",   SWITCH_PRO_CONTROLLER_TYPE_JOYCON_PAIR},
};

static const struct { const char* label; Switch2ProIdentity value; } switch2ProIdentities[] = {
    {"Pro Controller",    SWITCH2_PRO_IDENTITY_PRO},
    {"GameCube",          SWITCH2_PRO_IDENTITY_GAMECUBE},
    {"Joy-Con 2 (L)",     SWITCH2_PRO_IDENTITY_JOYCON2_LEFT},
    {"Joy-Con 2 (R)",     SWITCH2_PRO_IDENTITY_JOYCON2_RIGHT},
};

void MainMenuScreen::buildInputModeMenus() {
    switchProTypeMenu.clear();
    for (const auto& type : switchProTypes) {
        switchProTypeMenu.push_back({type.label, NULL, nullptr,
            [this] { return updateInputMode == INPUT_MODE_SWITCH_PRO ? (int32_t)updateSwitchProType : -2; },
            [this] { selectSwitchProType(); }, type.value});
    }
    switch2ProIdentityMenu.clear();
    for (const auto& identity : switch2ProIdentities) {
        switch2ProIdentityMenu.push_back({identity.label, NULL, nullptr,
            [this] { return updateInputMode == INPUT_MODE_SWITCH2_PRO ? (int32_t)updateSwitch2ProIdentity : -2; },
            [this] { selectSwitch2ProIdentity(); }, identity.value});
    }

    inputModeMenu.clear();
    for (uint8_t group = 0; group < INPUT_MODE_GROUP_COUNT; group++) {
        inputModeGroupMenus[group].clear();
        for (InputMode mode : inputModeGroups[group].modes) {
            for (const InputModeEntry& entry : inputModeEntries) {
                if (entry.value == mode) {
                    // modes with controller types open a submenu instead of selecting directly
                    std::vector<MenuEntry>* submenu = nullptr;
                    if (mode == INPUT_MODE_SWITCH_PRO) submenu = &switchProTypeMenu;
                    else if (mode == INPUT_MODE_SWITCH2_PRO) submenu = &switch2ProIdentityMenu;
                    inputModeGroupMenus[group].push_back({entry.label, NULL, submenu, [this] { return currentInputMode(); }, [this] { selectInputMode(); }, entry.value});
                    break;
                }
            }
        }
    }

    // a mode that isn't in a group yet still shows up, at the end of Primary
    for (const InputModeEntry& entry : inputModeEntries) {
        if (findInputModeGroup(entry.value) == -1) {
            inputModeGroupMenus[0].push_back({entry.label, NULL, nullptr, [this] { return currentInputMode(); }, [this] { selectInputMode(); }, entry.value});
        }
    }

    for (uint8_t group = 0; group < INPUT_MODE_GROUP_COUNT; group++) {
        if (inputModeGroupMenus[group].empty()) continue;
        inputModeMenu.push_back({inputModeGroups[group].label, NULL, &inputModeGroupMenus[group], [this] { return currentInputModeGroup(); }, [this] { testMenu(); }, group});
    }
}

void MainMenuScreen::selectSwitchProType() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue < 0) return;
    updateInputMode = INPUT_MODE_SWITCH_PRO;
    updateSwitchProType = (SwitchProControllerType)currentMenu->at(gpMenu->getIndex()).optionValue;

    chooseAndReturn();

    if (prevInputMode != updateInputMode || prevSwitchProType != updateSwitchProType) {
        changeRequiresReboot = true;
        changeRequiresSave = true;
    }
}

void MainMenuScreen::selectSwitch2ProIdentity() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue < 0) return;
    updateInputMode = INPUT_MODE_SWITCH2_PRO;
    updateSwitch2ProIdentity = (Switch2ProIdentity)currentMenu->at(gpMenu->getIndex()).optionValue;

    chooseAndReturn();

    if (prevInputMode != updateInputMode || prevSwitch2ProIdentity != updateSwitch2ProIdentity) {
        changeRequiresReboot = true;
        changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentInputModeGroup() {
    int8_t group = findInputModeGroup(updateInputMode);
    // modes without a group are listed under Primary
    return group == -1 ? 0 : group;
}

void MainMenuScreen::addSubmenu(const char* label, std::vector<MenuEntry>* submenu) {
    mainMenu.push_back({label, NULL, submenu, [this] { return modeValue(); }, [this] { testMenu(); }});
}

void MainMenuScreen::setupToggle(PendingToggle& toggle, bool* target, bool rebootOnChange) {
    toggle.target = target;
    toggle.prev = *target;
    toggle.update = *target;
    toggle.rebootOnChange = rebootOnChange;
}

std::vector<MenuEntry> MainMenuScreen::makeToggleMenu(PendingToggle* toggle) {
    return std::vector<MenuEntry>{
        {"Off", NULL, nullptr, [toggle] { return (int32_t)toggle->update; }, [this, toggle] { selectToggle(toggle); }, 0},
        {"On",  NULL, nullptr, [toggle] { return (int32_t)toggle->update; }, [this, toggle] { selectToggle(toggle); }, 1},
    };
}

void MainMenuScreen::selectToggle(PendingToggle* toggle) {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        bool valueToSave = (bool)currentMenu->at(gpMenu->getIndex()).optionValue;
        toggle->update = valueToSave;

        chooseAndReturn();

        if (toggle->prev != valueToSave) {
            changeRequiresSave = true;
            if (toggle->rebootOnChange) changeRequiresReboot = true;
        }
    }
}

void MainMenuScreen::selectTurboSpeed() {
    if (currentMenu->at(gpMenu->getIndex()).optionValue != -1) {
        uint32_t valueToSave = currentMenu->at(gpMenu->getIndex()).optionValue;
        updateTurboSpeed = valueToSave;

        chooseAndReturn();

        if (prevTurboSpeed != valueToSave) changeRequiresSave = true;
    }
}

int32_t MainMenuScreen::currentTurboSpeed() {
    return updateTurboSpeed;
}

void MainMenuScreen::rebootTo(System::BootMode mode) {
    // unsaved menu changes are dropped, same as rebooting from a hotkey
    EventManager::getInstance().triggerEvent(new GPRestartEvent(mode));
}
