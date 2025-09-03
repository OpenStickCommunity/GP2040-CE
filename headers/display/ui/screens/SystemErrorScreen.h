#ifndef _SYSTEMERRORSCREEN_H_
#define _SYSTEMERRORSCREEN_H_

#include "GPGFX_UI_widgets.h"

class SystemErrorScreen : public GPScreen {
    public:
        SystemErrorScreen() {}
        SystemErrorScreen(GPGFX* renderer, std::string message) { setRenderer(renderer); this->errorMessage = message; }
        virtual ~SystemErrorScreen() {}
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();

        std::string errorMessage;
    protected:
        virtual void drawScreen();
};

#endif
