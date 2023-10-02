#include "peripheralmanager.h"
#include "storagemanager.h"

PeripheralManager::PeripheralManager() {
    const PeripheralOptions& peripheralOptions = Storage::getInstance().getPeripheralOptions();

    if (peripheralOptions.blockI2C0.enabled) blockI2C0 = new PeripheralI2C(0, peripheralOptions.blockI2C0.sda, peripheralOptions.blockI2C0.scl, peripheralOptions.blockI2C0.speed);
    if (peripheralOptions.blockI2C1.enabled) blockI2C1 = new PeripheralI2C(1, peripheralOptions.blockI2C1.sda, peripheralOptions.blockI2C1.scl, peripheralOptions.blockI2C1.speed);
    
    //if (peripheralOptions.blockSPI0.enabled) blockSPI0 = new PeripheralSPI();
    //if (peripheralOptions.blockSPI1.enabled) blockSPI1 = new PeripheralSPI();
}

PeripheralI2C* PeripheralManager::getI2C(uint8_t block) {
    if (block < NUM_I2CS) {
        return ((block == 0) ? blockI2C0 : blockI2C1);
    }
    return nullptr;
}

PeripheralSPI* PeripheralManager::getSPI(uint8_t block) {
    if (block < NUM_SPIS) {
        return ((block == 0) ? blockSPI0 : blockSPI1);
    }
    return nullptr;
}

bool PeripheralManager::isI2CEnabled(uint8_t block) {
    if (block < NUM_I2CS) {
        return (((block == 0) ? blockI2C0 : blockI2C1) != nullptr);
    }
    return false;
}

bool PeripheralManager::isSPIEnabled(uint8_t block) {
    if (block < NUM_SPIS) {
        return (((block == 0) ? blockSPI0 : blockSPI1) != nullptr);
    }
    return false;
}
