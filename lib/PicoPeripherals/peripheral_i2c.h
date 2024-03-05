#ifndef _PERIPHERAL_I2C_H_
#define _PERIPHERAL_I2C_H_

#include <hardware/gpio.h>
#include <hardware/i2c.h>
#include <hardware/platform_defs.h>
#include <pico/i2c_slave.h>

//#define DEBUG_PERIPHERALI2C

#ifndef I2C_SLAVE_ID
#define I2C_SLAVE_ID 0xCE
#endif

#ifndef I2C0_ENABLED
#define I2C0_ENABLED 0
#endif

#ifndef I2C0_PIN_SDA
#define I2C0_PIN_SDA -1
#endif

#ifndef I2C0_PIN_SCL
#define I2C0_PIN_SCL -1
#endif

#ifndef I2C0_SPEED
#define I2C0_SPEED 400000
#endif

#ifndef I2C1_ENABLED
#define I2C1_ENABLED 0
#endif

#ifndef I2C1_PIN_SDA
#define I2C1_PIN_SDA -1
#endif

#ifndef I2C1_PIN_SCL
#define I2C1_PIN_SCL -1
#endif

#ifndef I2C1_SPEED
#define I2C1_SPEED 400000
#endif

class PeripheralI2C {
public:
    PeripheralI2C();
    ~PeripheralI2C() {}

    bool configured = false;

    i2c_inst_t* getController() { return _I2C; }

    void setConfig(uint8_t block, uint8_t sda, uint8_t scl, uint32_t speed, void (*slaveHandler)(i2c_inst_t *, i2c_slave_event_t) = nullptr);

    int16_t read(uint8_t address, uint8_t *data, uint16_t len, bool isBlock=false);
    int16_t readRegister(uint8_t address, uint8_t reg, uint8_t *data, uint16_t len);

    int16_t write(uint8_t address, uint8_t *data, uint16_t len, bool isBlock=true);

    uint8_t test(uint8_t address);
    void clear();
private:
    const uint32_t DEFAULT_SPEED = 400000;
    uint8_t _SDA;
    uint8_t _SCL;
    i2c_inst_t *_I2C;
    int32_t _Speed;
    bool isSlave;
    void (*slaveHandler)(i2c_inst_t *, i2c_slave_event_t) = nullptr;

    i2c_inst_t* _hardwareBlocks[NUM_I2CS] = {i2c0,i2c1};

    void setup();
};

#endif
