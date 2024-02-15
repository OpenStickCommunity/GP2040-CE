#include "spi_analog_ads1256.h"
#include "storagemanager.h"

#define ADS1256_MAX (float)((1 << 23) - 1)

bool SPIAnalog1256Input::available() {
    const AnalogADS1256Options& options = Storage::getInstance().getAddonOptions().analogADS1256Options;
    return (options.enabled && PeripheralManager::getInstance().isSPIEnabled(options.spiBlock));
}

void SPIAnalog1256Input::setup() {
    const AnalogADS1256Options& options = Storage::getInstance().getAddonOptions().analogADS1256Options;
    PeripheralSPI* spi = PeripheralManager::getInstance().getSPI(options.spiBlock);

    // Init our ADS1256 library
    ads = new ADS1256(spi, options.drdyPin, options.resetPin, -1, options.csPin, (float)ADS1256_VREF_VOLTAGE);
    ads->init();
}

void SPIAnalog1256Input::process() {
    for (uint8_t i = 0; i < 8; i++) {
        float ratio = std::clamp(ads->convertToVoltage(ads->cycleSingle()) / 3.3f, 0.0f, 1.0f);
        values[i] = static_cast<uint16_t>(65535.f * ratio);
    }

    // Tells the ADC we're done sampling and flags to reset
    // the read cycle next time an ADC read is performed
    ads->stopConversion();

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    gamepad->state.lx = values[0];
    gamepad->state.ly = values[1];
    gamepad->state.rx = values[2];
    gamepad->state.ry = values[3];
}
