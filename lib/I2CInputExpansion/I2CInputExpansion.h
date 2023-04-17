#ifndef _GPDAUGHTER_H_
#define _GPDAUGHTER_H_

#include <BitBang_I2C.h>

class I2CInputExpansion
{
private:

	BBI2C _i2c;
	int32_t _i2c_speed;
	uint8_t _i2c_addr;
    uint8_t _buff[32];

public:

    I2CInputExpansion(int bWire, int sda, int scl, i2c_inst_t *picoI2C, int32_t iSpeed, uint8_t addr = 0x37);

    void begin();
    void read();
    uint16_t get_value(unsigned int channel);
};

#endif
