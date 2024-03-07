#ifndef GPCOMMS_SPI_CONTROLLER_H_
#define GPCOMMS_SPI_CONTROLLER_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"
#include "gpcomms.h"

#ifndef GPCOMMS_SPI_CONTROLLER_ENABLED
#define GPCOMMS_SPI_CONTROLLER_ENABLED 1
#endif

#ifndef GPCOMMS_SPI_CONTROLLER_BLOCK
#define GPCOMMS_SPI_CONTROLLER_BLOCK 0
#endif

#define GPCommsSPIControllerName "GPCommsSPIController"

class GPCommsSPIControllerAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return GPCommsSPIControllerName; }
private:
	void sendGamepadStatus(Gamepad *gamepad);
	void sendGamepadState(Gamepad *gamepad);
	void sendGamepadMessage(Gamepad *gamepad, char *text, uint16_t length);

	PeripheralSPI *spi;
	uint8_t buf[GPCOMMS_BUFFER_SIZE] = {0};
};

#endif
