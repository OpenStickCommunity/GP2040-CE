#ifndef _CONFIGSCREEN_H_
#define _CONFIGSCREEN_H_

#include "GPGFX_UI_widgets.h"

class ConfigScreen : public GPScreen {
    public:
        ConfigScreen() {}
        ConfigScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual ~ConfigScreen(){}
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();
        uint8_t getDisplayMode() { return prevDisplayMode; }
    protected:
        virtual void drawScreen();
        uint16_t prevButtonState = 0;
        int8_t prevDisplayMode = 0;
        int8_t currDisplayMode = 0;
        std::string version;
};

#endif