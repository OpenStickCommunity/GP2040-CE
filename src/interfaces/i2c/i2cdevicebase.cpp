#include <vector>
#include "i2cdevicebase.h"

int8_t I2CDeviceBase::scanForDevice() {
    if (i2c != nullptr) {
        std::vector<uint8_t> addressList = getDeviceAddresses();
        for (uint8_t i = 0; i < addressList.size(); i++) {
            uint8_t result = i2c->test(addressList[i]);

            if (result) return addressList[i];
        }
    }
    return -1;
}