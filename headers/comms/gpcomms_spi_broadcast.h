#ifndef GPCOMMS_SPI_BROADCAST_H_
#define GPCOMMS_SPI_BROADCAST_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"
#include "gpcomms.h"

#ifndef GPCOMMS_SPI_BROADCAST_ENABLED
#define GPCOMMS_SPI_BROADCAST_ENABLED 0
#endif

#ifndef GPCOMMS_SPI_BROADCAST_BLOCK
#define GPCOMMS_SPI_BROADCAST_BLOCK 0
#endif

#define GPCommsSPIControllerName "GPCommsSPIController"

class GPCommsSPIBroadcastAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return GPCommsSPIControllerName; }
private:
	void sendStatus(Gamepad *gamepad);
	void sendState(Gamepad *gamepad);
	void sendMessage(char *text, uint16_t length);

	PeripheralSPI *spi;
	uint8_t buf[GPCOMMS_BUFFER_SIZE] = {0};
};

#endif
