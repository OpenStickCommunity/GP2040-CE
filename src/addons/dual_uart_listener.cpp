#include "addons/dual_uart_listener.h"

#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "crc.h"

// ===== UART configuration =====

#define DUAL_UART uart1
#define DUAL_UART_BAUDRATE 4000000

#ifndef SERIAL_TX_PIN
#define SERIAL_TX_PIN 20
#endif

#ifndef SERIAL_RX_PIN
#define SERIAL_RX_PIN 21
#endif

#ifndef SERIAL_CTS_PIN
#define SERIAL_CTS_PIN 26
#endif

#ifndef SERIAL_RTS_PIN
#define SERIAL_RTS_PIN 27
#endif

// ===== SLIP =====

#define END     0300
#define ESC     0333
#define ESC_END 0334
#define ESC_ESC 0335

#define MAX_PAYLOAD 512

// ===== Implementation =====

void DualUartListener::setup() {
    uart_init(DUAL_UART, DUAL_UART_BAUDRATE);
    uart_set_hw_flow(DUAL_UART, true, true);
    uart_set_translate_crlf(DUAL_UART, false);

    gpio_set_function(SERIAL_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(SERIAL_RX_PIN, GPIO_FUNC_UART);
    gpio_set_function(SERIAL_CTS_PIN, GPIO_FUNC_UART);
    gpio_set_function(SERIAL_RTS_PIN, GPIO_FUNC_UART);
}

void DualUartListener::process() {
    serialRead();
}

bool DualUartListener::serialRead() {
    static uint8_t buffer[MAX_PAYLOAD + 32];
    static uint16_t bytesRead = 0;
    static bool escaped = false;

    while (uart_is_readable(DUAL_UART)) {
        char c = uart_getc(DUAL_UART);

        if (escaped) {
            buffer[bytesRead++] = (c == ESC_END) ? END :
                                  (c == ESC_ESC) ? ESC : c;
            escaped = false;
        } else {
            if (c == END) {
                if (bytesRead > 4) {
                    uint32_t crcCalc = crc32(buffer, bytesRead - 4);
                    uint32_t crcRecv = 0;
                    for (int i = 0; i < 4; i++) {
                        crcRecv = (crcRecv << 8) | buffer[bytesRead - 1 - i];
                    }
                    if (crcCalc == crcRecv) {
                        handleMessage(buffer, bytesRead - 4);
                    }
                }
                bytesRead = 0;
            } else if (c == ESC) {
                escaped = true;
            } else {
                buffer[bytesRead++] = c;
            }
        }

        bytesRead %= sizeof(buffer);
    }

    return false;
}

bool DualUartListener::handleMessage(const uint8_t* data, uint16_t len) {
    if (!len) return false;

    switch ((DualCommand)data[0]) {

    case DualCommand::DEVICE_CONNECTED: {
        auto* msg = (const device_connected_t*)data;
        mount(
            msg->dev_addr,
            msg->interface,
            msg->report_descriptor,
            len - sizeof(device_connected_t)
        );
        break;
    }

    case DualCommand::DEVICE_DISCONNECTED: {
        auto* msg = (const device_disconnected_t*)data;
        unmount(msg->dev_addr, msg->interface);
        break;
    }

    case DualCommand::REPORT_RECEIVED: {
        auto* msg = (const report_received_t*)data;
        report_received(
            msg->dev_addr,
            msg->interface,
            msg->report,
            len - sizeof(report_received_t)
        );
        break;
    }

    default:
        break;
    }

    return true;
}
