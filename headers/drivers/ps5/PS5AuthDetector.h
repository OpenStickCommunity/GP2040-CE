/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_AUTH_DETECTOR_H_
#define _PS5_AUTH_DETECTOR_H_

#include "usblistener.h"
#include "drivers/ps5/PS5Auth.h"

// Upper bound on HID interfaces remembered for (re)binding.
#define PS5_AUTH_MAX_INTERFACES 8

//
// Watches the USB host for supported PS5 auth devices and owns the active
// PS5USBAuth. Host callbacks are forwarded only for the bound interface.
//
// Every HID interface that matches a candidate is remembered, so unplugging
// the bound dongle (or a failed probe) falls back to another one already
// connected. Binding only constructs the provider; it starts talking to the
// device from process(), never from inside a TinyUSB callback.
//
class PS5AuthDetector : public USBListener {
public:
    // Providers need to see stalls (len == 0) to pass them on or recover.
    virtual bool accepts_failed_report_completions() const { return true; }
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
        uint8_t type;       // index into the provider table
        bool rejected;      // probe failed; skip until unplugged
    };

    void unbind();
    void bindNext();

    Candidate candidates[PS5_AUTH_MAX_INTERFACES];
    uint8_t numCandidates = 0;
    int8_t activeIndex = -1;
    PS5USBAuth * active = nullptr;
    uint8_t activeType = 0;         // provider index of `active`
    bool parked = false;            // active provider waiting for its device to re-enumerate
};

#endif // _PS5_AUTH_DETECTOR_H_
