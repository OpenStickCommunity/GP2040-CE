/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _DUALSENSEPS5HOST_H_
#define _DUALSENSEPS5HOST_H_

#include <gphost.h>

#include "drivers/p5general/P5GeneralDescriptors.h"

//
// Dualsense PS5 Controller Host
//
class DualsensePS5Host : public GPHost {
public:
    static bool match(uint16_t vendor_id, uint16_t product_id);   
    virtual void initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len);
    virtual void gamepad(Gamepad * gamepad);
    virtual void process(uint8_t const* report, uint16_t len);
    virtual void update() {}
    virtual void shutdown() {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
private:
    P5GenerorReport prevReport;
    uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);
};

#endif // _DUALSENSEPS5HOST_H_
