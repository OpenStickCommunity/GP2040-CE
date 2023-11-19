#include "peripheral_usb.h"

PeripheralUSB::PeripheralUSB() {

}

PeripheralUSB::PeripheralUSB(uint8_t block, int8_t dp, int8_t enable5v, uint8_t order) {
    if (block < NUM_USBS) {
        //_SPI = _hardwareBlocks[block];
        _DP = dp;
        _Enable5v = enable5v;
        _Order = order;
        setup();
    } else {
        // currently not supported
    }
}

void PeripheralUSB::setup() {
    //if (_Enable5V != -1) { // Feather USB-A's require this
    //    gpio_init(_Enable5V);
    //    gpio_set_dir(_Enable5V, GPIO_IN);
    //    gpio_pull_up(_Enable5V);
    //}
    //
    //USBHostManager::getInstance().setDataPin(_DP);
}