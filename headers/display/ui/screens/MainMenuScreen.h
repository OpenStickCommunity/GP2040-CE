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
    protected:
        virtual void drawScreen();
    private:
        uint8_t menuIndex = 0;
        bool isPressed = false;
        uint32_t checkDebounce;
        std::vector<MenuEntry>* currentMenu;
        uint16_t prevButtonState = 0;
};

#endif