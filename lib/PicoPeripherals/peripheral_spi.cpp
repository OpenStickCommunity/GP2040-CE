#include "peripheral_spi.h"

PeripheralSPI::PeripheralSPI() {
#ifdef PICO_DEFAULT_SPI_INSTANCE
    _SPI = PICO_DEFAULT_SPI_INSTANCE;
    _TX = PICO_DEFAULT_SPI_TX_PIN;
    _RX = PICO_DEFAULT_SPI_RX_PIN;
    _SCK = PICO_DEFAULT_SPI_SCK_PIN;
    _CS = PICO_DEFAULT_SPI_CSN_PIN;
#endif
}

void PeripheralSPI::setConfig(uint8_t block, uint8_t tx, uint8_t rx, uint8_t sck, uint8_t cs) {
    if (block < NUM_SPIS) {
        _SPI = _hardwareBlocks[block];
        _TX = tx;
        _RX = rx;
        _SCK = sck;
        _CS = cs;
        configured = true;
        setup();
    } else {
        // currently not supported
    }
}

void PeripheralSPI::setup() {
    spi_init(spi_default, 1000 * 1000);
    gpio_set_function(_TX, GPIO_FUNC_SPI);
    gpio_set_function(_RX, GPIO_FUNC_SPI);
    gpio_set_function(_SCK, GPIO_FUNC_SPI);
    gpio_init(_CS);
}