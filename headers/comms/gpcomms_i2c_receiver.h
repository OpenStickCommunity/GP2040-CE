#ifndef GPCOMMS_I2C_RECEIVER_H_
#define GPCOMMS_I2C_RECEIVER_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"
#include "gpcomms.h"

#define GPCommsI2CReceiverName "GPCommsI2CReceiver"

class GPCommsI2CReceiverAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process() {}
	virtual std::string name() { return GPCommsI2CReceiverName; }
private:
	static void handler(i2c_inst_t *i2c, i2c_slave_event_t event);

	PeripheralI2C *i2c;
	uint8_t addr;
};

#endif
