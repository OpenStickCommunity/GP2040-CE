#include "GPGFX_UI.h"

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
    if (_configMode) {
        return Storage::getInstance().getPreviewDisplayOptions();
    } else {
        return Storage::getInstance().getDisplayOptions();
    }
}

bool GPGFX_UI::getConfigMode() {
    return Storage::getInstance().GetConfigMode();
}

uint16_t GPGFX_UI::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
