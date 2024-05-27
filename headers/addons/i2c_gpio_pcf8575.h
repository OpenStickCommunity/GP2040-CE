#ifndef PCF8575ADDON_H
#define PCF8575ADDON_H

#include "pcf8575.h"

#include "gpaddon.h"

#include "GamepadEnums.h"
#include "peripheralmanager.h"

#include <map>

#ifndef I2C_PCF8575_ENABLED
#define I2C_PCF8575_ENABLED 0
#endif

#ifndef I2C_PCF8575_SDA_PIN
#define I2C_PCF8575_SDA_PIN    -1
#endif

#ifndef I2C_PCF8575_SCL_PIN
#define I2C_PCF8575_SCL_PIN    -1
#endif

#ifndef I2C_PCF8575_BLOCK
#define I2C_PCF8575_BLOCK i2c0
#endif

#ifndef I2C_PCF8575_SPEED
#define I2C_PCF8575_SPEED 400000
#endif

#ifndef I2C_PCF8575_ADDRESS
#define I2C_PCF8575_ADDRESS 0x40
#endif

// Analog Module Name
#define PCF8575AddonName "PCF8575"

class PCF8575Addon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
    virtual std::string name() { return PCF8575AddonName; }

    std::map<uint8_t, GpioMappingInfo> pinRef;
private:
    PCF8575* pcf;

    bool inputButtonUp = false;
    bool inputButtonDown = false;
    bool inputButtonLeft = false;
    bool inputButtonRight = false;
    bool inputButtonB1 = false;
    bool inputButtonB2 = false;
    bool inputButtonB3 = false;
    bool inputButtonB4 = false;
    bool inputButtonL1 = false;
    bool inputButtonR1 = false;
    bool inputButtonL2 = false;
    bool inputButtonR2 = false;
    bool inputButtonS1 = false;
    bool inputButtonS2 = false;
    bool inputButtonL3 = false;
    bool inputButtonR3 = false;
    bool inputButtonA1 = false;
    bool inputButtonA2 = false;
    bool inputButtonFN = false;
};

#endif  // _I2CAnalog_H_
