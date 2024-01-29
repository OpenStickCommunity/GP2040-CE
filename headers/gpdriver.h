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

//
// GP2040-CE USB Device Class Driver
//
class GPDriver {
public:
    virtual void initialize() = 0;
    virtual void update() = 0;
    virtual void send_report(Gamepad * gamepad) = 0;
    virtual void receive_report(uint8_t *buffer) = 0;
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) = 0;
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) = 0;
    virtual bool control_xfer_cb(uint8_t rhport, uint8_t stage,
                                tusb_control_request_t const *request) = 0;
    virtual const uint16_t * get_device_string(uint8_t index, uint16_t langid) = 0;
    virtual const uint8_t * get_device_descriptor() = 0;
    virtual const uint8_t * get_report_descriptor(uint8_t itf) = 0;
    virtual const uint8_t * get_config_descriptor(uint8_t index) = 0;
    virtual const uint8_t * get_device_qualifier() = 0;
    virtual uint16_t GetJoystickMidValue() = 0;
    usbd_class_driver_t * get_class_driver() { return class_driver; }
private:
    usbd_class_driver_t * class_driver;
    InputMode inputMode;
};

#endif
