#ifndef _I2CAnalog1115_H
#define _I2CAnalog1115_H

#include "ads1115_dev.h"
#include "gamepad.h"
#include "gpaddon.h"
#include "GamepadEnums.h"
#include "peripheralmanager.h"


#ifndef I2C_ANALOG1115_ENABLED
#define I2C_ANALOG1115_ENABLED 1
#endif

#ifndef I2C_ANALOG1115_SDA_PIN
#define I2C_ANALOG1115_SDA_PIN    0
#endif

#ifndef I2C_ANALOG1115_SCL_PIN
#define I2C_ANALOG1115_SCL_PIN    1
#endif

#ifndef I2C_ANALOG1115_BLOCK
#define I2C_ANALOG1115_BLOCK i2c0
#endif

#ifndef I2C_ANALOG1115_SPEED
#define I2C_ANALOG1115_SPEED 400000
#endif

#ifndef I2C_ANALOG1115_ADDRESS
#define I2C_ANALOG1115_ADDRESS 0x48
#endif

// Analog ADS1115 Module Name
#define I2CAnalog1115Name "ADS1115I2CAnalog"

// NOTE probably have to rename the file
class I2CAnalog1115Input : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // Analog Setup
    virtual void preprocess() {}
    virtual void process();     // Analog Process
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return I2CAnalog1115Name; }
private:
    Gamepad * gamepad;
    ADS1115Device * ads;
    float pins[4];
    int channelHop;
    uint32_t uIntervalMS;       // ADS1115 Interval
    uint32_t nextTimer;         // Turbo Timer
};

#endif