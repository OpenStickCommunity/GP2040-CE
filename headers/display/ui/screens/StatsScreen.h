#ifndef _STATSSCREEN_H_
#define _STATSSCREEN_H_

#include "GPGFX_UI_widgets.h"

class StatsScreen : public GPScreen {
    public:
        StatsScreen() {}
        StatsScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual ~StatsScreen(){}
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();
    protected:
        virtual void drawScreen();
        uint16_t prevButtonState = 0;
};

#endif