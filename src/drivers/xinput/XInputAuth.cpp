#include "drivers/xinput/XInputAuth.h"
#include "drivers/xinput/XInputAuthUSBListener.h"
#include "peripheralmanager.h"

void XInputAuth::initialize() {
    if ( available() ) {
        listener = new XInputAuthUSBListener();
    }
}

bool XInputAuth::available() {
    return PeripheralManager::getInstance().isUSBEnabled(0);
}

void XInputAuth::process() {

}