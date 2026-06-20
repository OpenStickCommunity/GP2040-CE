/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _DUALSENSEPS5HOST_H_
#define _DUALSENSEPS5HOST_H_

#include <gphost.h>

#include "drivers/ps4/PS4Descriptors.h"
#include "drivers/ps4/PS4Driver.h"

typedef struct __attribute__((packed)) {
    uint8_t reportID;
    uint8_t leftStickX;
    uint8_t leftStickY;
    uint8_t rightStickX;
    uint8_t rightStickY;
    uint8_t leftTrigger;
    uint8_t rightTrigger;

    // 8 bit report counter.
    uint8_t reportCounter;

    // 4 bits for the d-pad.
    uint8_t dpad : 4;

    // 14 bits for buttons.
    uint16_t buttonWest : 1;
    uint16_t buttonSouth : 1;
    uint16_t buttonEast : 1;
    uint16_t buttonNorth : 1;
    uint16_t buttonL1 : 1;
    uint16_t buttonR1 : 1;
    uint16_t buttonL2 : 1;
    uint16_t buttonR2 : 1;
    uint16_t buttonSelect : 1;
    uint16_t buttonStart : 1;
    uint16_t buttonL3 : 1;
    uint16_t buttonR3 : 1;
    uint16_t buttonHome : 1;
    uint16_t buttonTouchpad : 1;
    uint16_t buttonMicMute : 1;

    uint8_t miscData[54];
} DSReport;

//
// Dualsense PS5 Controller Host
//
class DualsenseHost : public GPHost {
public:
    virtual bool match(uint8_t vendor_id, uint8_t product_id);   
    virtual void initialize();
    virtual void gamepad(Gamepad * gamepad);
    virtual void process(uint8_t const* report, uint16_t len); // all these?
    virtual void shutdown();
private:
    bool requiresInit;
};

#endif // _DUALSENSEHOST_H_
