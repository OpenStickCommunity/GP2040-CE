/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _SWITCHPROHOST_H_
#define _SWITCHPROHOST_H_

#include <gphost.h>

#include "drivers/switchpro/SwitchProDescriptors.h"
#include "drivers/switchpro/SwitchProDriver.h"

typedef struct __attribute__((packed)) {
    uint8_t command;
    uint8_t counter;
    uint8_t rumble_l[4];
    uint8_t rumble_r[4];
    uint8_t subcommand;
    uint8_t subcommand_args[3];
} SwitchProHostReport;

//
// GP2040-CE USB Host Driver
//
class SwitchProHost : public GPHost {
public:
    static bool match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len);   
    virtual void initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len);
    virtual void gamepad(Gamepad * gamepad);
    virtual void process(uint8_t const* report, uint16_t len);
    virtual void update();
    virtual void shutdown() {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
private:
    void setup_switch_pro(uint8_t const *report, uint16_t len);    
    uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);
    SwitchProReport prevReport;
    SwitchOutputSubtypes switchProState;
    bool switchProFinished;
    uint8_t switchReportCounter;
    uint8_t lastSwitchLed;
};

#endif
