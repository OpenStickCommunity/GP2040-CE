#ifndef _I2CINPUTEXPANSION_H
#define _I2CINPUTEXPANSION_H

#include <I2CInputExpansion.h>
#include <vector>

#include "gpaddon.h"

#include "GamepadEnums.h"

#ifndef I2C_INPUT_EXPANSION_ENABLED
#define I2C_INPUT_EXPANSION_ENABLED 0
#endif

#ifndef I2C_INPUT_EXPANSION_SDA_PIN
#define I2C_INPUT_EXPANSION_SDA_PIN -1
#endif

#ifndef I2C_INPUT_EXPANSION_SCL_PIN
#define I2C_INPUT_EXPANSION_SCL_PIN -1
#endif

#ifndef I2C_INPUT_EXPANSION_BLOCK
#define I2C_INPUT_EXPANSION_BLOCK i2c1
#endif

#ifndef I2C_INPUT_EXPANSION_SPEED
#define I2C_INPUT_EXPANSION_SPEED 100000
#endif

#ifndef I2C_INPUT_EXPANSION_ADDRESS
#define I2C_INPUT_EXPANSION_ADDRESS 0x37
#endif

#ifndef I2C_INPUT_EXPANSION_ANALOG_MAP
#define I2C_INPUT_EXPANSION_ANALOG_MAP "lx,ly,rx,ry,l2,r2"
#endif

#ifndef I2C_INPUT_EXPANSION_DIGITAL_MAP
#define I2C_INPUT_EXPANSION_DIGITAL_MAP ""
#endif

// I2C Input Expansion Name
#define I2CInputExpansionName "I2CInputExpansion"

class I2CInputExpansionAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();       // Analog Setup
	virtual void preprocess() {}
	virtual void process();     // Analog Process
    virtual std::string name() { return I2CInputExpansionName; }
private:
    void decode_adc_map(const std::string&);
    void decode_pin_map(const std::string&);
    I2CInputExpansion * expansion;
    std::vector<unsigned long> adc_map;
    std::vector<unsigned long> pin_map;
	uint32_t uIntervalMS; // Expansion board cooldown
	uint32_t nextTimer; // Throttle timer
};

#endif  // _I2CINPUTEXPANSION_H_
