#include <cstdio>
#include "peripheral_i2c.h"

PeripheralI2C::PeripheralI2C() {
#ifdef PICO_DEFAULT_I2C_INSTANCE

#if PICO_SDK_VERSION_MAJOR >= 2
    _I2C = PICO_DEFAULT_I2C_INSTANCE();
#else
    _I2C = PICO_DEFAULT_I2C_INSTANCE;
#endif

    _SDA = PICO_DEFAULT_I2C_SDA_PIN;
    _SCL = PICO_DEFAULT_I2C_SCL_PIN;
    _Speed = DEFAULT_SPEED;
#endif
}

void PeripheralI2C::setConfig(uint8_t block, int8_t sda, int8_t scl, uint32_t speed) {
    if ((block < NUM_I2CS) && (sda > -1) && (scl > -1)) {
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

    // reset the bus before using it
    clear();
}

int16_t PeripheralI2C::read(uint8_t address, uint8_t *data, uint16_t len, bool isBlock) {
    if ((_exclusiveAddress > -1) && (_exclusiveAddress != address)) return -1;

    int16_t result = i2c_read_blocking(_I2C, address, data, len, isBlock);
#ifdef DEBUG_PERIPHERALI2C
    printf("PeripheralI2C::write %d:%d (blocking? %d)\n", address, len, isBlock);
    for (int i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
    printf("\nResult: %d\n", result);
    printf("-----\n");
#endif    
    return result;
}

int16_t PeripheralI2C::readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len) {
    if ((_exclusiveAddress > -1) && (_exclusiveAddress != address)) return -1;

    int16_t registerCheck;
    registerCheck = i2c_write_blocking(_I2C, address, &reg, 1, true);
    if (registerCheck >= 0) {
        registerCheck = i2c_read_blocking(_I2C, address, data, len, false);
    }
    return (registerCheck >= 0);
}

int16_t PeripheralI2C::write(uint8_t address, uint8_t *data, uint16_t len, bool isBlock) {
    if ((_exclusiveAddress > -1) && (_exclusiveAddress != address)) return -1;

#ifdef DEBUG_PERIPHERALI2C
    printf("PeripheralI2C::write %d:%d (blocking? %d)\n", address, len, isBlock);
    for (int i = 0; i < len; i++) {
        printf("%02x ", data[i]);
    }
#endif
    int16_t result = i2c_write_blocking(_I2C, address, data, len, isBlock);
#ifdef DEBUG_PERIPHERALI2C
    printf("\nResult: %d\n", result);
    printf("-----\n");
#endif
    return result;
}

uint8_t PeripheralI2C::test(uint8_t address) {
    uint8_t data;
    int16_t ret = i2c_read_blocking(_I2C, address, &data, 1, false);
    return (ret >= 0);
}

void PeripheralI2C::clear() {
    // reset the bus
    test(0xFF);
}

std::map<uint8_t,bool> PeripheralI2C::scan() {
    std::map<uint8_t,bool> result;

    for (uint8_t addr = 0; addr < (1 << 7); ++addr) {
        int8_t ret;
        uint8_t rxdata;
        ret = i2c_read_blocking(_I2C, addr, &rxdata, 1, false);

        if (ret >= 0) {
            result.insert({addr,(ret >= 0)});
        }
    }

#ifdef DEBUG_PERIPHERALI2C
    printf("%d\n", result.size());
#endif

    return result;
}