#include <cstdio>
#include "peripheral_i2c.h"

PeripheralI2C::PeripheralI2C() {
#ifdef PICO_DEFAULT_I2C_INSTANCE
    _I2C = PICO_DEFAULT_I2C_INSTANCE;
    _SDA = PICO_DEFAULT_I2C_SDA_PIN;
    _SCL = PICO_DEFAULT_I2C_SCL_PIN;
    _Speed = DEFAULT_SPEED;
#endif
}

void PeripheralI2C::setConfig(uint8_t block, uint8_t sda, uint8_t scl, uint32_t speed) {
    if (block < NUM_I2CS) {
        _I2C = _hardwareBlocks[block];
        _SDA = sda;
        _SCL = scl;
        _Speed = speed;
        configured = true;
        setup();
    } else {
        // currently not supported
    }
}

void PeripheralI2C::setup() {
    if ((_SDA + 2 * i2c_hw_index(_I2C))%4 != 0) return;
    if ((_SCL + 3 + 2 * i2c_hw_index(_I2C))%4 != 0) return;

    i2c_init(_I2C, _Speed);
    gpio_set_function(_SDA, GPIO_FUNC_I2C);
    gpio_set_function(_SCL, GPIO_FUNC_I2C);

    gpio_pull_up(_SDA);
    gpio_pull_up(_SCL);
}

int16_t PeripheralI2C::read(uint8_t address, uint8_t *data, uint16_t len, bool isBlock) {
    return i2c_read_blocking(_I2C, address, data, len, isBlock);
}

int16_t PeripheralI2C::readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
    int16_t registerCheck;
    registerCheck = i2c_write_blocking(_I2C, address, &reg, 1, true);
    if (registerCheck >= 0) {
        registerCheck = i2c_read_blocking(_I2C, address, data, len, false);
    }
    return (registerCheck >= 0);
}

int16_t PeripheralI2C::write(uint8_t address, uint8_t *data, uint16_t len, bool isBlock) {
    return i2c_write_blocking(_I2C, address, data, len, isBlock);
}

uint8_t PeripheralI2C::test(uint8_t address) {
    uint8_t data;
    int16_t ret = i2c_read_blocking(_I2C, address, &data, 1, false);
    return (ret >= 0);
}