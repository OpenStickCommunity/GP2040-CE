#ifndef _RESTARTSCREEN_H_
#define _RESTARTSCREEN_H_

#include "GPGFX_UI_widgets.h"
#include "bitmaps.h"

class RestartScreen : public GPScreen {
    public:
        RestartScreen() {}
        RestartScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();

        void setBootMode(uint32_t mode);
    protected:
        virtual void drawScreen();
        uint32_t bootMode;
};

#endif