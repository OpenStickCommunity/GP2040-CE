/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_AUTH_DETECTOR_H_
#define _PS5_AUTH_DETECTOR_H_

#include "usblistener.h"
#include "drivers/ps5/PS5Auth.h"

#define PS5_AUTH_MAX_INTERFACES 8

class PS5AuthDetector : public USBListener {
public:
    virtual void setup();
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const * desc_report, uint16_t desc_len);
    virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {}
    virtual void unmount(uint8_t dev_addr);
    virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const * report, uint16_t len);
    virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const * report, uint16_t len) {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);

    void process();
    PS5Auth * auth() const { return active; }

private:
    struct Candidate {
        uint8_t dev_addr;
        uint8_t instance;
        uint8_t type;
        bool rejected;
    };

    void unbind();
    void bindNext();

    Candidate candidates[PS5_AUTH_MAX_INTERFACES];
    uint8_t numCandidates = 0;
    int8_t activeIndex = -1;
    PS5USBAuth * active = nullptr;
};

#endif // _PS5_AUTH_DETECTOR_H_
