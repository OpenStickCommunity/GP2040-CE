#ifndef _I2CDEVICEBASE_H_
#define _I2CDEVICEBASE_H_

#include <vector>

#include "peripheral_i2c.h"
#include "devicebase.h"

class I2CDeviceBase : public DeviceBase {
    public:
        I2CDeviceBase() {}
        ~I2CDeviceBase() {}

        virtual std::vector<uint8_t> getDeviceAddresses() const = 0;
};

#endif