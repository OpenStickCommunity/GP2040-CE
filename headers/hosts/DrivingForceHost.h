/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _DRIVINGFORCEHOST_H_
#define _DRIVINGFORCEHOST_H_

#include <gphost.h>

#include "drivers/ps3/PS3Descriptors.h"

//
// GP2040-CE USB Host Driver
//
class DrivingForceHost : public GPHost {
public:
    static bool match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len);   
    virtual void initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len);
    virtual void gamepad(Gamepad * gamepad);
    virtual void process(uint8_t const* report, uint16_t len);
    virtual void update() {}
    virtual void shutdown() {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
private:
};

#endif
