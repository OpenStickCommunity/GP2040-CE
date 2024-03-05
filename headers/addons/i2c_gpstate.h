#ifndef I2C_GPSTATE_H_
#define I2C_GPSTATE_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"

#ifndef I2C_GPSTATE_ENABLED
#define I2C_GPSTATE_ENABLED 1
#endif

#ifndef I2C_GPSTATE_BLOCK
#define I2C_GPSTATE_BLOCK 1
#endif

#ifndef I2C_GPSTATE_ADDRESS
#define I2C_GPSTATE_ADDRESS 0xCE
#endif

// BootselButton Module Name
#define I2CGPStateName "I2CGPState"

class I2CGPStateAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return I2CGPStateName; }
private:
	PeripheralI2C *i2c;
	uint8_t addr;
};

#endif
