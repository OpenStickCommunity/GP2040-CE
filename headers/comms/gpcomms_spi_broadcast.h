#ifndef GPCOMMS_SPI_BROADCAST_H_
#define GPCOMMS_SPI_BROADCAST_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"
#include "gpcomms.h"

#define GPCommsSPIBroadcastName "GPCommsSPIBroadcast"

class GPCommsSPIBroadcastAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return GPCommsSPIBroadcastName; }
private:
	void sendStatus(Gamepad *gamepad);
	void sendState(Gamepad *gamepad);
	void sendMessage(char *text, uint16_t length);

	PeripheralSPI *spi;
	uint8_t buf[GPCOMMS_BUFFER_SIZE] = {0};
};

#endif
