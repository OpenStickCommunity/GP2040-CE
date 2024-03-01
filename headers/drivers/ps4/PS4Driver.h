/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _PS4_DRIVER_H_
#define _PS4_DRIVER_H_

#include "gpdriver.h"
#include "drivers/ps4/PS4Descriptors.h"

class PS4Driver : public GPDriver {
public:
    virtual void initialize();
    virtual void process(Gamepad * gamepad, uint8_t * outBuffer);
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);
    virtual const uint16_t * get_descriptor_string_cb(uint8_t index, uint16_t langid);
    virtual const uint8_t * get_descriptor_device_cb();
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf) ;
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index);
    virtual const uint8_t * get_descriptor_device_qualifier_cb();
    virtual uint16_t GetJoystickMidValue();
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
};

#endif // _PS4_DRIVER_H_
