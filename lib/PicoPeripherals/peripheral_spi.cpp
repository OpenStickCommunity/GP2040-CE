#include "peripheral_spi.h"

PeripheralSPI::PeripheralSPI()
{
#ifdef PICO_DEFAULT_SPI_INSTANCE

#if PICO_SDK_VERSION_MAJOR >= 2
    _SPI = PICO_DEFAULT_SPI_INSTANCE();
#elif defined(PICO_DEFAULT_SPI_INSTANCE)
    _SPI = PICO_DEFAULT_SPI_INSTANCE;
#endif

    _TX = PICO_DEFAULT_SPI_TX_PIN;
    _RX = PICO_DEFAULT_SPI_RX_PIN;
    _SCK = PICO_DEFAULT_SPI_SCK_PIN;
    _CS = PICO_DEFAULT_SPI_CSN_PIN;
    _Speed = SPI_DEFAULT_SPEED;
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
    }
    else {
        // currently not supported
    }
}

void PeripheralSPI::setup() {
    if (initialized) {
        spi_deinit(_SPI);
    }

    spi_init(_SPI, _Speed);
    initialized = true;

    gpio_set_function(_SCK, GPIO_FUNC_SPI);
    gpio_set_function(_TX, GPIO_FUNC_SPI);
    gpio_set_function(_RX, GPIO_FUNC_SPI);
    gpio_pull_up(_RX);

    if (_UseDMA) {
        // DMA configuration - 2 channels (TX/RX)
        _dmaRxChannel = dma_claim_unused_channel(true);
        _dmaRxBuf = new uint8_t[DMA_BUFFER_SIZE]();
        dma_channel_config rxConfig = dma_channel_get_default_config(_dmaRxChannel);
        channel_config_set_transfer_data_size(&rxConfig, DMA_SIZE_8);
        channel_config_set_dreq(&rxConfig, spi_get_dreq(_SPI, false));
        channel_config_set_read_increment(&rxConfig, false);
        dma_channel_configure(
            _dmaRxChannel,         // Channel to be configured
            &rxConfig,             // The configuration we just created
            _dmaRxBuf,             // The initial write address
            &spi_get_hw(_SPI)->dr, // The initial read address
            DMA_BUFFER_SIZE,       // Element count (each element is of size transfer_data_size)
            false                  // Don't start immediately.
        );

        _dmaTxChannel = dma_claim_unused_channel(true);
        _dmaTxBuf = new uint8_t[DMA_BUFFER_SIZE]();
        dma_channel_config txConfig = dma_channel_get_default_config(_dmaTxChannel);
        channel_config_set_transfer_data_size(&txConfig, DMA_SIZE_8);
        channel_config_set_dreq(&txConfig, spi_get_dreq(_SPI, true));
        channel_config_set_write_increment(&txConfig, false);
        dma_channel_configure(
            _dmaTxChannel,         // Channel to be configured
            &txConfig,             // The configuration we just created
            &spi_get_hw(_SPI)->dr, // The initial write address
            _dmaTxBuf,             // The initial read address
            DMA_BUFFER_SIZE,       // Element count (each element is of size transfer_data_size)
            false                  // Don't start immediately.
        );

        dma_start_channel_mask((1u << _dmaTxChannel) | (1u << _dmaRxChannel));
    }
}

void PeripheralSPI::deactivate() {
    if (_UseDMA) {
        dma_channel_unclaim(_dmaRxChannel);
        dma_channel_unclaim(_dmaTxChannel);
        _dmaRxBuf = nullptr;
        _dmaTxBuf = nullptr;
    }
}

void PeripheralSPI::transfer(const uint8_t *tx, uint8_t *rx, size_t count) {
    if (tx == nullptr) {
        spi_read_blocking(_SPI, 0xFF, rx, count);
        return;
    }
    if (rx == nullptr) {
        spi_write_blocking(_SPI, tx, count);
        return;
    }

    spi_write_read_blocking(_SPI, tx, rx, count);
}

uint8_t PeripheralSPI::transfer(uint8_t tx) {
    uint8_t rx;
    spi_write_read_blocking(_SPI, &tx, &rx, 1);
    return rx;
}

uint16_t PeripheralSPI::transfer16(uint16_t tx) {
    uint16_t rx;
    spi_write16_read16_blocking(_SPI, &tx, &rx, 1);
    return rx;
}

void PeripheralSPI::select(int8_t cs) {
    _CSActive = cs > -1 ? cs : _CS;
    gpio_pull_down(_CSActive);
}

void PeripheralSPI::deselect() {
    if (_CSActive > -1) {
        gpio_pull_up(_CSActive);
        _CSActive = -1;
    }
}

void PeripheralSPI::beginTransaction(uint32_t speedMHz, spi_order_t bitOrder, SPIMode spiMode) {
    bool hasInitChange = !initialized || speedMHz != _Speed;
    bool hasFormatChange = bitOrder != _BitOrder || spiMode != _SpiMode;

    if (hasInitChange || hasFormatChange) {
        uint32_t flags = save_and_disable_interrupts();

        if (hasInitChange) {
            // if (initialized) {
            //     spi_deinit(_SPI);
            // }

            _Speed = speedMHz;
            spi_init(_SPI, _Speed);
            initialized = true;

        }

        if (hasFormatChange) {
            _BitOrder = bitOrder;
            _Cpol = get_cpol(spiMode);
            _Cpha = get_cpha(spiMode);
            spi_set_format(_SPI, 8, _Cpol, _Cpha, _BitOrder);
        }

        restore_interrupts(flags);
    }

}

void PeripheralSPI::endTransaction() {
    (void)0;
}
