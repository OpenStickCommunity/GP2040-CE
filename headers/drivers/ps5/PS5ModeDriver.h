/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_MODE_DRIVER_H_
#define _PS5_MODE_DRIVER_H_

#include "gpdriver.h"
#include "usblistener.h"
#include "drivers/ps5/PS5Driver.h"
#include "drivers/ps4/PS4Driver.h"

//
// Forwards USB host events to both auth listeners (PS5 dongle detection and
// the PS4 auth listener), so either path can find its device.
//
class PS5ModeListener : public USBListener {
public:
    void set(USBListener * first, USBListener * second) { a = first; b = second; }
    bool empty() const { return a == nullptr && b == nullptr; }

    virtual bool accepts_failed_report_completions() const {
        return (a && a->accepts_failed_report_completions()) || (b && b->accepts_failed_report_completions());
    }
    virtual void setup() {}
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const * desc_report, uint16_t desc_len) {
        if (a) a->mount(dev_addr, instance, desc_report, desc_len);
        if (b) b->mount(dev_addr, instance, desc_report, desc_len);
    }
    virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
        if (a) a->xmount(dev_addr, instance, controllerType, subtype);
        if (b) b->xmount(dev_addr, instance, controllerType, subtype);
    }
    virtual void unmount(uint8_t dev_addr) {
        if (a) a->unmount(dev_addr);
        if (b) b->unmount(dev_addr);
    }
    virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const * report, uint16_t len) {
        if (a) a->report_received(dev_addr, instance, report, len);
        if (b) b->report_received(dev_addr, instance, report, len);
    }
    virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const * report, uint16_t len) {
        if (a) a->report_sent(dev_addr, instance, report, len);
        if (b) b->report_sent(dev_addr, instance, report, len);
    }
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
        if (a && (len != 0 || a->accepts_failed_report_completions())) a->set_report_complete(dev_addr, instance, report_id, report_type, len);
        if (b && (len != 0 || b->accepts_failed_report_completions())) b->set_report_complete(dev_addr, instance, report_id, report_type, len);
    }
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
        if (a && (len != 0 || a->accepts_failed_report_completions())) a->get_report_complete(dev_addr, instance, report_id, report_type, len);
        if (b && (len != 0 || b->accepts_failed_report_completions())) b->get_report_complete(dev_addr, instance, report_id, report_type, len);
    }

private:
    USBListener * a = nullptr;
    USBListener * b = nullptr;
};

//
// INPUT_MODE_PS5: use the third-party PS5 path (PS5Driver + P5General /
// MagicBoots S5) when it can authenticate, otherwise fall back to the PS4
// arcade-stick path (PS4Driver(PS4_ARCADESTICK), PS4 dongle or key files).
//
//  - At boot the console connection is held for up to PS5_MODE_DECIDE_US.
//    A PS5 dongle that becomes ready in that window selects the PS5 path;
//    otherwise the PS4 path is used.
//  - On the PS5 path, if the console starts auth but does not complete it,
//    the device disconnects and re-enumerates as the PS4 arcade stick.
//
// The console only ever sees one identity at a time; the class driver and
// descriptors follow the active path.
//
class PS5ModeDriver : public GPDriver {
public:
    PS5ModeDriver() : ps4(PS4_ARCADESTICK) {}

    virtual void initialize();
    virtual void initializeAux();
    virtual bool process(Gamepad * gamepad);
    virtual void processAux();
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t * buffer, uint16_t reqlen);
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const * buffer, uint16_t bufsize);
    virtual void set_report_n(uint8_t itf, uint8_t report_id, hid_report_type_t report_type, uint8_t const * buffer, uint16_t bufsize);
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request);
    virtual const uint16_t * get_descriptor_string_cb(uint8_t index, uint16_t langid);
    virtual const uint8_t * get_descriptor_device_cb();
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf);
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index);
    virtual const uint8_t * get_descriptor_device_qualifier_cb();
    virtual const uint8_t * get_descriptor_other_speed_configuration_cb(uint8_t index);
    virtual uint16_t GetJoystickMidValue();
    virtual USBListener * get_usb_auth_listener();

    bool getAuthSent();
    bool getDongleAuthRequired();

    // Status bar text, padded to 6 characters:
    //   PS5P5G / PS5MS5  PS5 path, console auth done via P5General / MagicBoots S5
    //   PS5:AS           PS4 path, auth done via PS4 USB dongle
    //   PS4:AS           PS4 path, auth done via PS4 key files
    //   PS5              no completed auth
    const char * statusLabel();

private:
    enum class Path : uint8_t { Deciding, PS5, ReconnectPS5, SwitchingToPS4, PS4 };

    GPDriver * active();
    void connectAs(Path path);

    PS5Driver ps5;
    PS4Driver ps4;
    PS5ModeListener listener;

    Path path = Path::Deciding;
    bool usbHostAuth = false;
    uint64_t deadlineUs = 0;    // Decide window / switch hold-off end
    uint64_t decideStartUs = 0;
    char label[8];
};

#endif // _PS5_MODE_DRIVER_H_
