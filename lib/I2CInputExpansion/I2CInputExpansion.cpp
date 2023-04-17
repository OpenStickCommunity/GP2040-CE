#include "I2CInputExpansion.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#include <cstring>

I2CInputExpansion::I2CInputExpansion(int bWire, int sda, int scl, i2c_inst_t *picoI2C, int32_t speed, uint8_t addr)
{
    _i2c.iSDA = sda;
    _i2c.iSCL = scl;
    _i2c.picoI2C = picoI2C;
    _i2c.bWire = bWire;
    _i2c_speed = speed;
    _i2c_addr = addr;

    memset(_buff, 0, sizeof(_buff));
}

void I2CInputExpansion::begin()
{
    I2CInit(&_i2c, _i2c_speed);
}

void I2CInputExpansion::read()
{
    I2CRead(&_i2c, _i2c_addr, _buff, 32);
}

uint16_t I2CInputExpansion::get_value(unsigned int channel)
{
    constexpr auto n_chan = sizeof(_buff) / 2;

    if (channel >= n_chan)
        return 0;

    const auto i = 2 * channel;

    return (uint16_t)_buff[i] | (uint16_t)(_buff[i + 1] << 8);
}
