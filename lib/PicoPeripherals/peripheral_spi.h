#ifndef _PERIPHERAL_SPI_H_
#define _PERIPHERAL_SPI_H_

#include <hardware/gpio.h>
#include <hardware/spi.h>
#include <hardware/platform_defs.h>

class PeripheralSPI {
public:
    PeripheralSPI();
    ~PeripheralSPI() {}

    spi_inst_t* getController() { return _SPI; }

private:
    spi_inst_t *_SPI;
};

#endif
