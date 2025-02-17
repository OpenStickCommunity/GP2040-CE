/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _GPDRIVER_H_
#define _GPDRIVER_H_

#include "enums.pb.h"

#include "gamepad.h"

#include "tusb_config.h"
#include "tusb.h"
#include "class/hid/hid.h"
#include "device/usbd_pvt.h"

#include "usblistener.h"

// Forward declare gamepad
class Gamepad;

//
// GP2040-CE USB Device Class Driver
//
class GPDriver {
public:
    virtual void initialize() = 0;
    virtual void initializeAux() = 0;
    virtual bool process(Gamepad * gamepad) = 0;
    virtual void processAux() = 0;
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) = 0;
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) = 0;
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) = 0;
    virtual const uint16_t * get_descriptor_string_cb(uint8_t index, uint16_t langid) = 0;
    virtual const uint8_t * get_descriptor_device_cb() = 0;
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf) = 0;
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index) = 0;
    virtual const uint8_t * get_descriptor_device_qualifier_cb() = 0;
    virtual uint16_t GetJoystickMidValue() = 0;
    const usbd_class_driver_t * get_class_driver() { return &class_driver; }
    virtual USBListener * get_usb_auth_listener() = 0;
protected:
    usbd_class_driver_t class_driver;
};

#endif
