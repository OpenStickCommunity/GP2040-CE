#ifndef _PERIPHERALMANAGER_H_
#define _PERIPHERALMANAGER_H_

#include "peripheral_i2c.h"
#include "peripheral_spi.h"

#define PMGR PeripheralManager::getInstance()

class PeripheralManager {
public:
    PeripheralManager();
    ~PeripheralManager() {}
	void operator=(PeripheralManager const&)  = delete;
	static PeripheralManager& getInstance() // Thread-safe storage ensures cross-thread talk
	{
		static PeripheralManager instance;
		return instance;
	}

    PeripheralI2C* getI2C(uint8_t block);
    PeripheralSPI* getSPI(uint8_t block);

    bool isI2CEnabled(uint8_t block);
    bool isSPIEnabled(uint8_t block);
private:
    PeripheralI2C* blockI2C0 = nullptr;
    PeripheralI2C* blockI2C1 = nullptr;

    PeripheralSPI* blockSPI0 = nullptr;
    PeripheralSPI* blockSPI1 = nullptr;
};

#endif