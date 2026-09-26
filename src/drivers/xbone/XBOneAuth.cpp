#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/xbone/XBOneAuth.h"
#include "drivers/xbone/XBOneAuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

#include "drivers/xbone/XBOneDescriptors.h"
#include "drivers/shared/xgip_protocol.h"
#include "drivers/shared/xinput_host.h"

void XBOneAuth::initialize() {
    if ( available() ) {
        listener = new XBOneAuthUSBListener();
        xboxOneAuthData.xboneState = GPAuthState::auth_idle_state;
        xboxOneAuthData.authCompleted = false;
        xboxOneAuthData.auth_passthrough_enabled = false;
        xboxOneAuthData.auth_passthrough = false;
        queue_init(&xboxOneAuthData.relayToDevice, sizeof(XBOneRelayPacket), XBONE_RELAY_QUEUE_DEPTH);
        queue_init(&xboxOneAuthData.relayToConsole, sizeof(XBOneRelayPacket), XBONE_RELAY_QUEUE_DEPTH);
        xboxOneAuthData.relayDropped = 0;
        ((XBOneAuthUSBListener*)listener)->setup();
        ((XBOneAuthUSBListener*)listener)->setAuthData(&xboxOneAuthData);
    }
}

bool XBOneAuth::available() {
    return PeripheralManager::getInstance().isUSBEnabled(0);
}

void XBOneAuth::process() {
    ((XBOneAuthUSBListener*)listener)->process();
}

void XBOneAuth::processHost() {
    ((XBOneAuthUSBListener*)listener)->processHost();
}
