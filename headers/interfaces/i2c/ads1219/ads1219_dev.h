#ifndef _ADS1219DEVICE_H_
#define _ADS1219DEVICE_H_

#include <vector>

#include "i2cdevicebase.h"
#include "ads1219.h"

class ADS1219Device : public ADS1219, public I2CDeviceBase {
    public:
        // Constructor
        ADS1219Device(PeripheralI2C *i2cController, uint8_t addr = 0x40) : ADS1219(i2cController, addr) {}

        std::vector<uint8_t> getDeviceAddresses() const override {
            return {0x40};
        }
};

#endif
