/*
 * SPDX-License-Identifier: MIT
 * Switch Pro Controller constants and HID descriptors for Bluetooth
 * Ported from retro-pico-switch by DavidPagels
 */

#ifndef SWITCH_BT_CONSTS_H
#define SWITCH_BT_CONSTS_H

#include <stdint.h>

// Switch subcommand IDs
#define BLUETOOTH_PAIR_REQUEST 0x01
#define REQUEST_DEVICE_INFO    0x02
#define SET_MODE               0x03
#define TRIGGER_BUTTONS        0x04
#define SET_SHIPMENT           0x08
#define SPI_READ               0x10
#define SET_NFC_IR_STATE       0x22
#define SET_PLAYER             0x30
#define TOGGLE_IMU             0x40
#define IMU_SENSITIVITY        0x41
#define ENABLE_VIBRATION       0x48
#define SET_NFC_IR_CONFIG      0x21

// Vibration report options
static const uint8_t VIB_OPTS[4] = {0x00, 0x04, 0x08, 0x0C};

// Button masks for Switch report (byte 0)
#define SWITCH_MASK_Y  (1U << 0)
#define SWITCH_MASK_X  (1U << 1)
#define SWITCH_MASK_B  (1U << 2)
#define SWITCH_MASK_A  (1U << 3)
#define SWITCH_MASK_R  (1U << 6)
#define SWITCH_MASK_ZR (1U << 7)

// Button masks for Switch report (byte 1)
#define SWITCH_MASK_MINUS   (1U << 0)
#define SWITCH_MASK_PLUS    (1U << 1)
#define SWITCH_MASK_R3      (1U << 2)
#define SWITCH_MASK_L3      (1U << 3)
#define SWITCH_MASK_HOME    (1U << 4)
#define SWITCH_MASK_CAPTURE (1U << 5)

// Button masks for Switch report (byte 2)
#define SWITCH_MASK_L  (1U << 6)
#define SWITCH_MASK_ZL (1U << 7)

// D-pad masks (in byte 2, lower nibble)
#define SWITCH_HAT_UP        0x02
#define SWITCH_HAT_DOWN      0x01
#define SWITCH_HAT_LEFT      0x08
#define SWITCH_HAT_RIGHT     0x04

// Bluetooth HID report descriptor for Pro Controller
static const uint8_t switch_bt_report_descriptor[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x06, 0x01, 0xFF,  //   Usage Page (Vendor Defined 0xFF01)

    0x85, 0x21,        //   Report ID (33)
    0x09, 0x21,        //   Usage (0x21)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x81, 0x02,        //   Input (Data,Var,Abs)

    0x85, 0x30,        //   Report ID (48)
    0x09, 0x30,        //   Usage (0x30)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x81, 0x02,        //   Input (Data,Var,Abs)

    0x85, 0x31,        //   Report ID (49)
    0x09, 0x31,        //   Usage (0x31)
    0x75, 0x08,        //   Report Size (8)
    0x96, 0x69, 0x01,  //   Report Count (361)
    0x81, 0x02,        //   Input (Data,Var,Abs)

    0x85, 0x32,        //   Report ID (50)
    0x09, 0x32,        //   Usage (0x32)
    0x75, 0x08,        //   Report Size (8)
    0x96, 0x69, 0x01,  //   Report Count (361)
    0x81, 0x02,        //   Input (Data,Var,Abs)

    0x85, 0x33,        //   Report ID (51)
    0x09, 0x33,        //   Usage (0x33)
    0x75, 0x08,        //   Report Size (8)
    0x96, 0x69, 0x01,  //   Report Count (361)
    0x81, 0x02,        //   Input (Data,Var,Abs)

    0x85, 0x3F,        //   Report ID (63)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x10,        //   Usage Maximum (0x10)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,Null)
    0x05, 0x09,        //   Usage Page (Button)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x33,        //   Usage (Rx)
    0x09, 0x34,        //   Usage (Ry)
    0x16, 0x00, 0x00,              //   Logical Minimum (0)
    0x27, 0xFF, 0xFF, 0x00, 0x00,  //   Logical Maximum (65534)
    0x75, 0x10,                    //   Report Size (16)
    0x95, 0x04,                    //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs)
    0x06, 0x01, 0xFF,  //   Usage Page (Vendor Defined 0xFF01)

    0x85, 0x01,        //   Report ID (1)
    0x09, 0x01,        //   Usage (0x01)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x91, 0x02,        //   Output (Data,Var,Abs)

    0x85, 0x10,        //   Report ID (16)
    0x09, 0x10,        //   Usage (0x10)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x09,        //   Report Count (9)
    0x91, 0x02,        //   Output (Data,Var,Abs)

    0x85, 0x11,        //   Report ID (17)
    0x09, 0x11,        //   Usage (0x11)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x91, 0x02,        //   Output (Data,Var,Abs)

    0x85, 0x12,        //   Report ID (18)
    0x09, 0x12,        //   Usage (0x12)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x30,        //   Report Count (48)
    0x91, 0x02,        //   Output (Data,Var,Abs)
    0xC0,              // End Collection
};

#endif // SWITCH_BT_CONSTS_H




