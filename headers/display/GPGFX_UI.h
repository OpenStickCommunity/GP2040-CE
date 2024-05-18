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
        uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
    private:
        GPGFX* _renderer;
        DisplayOptions _displayOptions;
        bool _configMode;
};

#endif
