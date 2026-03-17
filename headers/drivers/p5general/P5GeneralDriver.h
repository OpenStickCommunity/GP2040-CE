/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _P5GENERAL_DRIVER_H_
#define _P5GENERAL_DRIVER_H_

#include "gpdriver.h"
#include "drivers/p5general/P5GeneralDescriptors.h"
#include "drivers/p5general/P5GeneralAuth.h"

typedef enum
{
    P5GENERAL_DEFINITION           = 0x03,
    P5GENERAL_SET_AUTH_PAYLOAD     = 0xF0,    // Set Auth Payload
    P5GENERAL_GET_SIGNATURE_NONCE  = 0xF1,    // Get Signature Nonce
    P5GENERAL_GET_SIGNING_STATE    = 0xF2,    // Get Signing State
} P5GeneralAuthReport;

class P5GeneralDriver : public GPDriver {
public:
    virtual void initialize();
    virtual bool process(Gamepad * gamepad);
    virtual void initializeAux();
    virtual void processAux();
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) { return false; }
    virtual const uint16_t * get_descriptor_string_cb(uint8_t index, uint16_t langid);
    virtual const uint8_t * get_descriptor_device_cb();
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf) ;
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index);
    virtual const uint8_t * get_descriptor_device_qualifier_cb() { return nullptr; }
    virtual uint16_t GetJoystickMidValue();
    virtual USBListener * get_usb_auth_listener();
    bool getAuthSent() { return false;}
    bool getDongleAuthRequired();
private:
    P5GenerorReport p5GeneralReport;
    P5GenerorReport p5GeneralReport_last;
    TouchpadData touchpadData;
    //PSSensor gyroscope;
    //PSSensor accelerometer;
    uint64_t last_report_us;
    P5GeneralAuth * p5GeneralAuthDriver;
    P5GeneralAuthData * p5GeneralAuthData;
    bool pointOneTouched = false;
    bool pointTwoTouched = false;
    uint8_t diff_report_repeat;
    uint8_t touchCounter;
};

#endif // _P5GENERAL_DRIVER_H_
