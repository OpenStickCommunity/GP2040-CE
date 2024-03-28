#ifndef GPCOMMS_SPI_RECEIVER_H_
#define GPCOMMS_SPI_RECEIVER_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"
#include "gpcomms.h"

#define GPCommsSPIReceiverName "GPCommsSPIReceiver"

class GPCommsSPIReceiverAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return GPCommsSPIReceiverName; }
private:
	PeripheralSPI *spi;
};

#endif
