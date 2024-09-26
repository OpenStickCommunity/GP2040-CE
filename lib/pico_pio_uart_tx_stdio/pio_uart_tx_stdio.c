#include "pio_uart_tx_stdio.h"
#include "pico/stdio/driver.h"
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "uart_tx.pio.h"

static PIO pio = pio0;
static uint sm = 0;

static void pio_uart_tx_stdio_out_chars(const char *buf, int len) {
	for (int i = 0; i < len; i++) {
		uart_tx_program_putc(pio, sm, buf[i]);
	}
}

static void pio_uart_tx_stdio_out_flush(void) {
	return;
}

static int pio_uart_tx_stdio_in_chars(char *buf, int len) {
	return 0;
}

static void pio_uart_tx_stdio_set_chars_available_callback(void (*fn)(void*), void *param) {
	return;
}

static stdio_driver_t pio_uart_tx_stdio_driver = {
	.out_chars = pio_uart_tx_stdio_out_chars,
	.out_flush = pio_uart_tx_stdio_out_flush,
	.in_chars = pio_uart_tx_stdio_in_chars,
#if PICO_STDIO_UART_SUPPORT_CHARS_AVAILABLE_CALLBACK
	.set_chars_available_callback = pio_uart_tx_stdio_set_chars_available_callback,
#endif
#if PICO_STDIO_ENABLE_CRLF_SUPPORT
	.crlf_enabled = PIO_UART_TX_STDIO_DEFAULT_CRLF
#endif
};

void pio_uart_tx_stdio_setup(void) {
	uint offset = pio_add_program(pio, &uart_tx_program);
	uart_tx_program_init(pio, sm, offset, PIO_UART_TX_STDIO_PIN, PIO_UART_TX_STDIO_BAUDRATE);
	stdio_set_driver_enabled(&pio_uart_tx_stdio_driver, true);
	// stdio_filter_driver(&pio_uart_tx_stdio_driver);
}
