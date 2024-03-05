#ifndef I2C_GPSTATE_CONTROLLER_H_
#define I2C_GPSTATE_CONTROLLER_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"

#ifndef I2C_GPSTATE_CONTROLLER_ENABLED
#define I2C_GPSTATE_CONTROLLER_ENABLED 1
#endif

#ifndef I2C_GPSTATE_CONTROLLER_BLOCK
#define I2C_GPSTATE_CONTROLLER_BLOCK 1
#endif

// BootselButton Module Name
#define I2CGPStateControllerName "I2CGPStateController"

class I2CGPStateControllerAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return I2CGPStateControllerName; }
private:
	PeripheralI2C *i2c;
	uint8_t addr;
};

#endif
