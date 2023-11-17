#ifndef _PERIPHERAL_USB_H_
#define _PERIPHERAL_USB_H_

#include <hardware/gpio.h>
#include <hardware/platform_defs.h>

#define NUM_USBS 1

class PeripheralUSB {
public:
    PeripheralUSB();
    PeripheralUSB(uint8_t block, uint8_t dp, uint8_t order, int8_t enable5V);
    ~PeripheralUSB() {}

//    spi_inst_t* getController() { return _SPI; }

private:
    uint8_t _DP;
    uint8_t _Order;
    int8_t _Enable5V;

    //spi_inst_t* _hardwareBlocks[NUM_USBS] = {spi0,spi1};

    void setup();
};

#endif
