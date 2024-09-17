#ifndef _WIIEXTDEVICE_H_
#define _WIIEXTDEVICE_H_

#include <vector>

#include "i2cdevicebase.h"
#include <WiiExtension.h>

class WiiExtensionDevice : public WiiExtension, public I2CDeviceBase {
    public:
        // Constructor
        WiiExtensionDevice() {}
        WiiExtensionDevice(PeripheralI2C *i2cController, uint8_t addr = WII_EXTENSION_I2C_ADDR) : WiiExtension(i2cController, addr) {}

        std::vector<uint8_t> getDeviceAddresses() const override {
            return {WII_EXTENSION_I2C_ADDR,WII_MOTIONPLUS_I2C_ADDR};
        }
};

#endif
