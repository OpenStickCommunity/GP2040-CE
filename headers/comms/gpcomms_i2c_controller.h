#ifndef GPCOMMS_I2C_CONTROLLER_H_
#define GPCOMMS_I2C_CONTROLLER_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"
#include "gpcomms.h"

#ifndef GPCOMMS_I2C_CONTROLLER_ENABLED
#define GPCOMMS_I2C_CONTROLLER_ENABLED 1
#endif

#ifndef GPCOMMS_I2C_CONTROLLER_BLOCK
#define GPCOMMS_I2C_CONTROLLER_BLOCK 1
#endif

#define GPCOMMS_I2C_BUFFER_SIZE 100

// BootselButton Module Name
#define GPCommsI2CControllerName "GPCommsI2CController"

class GPCommsI2CControllerAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return GPCommsI2CControllerName; }
private:
	void sendGamepadStatus(Gamepad *gamepad);
	void sendGamepadState(Gamepad *gamepad);
	void sendGamepadMessage(Gamepad *gamepad, char *text, uint16_t length);

	PeripheralI2C *i2c;
	uint8_t addr;
	uint8_t buf[GPCOMMS_I2C_BUFFER_SIZE] = {0};
};

#endif
