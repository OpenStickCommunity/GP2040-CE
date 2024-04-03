#include "MainMenuScreen.h"

extern uint32_t getMillis();

void MainMenuScreen::init() {
    getRenderer()->clearScreen();
}

void MainMenuScreen::shutdown() {
    clearElements();
}

void MainMenuScreen::drawScreen() {
    getRenderer()->drawText(1, 1, "GPGFX_UI Test Menu");

    for (size_t i = 0; i < currentMenu->size(); ++i) {
        MenuEntry entry = currentMenu->at(i);

        getRenderer()->drawText(3, 3+i, entry.label);
    }

/*
    if (!isPressed) {
        if (pressedUp()) {
            if (menuIndex > 0) {
                menuIndex--;
            } else {
                menuIndex = currentMenu->size()-1;
            }
            checkDebounce = getMillis();
            isPressed = true;
        } else if (pressedDown()) {
            if (menuIndex < currentMenu->size()-1) {
                menuIndex++;
            } else {
                menuIndex = 0;
            }
            checkDebounce = getMillis();
            isPressed = true;
        } else if (pressedB1()) {
            currentMenu->at(menuIndex).action();
            checkDebounce = getMillis();
            isPressed = true;
        }
    } else {
        if (isPressed && ((getMillis() - checkDebounce) > 400)) {
            isPressed = false;
        }
    }
*/

    getRenderer()->drawText(1, 3+menuIndex, ">");
}

void MainMenuScreen::setMenu(std::vector<MenuEntry>* menu) {
    currentMenu = menu;
}

int8_t MainMenuScreen::update() {
    uint16_t buttonState = getGamepad()->state.buttons;

    if (prevButtonState && !buttonState) {
        switch (prevButtonState) {
            case (GAMEPAD_MASK_B1):
                if (menuIndex > 0) {
                    menuIndex--;
                } else {
                    menuIndex = currentMenu->size()-1;
                }
                break;
            case (GAMEPAD_MASK_B2):
                if (menuIndex < currentMenu->size()-1) {
                    menuIndex++;
                } else {
                    menuIndex = 0;
                }
                break;
            case (GAMEPAD_MASK_S1):
                currentMenu->at(menuIndex).action();
                break;
            default:
                //prevDisplayMode = DisplayMode::CONFIG_INSTRUCTION;
                break;
        }
    }

    prevButtonState = buttonState;

    return -1;
}

/*
	void testMenu();

	std::vector<MenuEntry> mainMenu = {
		{"Menu 1", NULL, std::bind(&DisplayAddon::testMenu, this)},
		{"Menu 2", NULL, std::bind(&DisplayAddon::testMenu, this)},
		{"Menu 3", NULL, std::bind(&DisplayAddon::testMenu, this)},
		{"Menu 4", NULL, std::bind(&DisplayAddon::testMenu, this)},
		{"Menu 5", NULL, std::bind(&DisplayAddon::testMenu, this)},
		{"Menu 6", NULL, std::bind(&DisplayAddon::testMenu, this)},
		{"Menu 7", NULL, std::bind(&DisplayAddon::testMenu, this)},
		{"Menu 8", NULL, std::bind(&DisplayAddon::testMenu, this)},
	};

	std::vector<MenuEntry>* currentMenu = &mainMenu;
*/