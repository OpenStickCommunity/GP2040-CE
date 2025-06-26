/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _SWITCH_PRO_DRIVER_H_
#define _SWITCH_PRO_DRIVER_H_

#include <map>
#include <vector>
#include "gpdriver.h"
#include "drivers/switchpro/SwitchProDescriptors.h"

#define SWITCH_PRO_KEEPALIVE_TIMER 5

class SwitchProDriver : public GPDriver {
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
    virtual const uint8_t * get_hid_descriptor_report_cb(uint8_t itf) ;
    virtual const uint8_t * get_descriptor_configuration_cb(uint8_t index);
    virtual const uint8_t * get_descriptor_device_qualifier_cb();
    virtual uint16_t GetJoystickMidValue();
    virtual USBListener * get_usb_auth_listener() { return nullptr; }
private:
    uint8_t report[SWITCH_PRO_ENDPOINT_SIZE] = { };
    uint8_t last_report[SWITCH_PRO_ENDPOINT_SIZE] = { };
    SwitchProReport switchReport;
    uint8_t last_report_counter;
    uint32_t last_report_timer;
    bool isReady = false;
    bool isInitialized = false;
    bool isReportQueued = false;
    bool reportSent = false;
    uint8_t queuedReportID = 0;

    uint8_t handshakeCounter = 0;

    std::map<uint32_t, std::vector<uint8_t>> spiFlashData = {
        {0x6000, {
            // serial number
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,

            0xFF, 0xFF, 

            // device type
            0x03, 

            // unknown
            0xA0, 

            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

            // color options
            0x02,

            0xFF, 0xFF, 0xFF, 0xFF, 

            // config & calibration 1
            0xE3, 0xFF, 0x39, 0xFF, 0xED, 0x01, 0x00, 0x40,
            0x00, 0x40, 0x00, 0x40, 0x09, 0x00, 0xEA, 0xFF,
            0xA1, 0xFF, 0x3B, 0x34, 0x3B, 0x34, 0x3B, 0x34,

            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 

            // config & calibration 2
            // left stick
            0xD9, 0xB5, 0x68, 0xC8, 0x28, 0x79, 0x15, 0x06,
            0x5B, 

            // right stick
            0x48, 0xA8, 0x7D, 0x4B, 0x26, 0x63, 0xFB, 0x15,
            0x62, 

            0xFF,

            // body color
            0x32, 0x31, 0x32,

            // button color
            0xFF, 0xFF, 0xFF,

            // left grip color
            0xEC, 0x00, 0x8C,

            // right grip color
            0xEC, 0x00, 0x8C,

            0x01, 

            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF, 0x50, 0xFD, 0x00, 0x00, 0xC6,
            0x0F, 0x0F, 0x30, 0x61, 0xAE, 0x90, 0xD9, 0xD4,
            0x14, 0x54, 0x41, 0x15, 0x54, 0xC7, 0x79, 0x9C,
            0x33, 0x36, 0x63, 0x0F, 0x30, 0x61, 0xAE, 0x90,
            0xD9, 0xD4, 0x14, 0x54, 0x41, 0x15, 0x54, 0xC7,
            0x79, 0x9C, 0x33, 0x36, 0x63, 0xFF, 0xFF, 0xFF,
            0xFF, 0xFF, 0xFF
        }},
    };

    SwitchDeviceInfo deviceInfo;
    uint8_t playerID = 0;
    uint8_t inputMode = 0x30;
    bool isIMUEnabled = false;
    bool isVibrationEnabled = false;

    void sendIdentify();
    void sendSubCommand(uint8_t subCommand);

    bool sendReport(uint8_t reportID, const void* reportData, uint16_t reportLength);

    void readSPIFlash(uint8_t* dest, uint32_t address, uint8_t size);

    void handleConfigReport(uint8_t switchReportID, uint8_t switchReportSubID, const uint8_t *reportData, uint16_t reportLength);
    void handleFeatureReport(uint8_t switchReportID, uint8_t switchReportSubID, const uint8_t *reportData, uint16_t reportLength);
};

#endif // _SWITCH_PRO_DRIVER_H_