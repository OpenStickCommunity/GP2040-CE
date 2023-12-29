#include "peripheral_usb.h"

PeripheralUSB::PeripheralUSB() {
    _DP = USB_PERIPHERAL_PIN_DPLUS;
    _Enable5v = USB_PERIPHERAL_PIN_5V;
    _Order = USB_PERIPHERAL_PIN_ORDER;
}

void PeripheralUSB::setConfig(uint8_t block, int8_t dp, int8_t enable5v, uint8_t order) {
    if (block < NUM_USBS) {
        _USB = _hardwareBlocks[block];
        _DP = dp;
        _Enable5v = enable5v;
        _Order = order;
        configured = true;
        setup();
    } else {
        // currently not supported
    }
}

void PeripheralUSB::setup() {
    if (_DP != -1) {
        if (_Enable5v != -1) { // Feather USB-A's require this
            gpio_init(_Enable5v);
            gpio_set_dir(_Enable5v, GPIO_IN);
            gpio_pull_up(_Enable5v);
        }

        pio_cfg.pin_dp = _DP;
        pio_cfg.pinout = (_Order == 0 ? PIO_USB_PINOUT_DPDM : PIO_USB_PINOUT_DMDP);
        pio_cfg.sm_tx = 1; // NeoPico uses PIO0:0, move to state machine 1
    }
}