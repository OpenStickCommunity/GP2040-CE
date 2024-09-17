#ifndef _ADS1219DEVICE_H_
#define _ADS1219DEVICE_H_

#include <vector>

#include "i2cdevicebase.h"
#include "ADS1219.h"

class ADS1219Device : public ADS1219, public I2CDeviceBase {
    public:
        // Constructor
        ADS1219Device() {}
        ADS1219Device(PeripheralI2C *i2cController, uint8_t addr = 0x40) : ADS1219(i2cController, addr) {}

        std::vector<uint8_t> getDeviceAddresses() const override {
            return {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F};
        }
};

#endif
