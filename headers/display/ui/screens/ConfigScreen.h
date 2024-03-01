#ifndef _CONFIGSCREEN_H_
#define _CONFIGSCREEN_H_

#include "GPGFX_UI_widgets.h"

class ConfigScreen : public GPScreen {
    public:
        ConfigScreen() {}
        ConfigScreen(GPGFX* renderer) { setRenderer(renderer); }

        int8_t update();
        uint8_t getDisplayMode() { return prevDisplayMode; }
    protected:
        void drawScreen();

        uint16_t prevButtonState = 0;
        int8_t prevDisplayMode = 0;
        int8_t currDisplayMode = 0;
};

#endif