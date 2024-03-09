#ifndef GPCOMMS_I2C_BROADCAST_H_
#define GPCOMMS_I2C_BROADCAST_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"
#include "gpcomms.h"

#define GPCommsI2CBroadcastName "GPCommsI2CBroadcast"

class GPCommsI2CBroadcastAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return GPCommsI2CBroadcastName; }
private:
	void sendStatus(Gamepad *gamepad);
	void sendState(Gamepad *gamepad);
	void sendMessage(char *text, uint16_t length);

	PeripheralI2C *i2c;
	uint8_t addr;
	uint8_t buf[GPCOMMS_BUFFER_SIZE] = {0};
};

#endif
