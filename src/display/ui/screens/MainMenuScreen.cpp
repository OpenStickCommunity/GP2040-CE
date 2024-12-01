#include "MainMenuScreen.h"

extern uint32_t getMillis();

void MainMenuScreen::init() {
    //stdio_init_all();

    getRenderer()->clearScreen();
    currentMenu = &mainMenu;

    gpMenu = new GPMenu();
    gpMenu->setRenderer(getRenderer());
    gpMenu->setPosition(8, 16);
    gpMenu->setStrokeColor(1);
    gpMenu->setFillColor(1);
    gpMenu->setMenuSize(18, 5);
    gpMenu->setViewport(this->getViewport());
    gpMenu->setShape(GPShape_Type::GP_SHAPE_SQUARE);
    gpMenu->setMenuData(currentMenu);
    addElement(gpMenu);

    // Setup Reverse Input Button
    mapMenuUp = new GamepadButtonMapping(0);
    mapMenuDown = new GamepadButtonMapping(0);
    mapMenuLeft = new GamepadButtonMapping(0);
    mapMenuRight = new GamepadButtonMapping(0);
    mapMenuSelect = new GamepadButtonMapping(0);
    mapMenuBack = new GamepadButtonMapping(0);
    mapMenuToggle = new GamepadButtonMapping(0);

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
}

void MainMenuScreen::drawScreen() {
    getRenderer()->drawText(0, 0, "012345678901234567890");
    //
    //for (size_t i = 0; i < currentMenu->size(); ++i) {
    //    MenuEntry entry = currentMenu->at(i);
    //
    //    getRenderer()->drawText(3, 3+i, entry.label);
    //}
    //
    //getRenderer()->drawText(1, 3+menuIndex, ">");
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
            if (menuIndex > 0) {
                menuIndex--;
            } else {
                menuIndex = gpMenu->getDataSize()-1;
            }
            isPressed = true;
        } else if (values & mapMenuDown->pinMask) {
            if (menuIndex < gpMenu->getDataSize()-1) {
                menuIndex++;
            } else {
                menuIndex = 0;
            }
            isPressed = true;
        } else if (values & mapMenuSelect->pinMask) {
            currentMenu->at(menuIndex).action();
            isPressed = true;
        }
        gpMenu->setIndex(menuIndex);
    } else {
        isPressed = false;
    }

    if (prevButtonState && !buttonState) {
        switch (prevButtonState) {
            default:
                //prevDisplayMode = DisplayMode::CONFIG_INSTRUCTION;
                break;
        }
    }

    prevButtonState = buttonState;
    prevValues = values;

    return -1;
}

void MainMenuScreen::testMenu() {
    
}
