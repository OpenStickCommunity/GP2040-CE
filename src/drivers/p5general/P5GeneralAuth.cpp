#include "drivers/p5general/P5GeneralAuth.h"
#include "drivers/p5general/P5GeneralAuthUSBListener.h"
#include "peripheralmanager.h"

void P5GeneralAuth::initialize() {
    if ( !available() ) {
        return;
    }

    p5GeneralAuthData.dongle_ready = false;
    p5GeneralAuthData.passthrough_state = P5GeneralGPAuthState::p5g_auth_idle;
    memset(p5GeneralAuthData.auth_buffer, 0, sizeof(p5GeneralAuthData.auth_buffer));
    listener = new P5GeneralAuthUSBListener();
    ((P5GeneralAuthUSBListener*)listener)->setup();
    ((P5GeneralAuthUSBListener*)listener)->setAuthData(&p5GeneralAuthData);
}

bool P5GeneralAuth::available() {
    return PeripheralManager::getInstance().isUSBEnabled(0);
}

void P5GeneralAuth::process() {
    ((P5GeneralAuthUSBListener*)listener)->process();
}
