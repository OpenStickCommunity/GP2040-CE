/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _XINPUT_DRIVER_H_
#define _XINPUT_DRIVER_H_

#include "gpdriver.h"
#include "usblistener.h"
#include "drivers/shared/gpauthdriver.h"
#include "drivers/xinput/XInputAuth.h"
#include "drivers/xinput/XInputDescriptors.h"

#define XINPUT_OUT_SIZE 32

class XInputDriver : public GPDriver {
public:
    virtual void initialize();
    virtual bool process(Gamepad * gamepad);
    virtual void initializeAux();
    virtual void processAux();
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);
    virtual const uint16_t * get_descriptor_string_cb(uint8_t index, uint16_t langid);
    virtual const uint8_t * get_descriptor_device_cb();
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf) ;
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index);
    virtual const uint8_t * get_descriptor_device_qualifier_cb();
    virtual uint16_t GetJoystickMidValue();
    virtual USBListener * get_usb_auth_listener();
    bool getAuthEnabled();
private:
    uint8_t last_report[CFG_TUD_ENDPOINT0_SIZE] = { };
    XInputReport xinputReport;
    XInputAuth * xAuthDriver;
    uint8_t featureBuffer[XINPUT_OUT_SIZE];
    uint8_t tud_buffer[64];
};

#endif
