#ifndef GPCOMMS_I2C_PERIPHERAL_H_
#define GPCOMMS_I2C_PERIPHERAL_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"
#include "gpcomms.h"

#ifndef GPCOMMS_I2C_PERIPHERAL_ENABLED
#define GPCOMMS_I2C_PERIPHERAL_ENABLED 0
#endif

#ifndef GPCOMMS_I2C_PERIPHERAL_BLOCK
#define GPCOMMS_I2C_PERIPHERAL_BLOCK 0
#endif

// BootselButton Module Name
#define GPCommsI2CPeripheralName "GPCommsI2CPeripheral"

class GPCommsI2CPeripheralAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process() {}
	virtual std::string name() { return GPCommsI2CPeripheralName; }
private:
	static void handler(i2c_inst_t *i2c, i2c_slave_event_t event);

	PeripheralI2C *i2c;
	uint8_t addr;
};

#endif
