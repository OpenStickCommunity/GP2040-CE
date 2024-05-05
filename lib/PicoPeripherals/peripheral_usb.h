#ifndef _PERIPHERAL_USB_H_
#define _PERIPHERAL_USB_H_

#include <hardware/gpio.h>
#include <hardware/platform_defs.h>
#include "pio_usb.h"

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
    ~PeripheralUSB() {}

    bool configured = false;
    
    pio_usb_configuration_t* getController() { return _USB; }

    void setConfig(uint8_t block, int8_t dp, int8_t enable5v, uint8_t order);
private:
    int8_t _DP;
    uint8_t _Order;
    int8_t _Enable5v;

    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_usb_configuration_t *_USB = (&pio_cfg);

    pio_usb_configuration_t* _hardwareBlocks[NUM_USBS] = {&pio_cfg};

    void setup();
};

#endif
