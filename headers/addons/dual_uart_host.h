#pragma once

#include "gpaddon.h"

#ifndef DUAL_UART_HOST_ENABLED
#define DUAL_UART_HOST_ENABLED 1
#endif

#define DualUartHostName "Dual UART Host"

class DualUartHostAddon : public GPAddon {
public:
    bool available() override;
    void setup() override;
    void preprocess() override;
    void process() override {}
    void postprocess(bool) override {}
    void reinit() override;
    std::string name() override { return DualUartHostName; }
};
