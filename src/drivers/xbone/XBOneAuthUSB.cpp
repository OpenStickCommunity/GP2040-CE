#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/xbone/XBOneAuthUSB.h"
#include "drivers/xbone/XBOneAuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

#include "drivers/xbone/XBOneDescriptors.h"
#include "drivers/shared/xgip_protocol.h"
#include "drivers/shared/xinput_host.h"
#include "drivers/shared/xbonedata.h"

void XBOneAuthUSB::initialize() {
    if ( available() ) {
        listener = new XBOneAuthUSBListener();
    }
}

bool XBOneAuthUSB::available() {
	return (PeripheralManager::getInstance().isUSBEnabled(0));
}

void XBOneAuthUSB::process() {
    ((XBOneAuthUSBListener*)listener)->process();
}
