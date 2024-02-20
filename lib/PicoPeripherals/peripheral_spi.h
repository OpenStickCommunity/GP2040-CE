#ifndef _PERIPHERAL_SPI_H_
#define _PERIPHERAL_SPI_H_

#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/platform_defs.h>


#ifndef SPI0_ENABLED
#define SPI0_ENABLED 0
#endif

#ifndef SPI0_PIN_RX
#define SPI0_PIN_RX -1
#endif

#ifndef SPI0_PIN_CS
#define SPI0_PIN_CS -1
#endif

#ifndef SPI0_PIN_SCK
#define SPI0_PIN_SCK -1
#endif

#ifndef SPI0_PIN_TX
#define SPI0_PIN_TX -1
#endif

#ifndef SPI1_ENABLED
#define SPI1_ENABLED 0
#endif

#ifndef SPI1_PIN_RX
#define SPI1_PIN_RX -1
#endif

#ifndef SPI1_PIN_CS
#define SPI1_PIN_CS -1
#endif

#ifndef SPI1_PIN_SCK
#define SPI1_PIN_SCK -1
#endif

#ifndef SPI1_PIN_TX
#define SPI1_PIN_TX -1
#endif

class PeripheralSPI {
public:
    PeripheralSPI();
    ~PeripheralSPI() {}

    bool configured = false;

    spi_inst_t* getController() { return _SPI; }

    void setConfig(uint8_t block, uint8_t tx, uint8_t rx, uint8_t sck, uint8_t cs);

    int16_t write(uint8_t *data, uint16_t len);
private:
    uint8_t _RX;
    uint8_t _TX;
    uint8_t _SCK;
    uint8_t _CS;
    spi_inst_t *_SPI;

    spi_inst_t* _hardwareBlocks[NUM_SPIS] = {spi0,spi1};

    void setup();
};

#endif
