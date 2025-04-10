/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS4_DRIVER_H_
#define _PS4_DRIVER_H_

#include "gpdriver.h"
#include "drivers/ps4/PS4Descriptors.h"

// Authentication
#include "drivers/ps4/PS4Auth.h"

typedef enum
{
    PS4_GET_CALIBRATION      = 0x02,    // PS4 Controller Calibration
    PS4_DEFINITION           = 0x03,    // PS4 Controller Definition
    PS4_SET_FEATURE_STATE    = 0x05,    // PS4 Controller Features
    PS4_GET_MAC_ADDRESS      = 0x12,    // PS4 Controller MAC
    PS4_SET_HOST_MAC         = 0x13,    // Set Host MAC
    PS4_SET_USB_BT_CONTROL   = 0x14,    // Set USB/BT Control Mode
    PS4_GET_VERSION_DATE     = 0xA3,    // PS4 Controller Version & Date
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
    virtual bool process(Gamepad * gamepad);
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
    bool getDongleAuthRequired();
private:
    uint8_t last_report[CFG_TUD_ENDPOINT0_SIZE] = { };
    uint8_t last_report_counter;
    uint16_t last_axis_counter;
    PS4Report ps4Report;
    TouchpadData touchpadData;
    PSSensorData sensorData;
    uint32_t last_report_timer;
    PS4Auth * ps4AuthDriver;
    PS4AuthData * ps4AuthData;      // PS4 Authentication Data
    uint8_t cur_nonce_chunk;            // PS4 Encryption Nonce Chunk (Max 19)
    uint8_t cur_nonce_id;
    uint32_t controllerType;        // PS4 DS4 / PS5 Third-Party
    bool pointOneTouched = false;
    bool pointTwoTouched = false;
    uint8_t touchCounter;
    PS4FeatureOutputReport ps4Features;
    uint8_t lastFeatures[PS4_FEATURES_SIZE] = { };
    uint8_t deviceDescriptor[sizeof(ps4_device_descriptor)];
    bool authsent;
};

#endif // _PS4_DRIVER_H_
