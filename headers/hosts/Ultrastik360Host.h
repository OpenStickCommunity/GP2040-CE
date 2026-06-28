/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _ULTRASTIK360HOST_H_
#define _ULTRASTIK360HOST_H_

#include <gphost.h>

// Ultrastik 360
typedef struct TU_ATTR_PACKED
{
    uint8_t GD_GamePadPointerX;
    uint8_t GD_GamePadPointerY;

    struct {
        uint8_t BTN_GamePadButton1 : 1;
        uint8_t BTN_GamePadButton2 : 1;
        uint8_t BTN_GamePadButton3 : 1;
        uint8_t BTN_GamePadButton4 : 1;
        uint8_t BTN_GamePadButton5 : 1;
        uint8_t BTN_GamePadButton6 : 1;
        uint8_t BTN_GamePadButton7 : 1;
        uint8_t BTN_GamePadButton8 : 1;
        uint8_t BTN_GamePadButton9 : 1;
        uint8_t BTN_GamePadButton10 : 1;
        uint8_t BTN_GamePadButton11 : 1;
        uint8_t BTN_GamePadButton12 : 1;
        uint8_t BTN_GamePadButton13 : 1;
        uint8_t BTN_GamePadButton14 : 1;
        uint8_t BTN_GamePadButton15 : 1;
        uint8_t padding : 1;
    };

} ultrastik360_t;

//
// Ultrastik 360 Controller Host
//
class Ultrastik360Host : public GPHost {
public:
    static bool match(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len);   
    virtual void initialize(uint8_t dev_addr, uint8_t instance, uint16_t vendor_id, uint16_t product_id, uint8_t const* desc_report, uint16_t desc_len);
    virtual void gamepad(Gamepad * gamepad);
    virtual void process(uint8_t const* report, uint16_t len);
    virtual void update() {}
    virtual void shutdown() {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {}
private:
    uint32_t map(uint32_t x, uint32_t in_min, uint32_t in_max, uint32_t out_min, uint32_t out_max);
};

#endif
