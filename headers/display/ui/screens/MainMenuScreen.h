#ifndef _MAINMENUSCREEN_H_
#define _MAINMENUSCREEN_H_

#include "GPGFX_UI_widgets.h"
#include "GPGFX_UI_types.h"

class MainMenuScreen : public GPScreen {
    public:
        MainMenuScreen() {}
        MainMenuScreen(GPGFX* renderer) { setRenderer(renderer); }
        void setMenu(std::vector<MenuEntry>* menu);
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();

        void testMenu();
    protected:
        virtual void drawScreen();
    private:
        uint8_t menuIndex = 0;
        bool isPressed = false;
        uint32_t checkDebounce;
        std::vector<MenuEntry>* currentMenu;
        uint16_t prevButtonState = 0;
        Mask_t prevValues;
        GPMenu* gpMenu;

        GamepadButtonMapping *mapMenuUp;
        GamepadButtonMapping *mapMenuDown;
        GamepadButtonMapping *mapMenuLeft;
        GamepadButtonMapping *mapMenuRight;
        GamepadButtonMapping *mapMenuSelect;
        GamepadButtonMapping *mapMenuBack;
        GamepadButtonMapping *mapMenuToggle;

        std::vector<MenuEntry> mainMenu = {
            {"Input Mode", NULL, std::bind(&MainMenuScreen::testMenu, this)},
            {"D-Pad Mode", NULL, std::bind(&MainMenuScreen::testMenu, this)},
            {"SOCD Mode", NULL, std::bind(&MainMenuScreen::testMenu, this)},
            {"Focus Mode", NULL, std::bind(&MainMenuScreen::testMenu, this)},
            {"Turbo", NULL, std::bind(&MainMenuScreen::testMenu, this)},
            {"Profile", NULL, std::bind(&MainMenuScreen::testMenu, this)},
            {"Lighting", NULL, std::bind(&MainMenuScreen::testMenu, this)},
        };
};

#endif