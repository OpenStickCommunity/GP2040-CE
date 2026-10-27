/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_DRIVER_H_
#define _PS5_DRIVER_H_

#include "gpdriver.h"
#include "drivers/ps5/PS5Descriptors.h"
#include "drivers/ps5/PS5AuthDetector.h"

class PS5Driver : public GPDriver {
public:
    virtual void initialize();
    virtual bool process(Gamepad * gamepad);
    virtual void initializeAux();
    virtual void processAux() {}
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t * buffer, uint16_t reqlen);
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const * buffer, uint16_t bufsize);
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request) { return false; }
    virtual const uint16_t * get_descriptor_string_cb(uint8_t index, uint16_t langid);
    virtual const uint8_t * get_descriptor_device_cb();
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf);
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index);
    virtual const uint8_t * get_descriptor_device_qualifier_cb() { return nullptr; }
    virtual uint16_t GetJoystickMidValue();
    virtual USBListener * get_usb_auth_listener();

    bool getAuthSent();             // an auth provider is bound and ready
    bool getDongleAuthRequired();   // USB host carries the auth device

private:
    PS5Auth * readyAuth() const;
    void buildReport(Gamepad * gamepad);
    bool sendSigned(PS5Auth * auth, bool due);
    bool sendUnsigned(bool due);
    uint16_t getDefaultFeature(uint8_t report_id, uint8_t * buffer, uint16_t reqlen);

    PS5AuthDetector authDetector;
    bool usbHostAuth = false;

    PS5Report ps5Report;
    PS5Report lastReport;
    TouchpadData touchpadData;
    bool pointOneTouched = false;
    bool pointTwoTouched = false;
    uint8_t touchCounter = 0;

    uint8_t repeatCount = 0;        // Re-send a changed report a few times
    uint64_t lastSentUs = 0;

    bool authWasReady = false;      // For logging ready/lost transitions
    uint32_t signedSent = 0;        // Signed reports sent (logging)
    uint32_t submitted = 0;         // Reports handed to the dongle (logging)
    uint64_t lastSignedUs = 0;      // Last signed report sent (stall logging)
    bool stallLogged = false;

    uint8_t signedOut[PS5_ENDPOINT_SIZE];
    bool signedOutPending = false;  // Signed report waiting for the IN endpoint

    // Last challenge seen, so "not ready" can be answered without a provider
    uint8_t consoleFrameId = 0;
    uint8_t consoleAuthType = 0;
};

#endif // _PS5_DRIVER_H_
