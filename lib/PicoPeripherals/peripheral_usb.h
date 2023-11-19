#ifndef _PERIPHERAL_USB_H_
#define _PERIPHERAL_USB_H_

#include <hardware/gpio.h>
#include <hardware/platform_defs.h>

#ifndef USB_PERIPHERAL_ENABLED
#define USB_PERIPHERAL_ENABLED 0
#endif

#ifndef USB_PERIPHERAL_PIN_DPLUS
#define USB_PERIPHERAL_PIN_DPLUS -1
#endif

#ifndef USB_PERIPHERAL_PIN_5V
#define USB_PERIPHERAL_PIN_5V -1
#endif

#ifndef USB_PERIPHERAL_PIN_ORDER
#define USB_PERIPHERAL_PIN_ORDER 0
#endif

#define NUM_USBS 1

class PeripheralUSB {
public:
    PeripheralUSB();
    PeripheralUSB(uint8_t block, int8_t dp, int8_t enable5v, uint8_t order);
    ~PeripheralUSB() {}

//    spi_inst_t* getController() { return _SPI; }

private:
    uint8_t _DP;
    uint8_t _Order;
    int8_t _Enable5v;

    //spi_inst_t* _hardwareBlocks[NUM_USBS] = {spi0,spi1};

    void setup();
};

#endif
