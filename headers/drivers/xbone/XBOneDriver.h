/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _XBONE_DRIVER_H_
#define _XBONE_DRIVER_H_

#include "gpdriver.h"
#include "drivers/xbone/XBOneDescriptors.h"
#include "drivers/shared/xgip_protocol.h"
#include "drivers/shared/gpauthdriver.h"

class XBOneDriver : public GPDriver {
public:
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
    bool getAuthSent();
private:
    virtual void update();
    void process_report_queue(uint32_t now);
    bool send_xbone_usb(uint8_t const *buffer, uint16_t bufsize);
    void set_ack_wait();
    uint8_t last_report[CFG_TUD_ENDPOINT0_SIZE] = { };
    uint8_t last_report_counter;
    XboxOneGamepad_Data_t xboneReport;
    uint32_t keep_alive_timer;
    uint8_t keep_alive_sequence;
    uint8_t virtual_keycode_sequence;
    bool xb1_guide_pressed;
    GPAuthDriver * authDriver;
};

#endif // _XBONE_DRIVER_H_
