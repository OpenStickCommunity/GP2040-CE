/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_DRIVER_H_
#define _PS5_DRIVER_H_

#include "gpdriver.h"
#include "drivers/ps5/PS5Descriptors.h"
#include "drivers/ps5/PS5AuthDetector.h"

//
// PS5 mode: presents a licensed third-party PS5 controller and builds its
// input reports.
//
// Authentication is optional and external. Without a ready auth provider the
// controller still works (unsigned reports) until the console drops it for
// failing auth. When a provider becomes ready, reports are routed through it
// for signing and the console's auth feature reports are forwarded to it.
//
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

    bool getAuthSent();             // the console completed auth with the bound provider
    bool getDongleAuthRequired();   // USB host carries the auth device

    // Bound provider that is ready to sign, or nullptr.
    PS5Auth * readyAuth() const;

    // The console started auth but did not complete it in time. Used by
    // PS5ModeDriver to fall back to the PS4 arcade-stick path.
    bool consoleAuthFailed() const;

    // Lock the console-facing identity to the bound provider's (third-party
    // when none). Call while disconnected, right before connecting.
    void selectIdentity();
    // A ready provider needs a different identity than the one presented.
    bool identityMismatch() const;

    // A provider is bound but still initializing (used to extend the boot wait).
    bool authInitializing() const;

    // A PS4-mode auth dongle is attached (PS5 mode should use the PS4 path).
    bool ps4DongleAttached() const { return authDetector.ps4DongleAttached(); }

    // Class driver hooks (called through static trampolines).
    uint16_t classOpen(uint8_t rhport, tusb_desc_interface_t const * itf, uint16_t max_len);
    void classReset(uint8_t rhport);
    bool classControlXfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request);

private:
    void buildReport(Gamepad * gamepad);
    bool sendSigned(PS5Auth * auth, bool due);
    bool sendUnsigned(bool due);
    uint16_t getDefaultFeature(uint8_t report_id, uint8_t * buffer, uint16_t reqlen);
    void trackConsoleAuth(PS5Auth * auth, uint8_t report_id, const uint8_t * buffer, uint16_t len);
    void serviceProxy();
    bool audioControlXfer(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request);
    void resetAudio();

    PS5AuthDetector authDetector;
    const PS5Identity * identity = &ps5ThirdPartyIdentity;
    bool usbHostAuth = false;

    PS5Report ps5Report;
    PS5Report lastReport;
    TouchpadData touchpadData;
    bool pointOneTouched = false;
    bool pointTwoTouched = false;
    uint8_t touchCounter = 0;

    uint8_t repeatCount = 0;        // Re-send a changed report a few times
    uint64_t lastSentUs = 0;

    bool authWasReady = false;      // A provider was ready on the last pass (detects ready/lost)
    bool consoleAuthDone = false;   // Console completed auth with the bound provider

    uint8_t signedOut[PS5_ENDPOINT_SIZE];
    bool signedOutPending = false;  // Signed report waiting for the IN endpoint

    uint64_t consoleAuthStartUs = 0; // First console F0 of the current provider (0 = none yet)

    // Console feature read held while a provider fetches it live
    bool proxyPending = false;
    uint8_t proxyRhport = 0;
    uint8_t proxyReportId = 0;
    tusb_control_request_t proxyRequest {};
    uint8_t proxyBuffer[PS5_ENDPOINT_SIZE];
    uint64_t proxyStartUs = 0;

    // Audio interface state for the composite DualSense identity
    uint8_t audioAlt[3] = {0, 0, 0};
    uint8_t audioCurrent[2][2];
    uint8_t audioBuffer[8];
    uint8_t audioPendingIndex = 0xFF;

    // Last challenge seen, so "not ready" can be answered without a provider
    uint8_t consoleFrameId = 0;
    uint8_t consoleAuthType = 0;
};

#endif // _PS5_DRIVER_H_
