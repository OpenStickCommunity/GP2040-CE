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
    virtual void update();
    virtual void send_report(Gamepad * gamepad);
    virtual void receive_report(uint8_t *buffer);
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);
    virtual bool control_xfer_cb(uint8_t rhport, uint8_t stage,
                                tusb_control_request_t const *request);
    virtual const uint16_t * get_device_string(uint8_t index, uint16_t langid);
    virtual const uint8_t * get_device_descriptor();
    virtual const uint8_t * get_report_descriptor(uint8_t itf) ;
    virtual const uint8_t * get_config_descriptor(uint8_t index);
    virtual const uint8_t * get_device_qualifier();
    virtual uint16_t GetJoystickMidValue();
private:
    // Lots of things here
	uint8_t last_report[CFG_TUD_ENDPOINT0_SIZE] = { };
	uint8_t last_report_counter = 0;
	uint16_t last_axis_counter = 0;
    PS4Report ps4Report;
    TouchpadData touchpadData;
    uint32_t keep_alive_timer;
	uint8_t keep_alive_sequence;
};

#endif // _PS4_DRIVER_H_
