#ifndef _PERIPHERAL_SPI_H_
#define _PERIPHERAL_SPI_H_

#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/platform_defs.h>

class PeripheralSPI {
public:
    PeripheralSPI();
    ~PeripheralSPI() {}

    bool configured = false;

    spi_inst_t* getController() { return _SPI; }

    void setConfig(uint8_t block, uint8_t tx, uint8_t rx, uint8_t sck, uint8_t cs);

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
