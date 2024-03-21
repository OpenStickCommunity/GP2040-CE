/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS4_DRIVER_H_
#define _PS4_DRIVER_H_

#include "gpdriver.h"
#include "drivers/ps4/PS4Descriptors.h"

// Authentication
#include "drivers/shared/gpauthdriver.h"

typedef enum {
    no_nonce = 0,
    receiving_nonce = 1,
    nonce_ready = 2,
    signed_nonce_ready = 3,
    sending_nonce = 4,
    waiting_reset = 5
} PS4State;

typedef enum
{
    PS4_DEFINITION           = 0x03,    // PS4 Controller Definition
    PS4_SET_AUTH_PAYLOAD     = 0xF0,    // Set Auth Payload
    PS4_GET_SIGNATURE_NONCE  = 0xF1,    // Get Signature Nonce
    PS4_GET_SIGNING_STATE    = 0xF2,    // Get Signing State
    PS4_RESET_AUTH           = 0xF3     // Unknown (PS4 Report 0xF3)
} PS4AuthReport;

// PS4 Auth buffer must be used by callbacks, core0, and core1
// Send back in 56 byte chunks:
//    256 byte - nonce signature
//    16 byte  - ps4 serial
//    256 byte - RSA N
//    256 byte - RSA E
//    256 byte - ps4 signature
//    24 byte  - zero padding

class PS4Driver : public GPDriver {
public:
    PS4Driver(uint32_t type): controllerType(type) {}
    virtual void initialize();
    virtual void process(Gamepad * gamepad, uint8_t * outBuffer);
    virtual void initializeAux();
    virtual void processAux();
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);
    virtual const uint16_t * get_descriptor_string_cb(uint8_t index, uint16_t langid);
    virtual const uint8_t * get_descriptor_device_cb();
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf) ;
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index);
    virtual const uint8_t * get_descriptor_device_qualifier_cb();
    virtual uint16_t GetJoystickMidValue();
    virtual USBListener * get_usb_auth_listener();

    bool getAuthSent() { return authsent;}
private:
    // Lots of things here
    void save_nonce(uint8_t nonce_id, uint8_t nonce_page, uint8_t * buffer, uint16_t buflen);
    uint8_t last_report[CFG_TUD_ENDPOINT0_SIZE] = { };
    uint8_t last_report_counter;
    uint16_t last_axis_counter;
    uint8_t cur_nonce_id;
    PS4Report ps4Report;
    TouchpadData touchpadData;
    uint32_t last_report_timer;
    uint8_t send_nonce_part;
    uint32_t controllerType;
    GPAuthDriver * authDriver;

    PS4State ps4State;
    bool authsent;
    uint8_t nonce_buffer[256];
    uint8_t nonce_id; // used in pass-through mode
};

#endif // _PS4_DRIVER_H_
