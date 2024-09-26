#ifndef _PIO_UART_TX_STDIO_H_
#define _PIO_UART_TX_STDIO_H_

#ifndef PIO_UART_TX_STDIO_PIN
#define PIO_UART_TX_STDIO_PIN 0
#endif
#ifndef PIO_UART_TX_STDIO_BAUDRATE
#define PIO_UART_TX_STDIO_BAUDRATE 115200
#endif

#ifndef PIO_UART_TX_STDIO_DEFAULT_CRLF
#define PIO_UART_TX_STDIO_DEFAULT_CRLF PICO_STDIO_DEFAULT_CRLF
#endif

void pio_uart_tx_stdio_setup(void);

#endif
