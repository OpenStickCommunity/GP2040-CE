/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _XBOX360HOST_H_
#define _XBOX360HOST_H_

#include <gphost.h>

#include "drivers/xinput/XInputDescriptors.h"

//
// Xbox 360 Controller Host
//
class Xbox360Host : public GPHost {
public:
    static bool match(uint16_t vendor_id, uint16_t product_id);   
    virtual void initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len);
    virtual void gamepad(Gamepad * gamepad);
    virtual void process(uint8_t const* report, uint16_t len);
    virtual void update();
    virtual void shutdown() {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
private:
    void xbox360_set_led(uint8_t quadrant);
    void xinput_set_rumble(uint8_t left, uint8_t right);
    uint8_t last_left_rumble;
    uint8_t last_right_rumble;
    XInputReport prev_report;
};

#endif // _XBOX360HOST_H_
