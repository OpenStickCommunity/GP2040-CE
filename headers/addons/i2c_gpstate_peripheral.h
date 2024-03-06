#ifndef I2C_GPSTATE_PERIPHERAL_H_
#define I2C_GPSTATE_PERIPHERAL_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"


#ifndef I2C_GPSTATE_PERIPHERAL_ENABLED
#define I2C_GPSTATE_PERIPHERAL_ENABLED 1
#endif

#ifndef I2C_GPSTATE_PERIPHERAL_BLOCK
#define I2C_GPSTATE_PERIPHERAL_BLOCK 1
#endif

// BootselButton Module Name
#define I2CGPStatePeripheralName "I2CGPStatePeripheral"

class I2CGPStatePeripheralAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return I2CGPStatePeripheralName; }
private:
	static void handleGamepadStatus(I2C_GPStatus gpStatus);
	static void handleGamepadState(I2C_GPState gpState);
	static void handleGamepadMessage(I2C_GPMessage gpMessage);

	PeripheralI2C *i2c;
	uint8_t addr;
};

#endif
