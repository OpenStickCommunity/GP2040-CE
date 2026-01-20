/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS5_DRIVER_H_
#define _PS5_DRIVER_H_

#include "gpdriver.h"
#include "drivers/ps5/PS5Descriptors.h"
#include "drivers/ps5/PS5Auth.h"

typedef enum
{
    PS5_DEFINITION           = 0x03,
    PS5_GET_CALIBRATION      = 0x05,    // Calibration
    PS5_GET_PAIRINFO         = 0x09,    // Pair Information
    PS5_GET_FIRWMARE         = 0x20,    // Firmware Version
    PS5_SET_TEST_PARAM       = 0x80,    // Dual Sense (SET TEST)
    PS5_GET_TEST_PARAM       = 0x81,    // Dual Sense (GET TEST)
    PS5_SET_AUTH_PAYLOAD     = 0xF0,    // Set Auth Payload
    PS5_GET_SIGNATURE_NONCE  = 0xF1,    // Get Signature Nonce
    PS5_GET_SIGNING_STATE    = 0xF2,    // Get Signing State
} PS5AuthReport;

typedef enum
{
    PS5_MAYFLASH_GET_AUTH         = 0x01,    // Mayflash S5 - Get Auth
    PS5_MAYFLASH_AUTH_COMPLETE    = 0x02,    // Mayflash S5 - Auth Complete
    PS5_MAYFLASH_ENC_READY        = 0x03,    // Mayflash S5 - Encryption Ready
} MFS5AuthReport;

class PS5Driver : public GPDriver {
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
    PS5Report ps5Report;
    PS5Report ps5Report_last;
    TouchpadData touchpadData;
    //PSSensor gyroscope;
    //PSSensor accelerometer;
    uint64_t last_report_us;
    PS5Auth * ps5AuthDriver;
    PS5AuthData * ps5AuthData;
    bool pointOneTouched = false;
    bool pointTwoTouched = false;
    uint8_t diff_report_repeat;
    uint8_t touchCounter;
};

#endif // _PS5_DRIVER_H_
