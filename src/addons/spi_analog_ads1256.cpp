#include "spi_analog_ads1256.h"
#include "storagemanager.h"

bool SPIAnalog1256Input::available() {
    const AnalogADS1256Options& options = Storage::getInstance().getAddonOptions().analogADS1256Options;
    return (options.enabled && PeripheralManager::getInstance().isSPIEnabled(options.spiBlock));
}

void SPIAnalog1256Input::setup() {
    const AnalogADS1256Options& options = Storage::getInstance().getAddonOptions().analogADS1256Options;
    PeripheralSPI* spi = PeripheralManager::getInstance().getSPI(options.spiBlock);
    enableTriggers = options.enableTriggers;
    readChannelCount = 4 + (enableTriggers ? 2 : 0);
    analogMax = options.avdd;

    Gamepad * gamepad = Storage::getInstance().GetGamepad();
    gamepad->hasAnalogTriggers = enableTriggers;

    // Init our ADS1256 library
    ads = new ADS1256(spi, options.drdyPin, -1, -1, options.csPin, (float)ADS1256_VREF_VOLTAGE);
    ads->init(ADS1256_DRATE_30000SPS, ADS1256_PGA_1, true);
}

void SPIAnalog1256Input::process() {
    // Read the first X channels
    for (uint8_t i = 0; i < readChannelCount; i++) {
        values[i] = ads->convertToVoltage(ads->cycleSingle());
    }

    // Tells the ADC we're done sampling and flags to reset
    // the read cycle next time an ADC read is performed
    ads->stopConversion();

    Gamepad * gamepad = Storage::getInstance().GetGamepad();

    gamepad->state.lx = convert24to16bit(values[0]);
    gamepad->state.ly = convert24to16bit(values[1]);
    gamepad->state.rx = convert24to16bit(values[2]);
    gamepad->state.ry = convert24to16bit(values[3]);

    if (enableTriggers) {
        gamepad->hasAnalogTriggers = enableTriggers;
        gamepad->state.lt = convert24to8bit(values[4]);
        gamepad->state.rt = convert24to8bit(values[5]);
    }
}

uint8_t SPIAnalog1256Input::convert24to8bit(float voltage) {
    float ratio = std::clamp(voltage, 0.0f, analogMax) / analogMax;
    return (uint8_t)(255.f * ratio);
}

uint16_t SPIAnalog1256Input::convert24to16bit(float voltage) {
    float ratio = std::clamp(voltage, 0.0f, analogMax) / analogMax;
    return (uint16_t)(65535.f * ratio);
}
