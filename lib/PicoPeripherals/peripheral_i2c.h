#ifndef _PERIPHERAL_I2C_H_
#define _PERIPHERAL_I2C_H_

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/platform_defs.h>

class PeripheralI2C {
public:
    PeripheralI2C();
    ~PeripheralI2C() {}

    bool configured = false;

    i2c_inst_t* getController() { return _I2C; }

    void setConfig(uint8_t block, uint8_t sda, uint8_t scl, uint32_t speed);

    int16_t read(uint8_t address, uint8_t *data, uint16_t len, bool isBlock=false);
    int16_t readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);

    int16_t write(uint8_t address, uint8_t *data, uint16_t len, bool isBlock=true);

    uint8_t test(uint8_t address);
private:
    const uint32_t DEFAULT_SPEED = 100000;

    uint8_t _SDA;
    uint8_t _SCL;
    i2c_inst_t *_I2C;
    int32_t _Speed;

    i2c_inst_t* _hardwareBlocks[NUM_I2CS] = {i2c0,i2c1};

    void setup();
};

#endif
