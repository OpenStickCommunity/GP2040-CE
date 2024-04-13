#ifndef _SPLASHSCREEN_H_
#define _SPLASHSCREEN_H_

#include "GPGFX_UI_widgets.h"
#include "bitmaps.h"

class SplashScreen : public GPScreen {
    public:
        SplashScreen() {}
        SplashScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();
    protected:
        virtual void drawScreen();
        uint16_t prevButtonState = 0;
        uint32_t splashStartTime = 0;
        bool configMode;
};

#endif