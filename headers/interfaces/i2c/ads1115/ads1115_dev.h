#ifndef _ADS1115DEVICE_H_
#define _ADS1115DEVICE_H_

#include <vector>

#include "ADS1115.h"
#include "i2cdevicebase.h"

class ADS1115Device : public ADS1115, public I2CDeviceBase {
public:
  // Constructor
  ADS1115Device() {}
  ADS1115Device(PeripheralI2C *i2cController,
                uint8_t addr = ADS111X_I2C_ADDR_GND)
      : ADS1115(i2cController, addr) {}

  std::vector<uint8_t> getDeviceAddresses() const override {
    return {ADS111X_I2C_ADDR_GND, ADS111X_I2C_ADDR_SCLK, ADS111X_I2C_ADDR_SDA,
            ADS111X_I2C_ADDR_VDD};
  }
};

#endif
