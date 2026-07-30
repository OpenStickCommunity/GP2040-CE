/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _GPHOST_H_
#define _GPHOST_H_

#include <cstdint>

#include "gamepad/GamepadState.h"
#include "gamepad.h"

//
// GP2040-CE USB Device Class Driver
//
class GPHost {
public:
    virtual ~GPHost() { }
    virtual void initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len) = 0;
    virtual void process(uint8_t const* report, uint16_t len) = 0;
    virtual void update() = 0;
    virtual void gamepad(Gamepad * gamepad) = 0;
    virtual void shutdown() = 0;
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) = 0;
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) = 0;
protected:
    GamepadState _controller_host_state;
    uint8_t _dev_addr;
    uint8_t _instance;
    uint16_t _vendor_id;
    uint16_t _product_id;
};

#endif
