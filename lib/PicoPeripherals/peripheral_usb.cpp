#include "peripheral_usb.h"

PeripheralUSB::PeripheralUSB() {

}

PeripheralUSB::PeripheralUSB(uint8_t block, uint8_t dp, uint8_t order, int8_t enable5V) {
    if (block < NUM_USBS) {
        //_SPI = _hardwareBlocks[block];
        _Order = order;
        _DP = dp;
        _Enable5V = enable5V;
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