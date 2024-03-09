#ifndef GPCOMMS_SPI_PERIPHERAL_H_
#define GPCOMMS_SPI_PERIPHERAL_H_

#include "gpaddon.h"
#include "gamepad.h"
#include "peripheralmanager.h"
#include "gpcomms.h"

#ifndef GPCOMMS_SPI_PERIPHERAL_ENABLED
#define GPCOMMS_SPI_PERIPHERAL_ENABLED 0
#endif

#ifndef GPCOMMS_SPI_PERIPHERAL_BLOCK
#define GPCOMMS_SPI_PERIPHERAL_BLOCK 0
#endif

// BootselButton Module Name
#define GPCommsSPIPeripheralName "GPCommsSPIPeripheral"

class GPCommsSPIPeripheralAddon : public GPAddon {
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return GPCommsSPIPeripheralName; }
private:
	PeripheralSPI *spi;
};

#endif
