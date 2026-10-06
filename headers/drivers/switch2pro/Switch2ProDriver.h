/*
 * SPDX-License-Identifier: MIT
 * Nintendo Switch 2 Pro Controller (057E:2069) wired USB emulation.
 */

#ifndef _SWITCH2_PRO_DRIVER_H_
#define _SWITCH2_PRO_DRIVER_H_

#include "gpdriver.h"
#include "drivers/switch2pro/Switch2ProDescriptors.h"

#define SWITCH2_PRO_KEEPALIVE_MS 50

class Switch2ProDriver : public GPDriver {
public:
    virtual void initialize();
    virtual bool process(Gamepad * gamepad);
    virtual void initializeAux() {}
    virtual void processAux() {}
    virtual uint16_t get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen);
    virtual void set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize);
    virtual bool vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);
    virtual const uint16_t * get_descriptor_string_cb(uint8_t index, uint16_t langid);
    virtual const uint8_t * get_descriptor_device_cb();
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf);
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index);
    virtual const uint8_t * get_descriptor_device_qualifier_cb();
    virtual const uint8_t * get_descriptor_other_speed_configuration_cb(uint8_t index);
    virtual uint16_t GetJoystickMidValue();
    virtual USBListener * get_usb_auth_listener() { return nullptr; }
private:
    uint8_t inputReport[SWITCH2_PRO_INPUT_REPORT_LEN] = { };
    uint8_t lastSentState[11] = { };
    uint32_t lastSentMs = 0;
    uint8_t reportCounter = 0;
};

#endif // _SWITCH2_PRO_DRIVER_H_
