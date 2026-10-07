#include "GPGFX_UI.h"

#include "drivermanager.h"

#include <cstring>

GPGFX_UI::GPGFX_UI() {
}

Gamepad* GPGFX_UI::getGamepad() { 
    return Storage::getInstance().GetGamepad();
}

Gamepad* GPGFX_UI::getProcessedGamepad() { 
    return Storage::getInstance().GetProcessedGamepad();
}

DisplayOptions GPGFX_UI::getDisplayOptions() {
    return Storage::getInstance().getDisplayOptions();
}
