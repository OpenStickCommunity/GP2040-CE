#ifndef _GPGFX_UI_H_
#define _GPGFX_UI_H_

#include <string>
#include <math.h>

#include "GPGFX_core.h"
#include "GPGFX.h"
#include "GPGFX_UI_types.h"
#include "config.pb.h"
#include "GamepadState.h"
#include "storagemanager.h"
#include "eventmanager.h"

class GPGFX_UI {
    public:
        GPGFX_UI();
        GPGFX_UI(GPGFX* renderer) { setRenderer(renderer); }
        void setRenderer(GPGFX* renderer) { _renderer = renderer; }
        GPGFX* getRenderer() { return _renderer; }
        Gamepad* getGamepad();
        Gamepad* getProcessedGamepad();
        DisplayOptions getDisplayOptions();
    private:
        GPGFX* _renderer;
        DisplayOptions _displayOptions;
        bool _configMode;
};

#endif
