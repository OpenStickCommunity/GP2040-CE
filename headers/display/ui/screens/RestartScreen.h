#ifndef _RESTARTSCREEN_H_
#define _RESTARTSCREEN_H_

#include "GPGFX_UI_widgets.h"

class RestartScreen : public GPScreen {
    public:
        RestartScreen() {}
        RestartScreen(GPGFX* renderer, uint32_t mode) { setRenderer(renderer); setBootMode(mode); }
        virtual ~RestartScreen() {}
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();

        void setBootMode(uint32_t mode);
    protected:
        virtual void drawScreen();
        uint32_t bootMode;
};

#endif