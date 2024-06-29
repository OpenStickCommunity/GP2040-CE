// PCF8575 Library
// category=Signal Input/Output

// Written by:
//   Mike Parks <mikepparks@gmail.com>

#ifndef _PCF8575_H_
#define _PCF8575_H_

#include <vector>

#include "i2cdevicebase.h"

class PCF8575 : public I2CDeviceBase {
    public:
        // Constructor
        PCF8575() {}
        PCF8575(PeripheralI2C *i2cController, uint8_t addr = 0x20) {
            this->i2c = i2cController;
            this->address = addr;
        }

        std::vector<uint8_t> getDeviceAddresses() const override {
            return {0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27};
        }

        //// Methods
        void begin();
        void reset();
        //void start();

        void setI2C(PeripheralI2C *i2cController) { this->i2c = i2cController; }
        void setAddress(uint8_t addr) { this->address = addr; }

        void send(uint16_t value);
        uint16_t receive();

        uint16_t pins() { return dataReceived; }

        void setPin(uint8_t pinNumber, uint8_t value);
        bool getPin(uint8_t pinNumber);
    private:
        const uint16_t initialValue = 0xFFFF;
        uint8_t uc[128];

        uint16_t dataSent;
        uint16_t dataReceived = initialValue;
    protected:
        PeripheralI2C* i2c = nullptr;
        uint8_t address = 0;
};

#endif
