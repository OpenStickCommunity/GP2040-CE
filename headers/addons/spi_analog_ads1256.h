#ifndef SPI_ANALOG_ADS1256_H_
#define SPI_ANALOG_ADS1256_H_

#include <ADS1256.h>
#include <algorithm>

#include "gpaddon.h"

#include "GamepadEnums.h"
#include "peripheralmanager.h"

#ifndef SPI_ANALOG1256_ENABLED
#define SPI_ANALOG1256_ENABLED 0
#endif

#ifndef SPI_ANALOG1256_CS_PIN
#define SPI_ANALOG1256_CS_PIN -1
#endif

#ifndef SPI_ANALOG1256_DRDY_PIN
#define SPI_ANALOG1256_DRDY_PIN -1
#endif

#ifndef SPI_ANALOG1256_BLOCK
#define SPI_ANALOG1256_BLOCK spi0
#endif

#ifndef SPI_ANALOG1256_SPEED
#define SPI_ANALOG1256_SPEED 5000000
#endif

// Analog Module Name
#define SPIAnalog1256Name "SPIAnalogADS1256"

class SPIAnalog1256Input : public GPAddon {
public:
    virtual bool available();
    virtual void setup();       // Analog Setup
    virtual void preprocess() {}
    virtual void process();     // Analog Process
    virtual void postprocess(bool sent) {}
    virtual void reinit() {}
    virtual std::string name() { return SPIAnalog1256Name; }
private:
    uint8_t convert24to8bit(float voltage);
    uint16_t convert24to16bit(float voltage);

    ADS1256 * ads;
    float values[ADS1256_CHANNEL_COUNT]; // Cache for latest read values
    bool enableTriggers;
    uint8_t readChannelCount; // Number of channels to read from the ADC
    float analogMax = ADS1256_MAX_3V;
};

#endif  // SPI_ANALOG_ADS1256_H_
