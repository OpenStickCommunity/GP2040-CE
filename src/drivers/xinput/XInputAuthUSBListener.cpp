#include "drivers/xinput/XInputAuthUSBListener.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

#include "drivers/shared/xinput_host.h"

void XInputAuthUSBListener::setup() {
}

void XInputAuthUSBListener::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    if ( controllerType == xinput_type_t::XBOX360) {
        xinput_dev_addr = dev_addr;
        xinput_instance = instance;
        mounted = true;
    }
}

void XInputAuthUSBListener::unmount(uint8_t dev_addr) {
    // Do not reset dongle_ready on unmount (Magic-X will remount but still be ready)
    mounted = false;
}

void XInputAuthUSBListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if ( mounted == false )
        return;
}
