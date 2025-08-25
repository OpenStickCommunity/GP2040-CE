#ifndef _NOUSBDETECTED_H_
#define _NOUSBDETECTED_H_

#include "GPGFX_UI_widgets.h"

class NoUSBDetectedScreen : public GPScreen {
    public:
        NoUSBDetectedScreen() {}
        NoUSBDetectedScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual ~NoUSBDetectedScreen() {}
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();

    protected:
        virtual void drawScreen();
};

#endif
