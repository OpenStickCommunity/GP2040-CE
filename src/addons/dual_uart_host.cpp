#include "addons/dual_uart_host.h"
#include "addons/dual_uart_listener.h"
#include "storagemanager.h"

bool DualUartHostAddon::available() {
// #if DUAL_UART_HOST_ENABLED
    // До появления dual_input_addon — просто флаг сборки
    return true;
// #else
    // return false;
// #endif
}

void DualUartHostAddon::setup() {
    listener = new DualUartListener();
    listener->setup();
}

void DualUartHostAddon::preprocess() {
    if (listener) {
        ((DualUartListener*)listener)->process();
    }
}

void DualUartHostAddon::reinit() {
    delete listener;
    setup();
}
