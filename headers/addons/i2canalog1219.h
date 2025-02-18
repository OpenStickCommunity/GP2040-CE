#ifndef _I2CAnalog_H
#define _I2CAnalog_H

#include "ads1219_dev.h"

#include "gpaddon.h"

#include "GamepadEnums.h"
#include "peripheralmanager.h"

#ifndef I2C_ANALOG1219_ENABLED
#define I2C_ANALOG1219_ENABLED 0
#endif

#ifndef I2C_ANALOG1219_SDA_PIN
#define I2C_ANALOG1219_SDA_PIN    -1
#endif

#ifndef I2C_ANALOG1219_SCL_PIN
#define I2C_ANALOG1219_SCL_PIN    -1
#endif

#ifndef I2C_ANALOG1219_BLOCK
#define I2C_ANALOG1219_BLOCK i2c0
#endif

#ifndef I2C_ANALOG1219_SPEED
#define I2C_ANALOG1219_SPEED 400000
#endif

#ifndef I2C_ANALOG1219_ADDRESS
#define I2C_ANALOG1219_ADDRESS 0x40
#endif

// Analog Module Name
#define I2CAnalog1219Name "I2CAnalog"

typedef struct {
    float A[4];
} ADS_PINS;

class I2CAnalog1219Input : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // Analog Setup
    virtual void preprocess() {}
    virtual void process();     // Analog Process
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return I2CAnalog1219Name; }
private:
    ADS1219Device * ads;
    ADS_PINS pins;
    int channelHop;
    uint32_t uIntervalMS;       // ADS1219 Interval
    uint32_t nextTimer;         // Turbo Timer
};

#endif  // _I2CAnalog_H_
