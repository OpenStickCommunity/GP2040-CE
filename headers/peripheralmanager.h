#ifndef _PERIPHERALMANAGER_H_
#define _PERIPHERALMANAGER_H_

#include "peripheral_i2c.h"
#include "peripheral_spi.h"
#include "peripheral_usb.h"

#define PMGR PeripheralManager::getInstance()

class PeripheralManager {
public:
    PeripheralManager(){}
	PeripheralManager(PeripheralManager const&) = delete;
	void operator=(PeripheralManager const&)  = delete;
	static PeripheralManager& getInstance() // Thread-safe storage ensures cross-thread talk
	{
		static PeripheralManager instance;
		return instance;
	}

    PeripheralI2C* getI2C(uint8_t block);
    PeripheralSPI* getSPI(uint8_t block);
    PeripheralUSB* getUSB(uint8_t block);

    void initUSB();
    void initI2C();
    void initSPI();

    bool isI2CEnabled(uint8_t block);
    bool isSPIEnabled(uint8_t block);
    bool isUSBEnabled(uint8_t block);
private:
    PeripheralI2C blockI2C0;
    PeripheralI2C blockI2C1;

    PeripheralSPI blockSPI0;
    PeripheralSPI blockSPI1;

    PeripheralUSB blockUSB0;
};

#endif