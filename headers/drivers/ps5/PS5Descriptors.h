/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

// USB identities presented to the console in PS5 mode. The driver presents
// the identity matching the bound auth dongle's signing format:
//  - third-party PS5 controller (Besavior P5General identity and layout)
//  - DualSense CFI-ZCT1 (what the Mayflash MagicBoots S5 signs for)
// With no dongle the third-party identity is used.

#ifndef _PS5_DESCRIPTORS_H_
#define _PS5_DESCRIPTORS_H_

#include <stdint.h>
#include "tusb.h"
#include "drivers/ps4/PS4Descriptors.h"   // TouchpadData, PSSensor

#define PS5_ENDPOINT_SIZE 64

#define PS5_VENDOR_ID     0x2B81    // third-party (P5General)
#define PS5_PRODUCT_ID    0x0101

#define DUALSENSE_VENDOR_ID   0x054C
#define DUALSENSE_PRODUCT_ID  0x0CE6

// HAT report (4 bits)
#define PS5_HAT_UP        0x00
#define PS5_HAT_UPRIGHT   0x01
#define PS5_HAT_RIGHT     0x02
#define PS5_HAT_DOWNRIGHT 0x03
#define PS5_HAT_DOWN      0x04
#define PS5_HAT_DOWNLEFT  0x05
#define PS5_HAT_LEFT      0x06
#define PS5_HAT_UPLEFT    0x07
#define PS5_HAT_NOTHING   0x0F

// Analog sticks report 8 bits
#define PS5_JOYSTICK_MIN 0x00
#define PS5_JOYSTICK_MID 0x80
#define PS5_JOYSTICK_MAX 0xFF

// Touchpad range (matches the existing PS4/P5General drivers)
#define PS5_TP_X_MIN 0
#define PS5_TP_X_MAX 1920
#define PS5_TP_Y_MIN 0
#define PS5_TP_Y_MAX 943

#define PS5_TP_MAX_COUNT 128

// Console-facing feature reports
typedef enum {
    PS5_FEATURE_DEFINITION       = 0x03,   // Controller definition (PS4ControllerConfig layout)
    PS5_FEATURE_CALIBRATION      = 0x05,   // Sensor calibration
    PS5_FEATURE_SET_BLUETOOTH    = 0x08,   // Bluetooth control
    PS5_FEATURE_PAIRING_INFO     = 0x09,   // Pairing info / MAC
    PS5_FEATURE_FIRMWARE         = 0x20,   // Firmware version
    PS5_FEATURE_SET_TEST         = 0x80,   // Test command (set)
    PS5_FEATURE_GET_TEST         = 0x81,   // Test command (get)
    PS5_FEATURE_SET_AUTH_PAYLOAD = 0xF0,   // Auth challenge, console -> controller
    PS5_FEATURE_GET_SIGNATURE    = 0xF1,   // Auth response, controller -> console
    PS5_FEATURE_GET_SIGNING_STATE= 0xF2,   // Auth status
} PS5FeatureReport;

// Input report 0x01 as built by PS5Driver, third-party layout (byte-compatible
// with P5GenerorReport). Bytes 0-11 are identical in the DualSense layout,
// which is all the MagicBoots S5 takes from it.
typedef struct __attribute__((packed)) {
    uint8_t report_id;
    uint8_t left_stick_x;
    uint8_t left_stick_y;
    uint8_t right_stick_x;
    uint8_t right_stick_y;

    uint8_t left_trigger;
    uint8_t right_trigger;

    uint8_t seq_number;

    uint8_t dpad : 4;
    uint8_t button_west : 1;
    uint8_t button_south : 1;
    uint8_t button_east : 1;
    uint8_t button_north : 1;

    uint8_t button_l1 : 1;
    uint8_t button_r1 : 1;
    uint8_t button_l2 : 1;
    uint8_t button_r2 : 1;
    uint8_t button_select : 1;
    uint8_t button_start : 1;
    uint8_t button_l3 : 1;
    uint8_t button_r3 : 1;

    uint8_t button_home : 1;
    uint8_t button_touchpad : 1;
    uint8_t : 6;

    uint8_t reserved_11;

    uint32_t auth_seq_number;

    PSSensor gyroscope;
    PSSensor accelerometer;

    uint16_t reserved_28_29;
    uint16_t data_30_31;                // 0x001A, as sent by the P5General

    TouchpadData touchpad_data;         // 0x20

    uint8_t reserved_40_55[16];

    uint8_t aes_cmac[8];                // 0x38, filled in by the auth dongle
} PS5Report;

static_assert(sizeof(PS5Report) == PS5_ENDPOINT_SIZE, "PS5Report must be 64 bytes");

static const uint8_t ps5tp_string_language[]     = { 0x09, 0x04 };
static const uint8_t ps5tp_string_manufacturer[] = "Activtor";
static const uint8_t ps5tp_string_product[]      = "P5General";
static const uint8_t ps5tp_string_version[]      = "0.1";

static const uint8_t * ps5tp_string_descriptors[] __attribute__((unused)) =
{
    ps5tp_string_language,
    ps5tp_string_manufacturer,
    ps5tp_string_product,
    ps5tp_string_version
};

static const uint8_t ps5tp_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass
    0x00,        // bDeviceProtocol
    0x40,        // bMaxPacketSize0 64
    TU_U16_LOW(PS5_VENDOR_ID),  TU_U16_HIGH(PS5_VENDOR_ID),   // idVendor 0x2B81
    TU_U16_LOW(PS5_PRODUCT_ID), TU_U16_HIGH(PS5_PRODUCT_ID),  // idProduct 0x0101
    0x01, 0x00,  // bcdDevice 0.01
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x00,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};

// Third-party PS5 report descriptor (from the P5General driver). Declares
// input 0x01, output 0x02, feature 0x03 (definition), 0xE0, and F0/F1/F2.
static const uint8_t ps5tp_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x09, 0x33,        //   Usage (Rx)
    0x09, 0x34,        //   Usage (Ry)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x20,        //   Usage (0x20)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x07,        //   Logical Maximum (7)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0E,        //   Usage Maximum (0x0E)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0E,        //   Report Count (14)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x21,        //   Usage (0x21)
    0x95, 0x0E,        //   Report Count (14)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x09, 0x22,        //   Usage (0x22)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x34,        //   Report Count (52)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x85, 0x02,        //   Report ID (2)
    0x09, 0x23,        //   Usage (0x23)
    0x95, 0x2F,        //   Report Count (47)
    0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0x03,        //   Report ID (3)
    0x0A, 0x21, 0x28,  //   Usage (0x2821)
    0x95, 0x2F,        //   Report Count (47)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x06, 0x80, 0xFF,  //   Usage Page (Vendor Defined 0xFF80)
    0x85, 0xE0,        //   Report ID (-32)
    0x09, 0x57,        //   Usage (0x57)
    0x95, 0x02,        //   Report Count (2)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
    0x06, 0xF0, 0xFF,  // Usage Page (Vendor Defined 0xFFF0)
    0x09, 0x40,        // Usage (0x40)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0xF0,        //   Report ID (-16)
    0x09, 0x47,        //   Usage (0x47)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF1,        //   Report ID (-15)
    0x09, 0x48,        //   Usage (0x48)
    0x95, 0x3F,        //   Report Count (63)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x85, 0xF2,        //   Report ID (-14)
    0x09, 0x49,        //   Usage (0x49)
    0x95, 0x0F,        //   Report Count (15)
    0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              // End Collection
};

static_assert(sizeof(ps5tp_report_descriptor) == 165, "third-party report descriptor is 165 bytes");

static const uint8_t ps5tp_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x29, 0x00,  // wTotalLength 41
    0x01,        // bNumInterfaces 1
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0x80,        // bmAttributes (Bus Powered)
    0xFA,        // bMaxPower 500mA

    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x00,        // bInterfaceNumber 0
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints 2
    0x03,        // bInterfaceClass (HID)
    0x00,        // bInterfaceSubClass
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.11
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID Report)
    TU_U16_LOW(sizeof(ps5tp_report_descriptor)), TU_U16_HIGH(sizeof(ps5tp_report_descriptor)),

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x82,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x01,        // bEndpointAddress (OUT/H2D)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x06,        // bInterval 6
};

static_assert(sizeof(ps5tp_configuration_descriptor) == 41, "wTotalLength mismatch");

// ---- DualSense (CFI-ZCT1) identity ----

static const uint8_t ds_string_language[]     = { 0x09, 0x04 };
static const uint8_t ds_string_manufacturer[] = "Sony Interactive Entertainment";
static const uint8_t ds_string_product[]      = "DualSense Wireless Controller";
static const uint8_t ds_string_version[]      = "1.0";

static const uint8_t * ds_string_descriptors[] __attribute__((unused)) =
{
    ds_string_language,
    ds_string_manufacturer,
    ds_string_product,
    ds_string_version
};

static const uint8_t ds_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass
    0x00,        // bDeviceProtocol
    0x40,        // bMaxPacketSize0 64
    TU_U16_LOW(DUALSENSE_VENDOR_ID),  TU_U16_HIGH(DUALSENSE_VENDOR_ID),   // idVendor 0x054C
    TU_U16_LOW(DUALSENSE_PRODUCT_ID), TU_U16_HIGH(DUALSENSE_PRODUCT_ID),  // idProduct 0x0CE6
    0x00, 0x01,  // bcdDevice 1.00
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x00,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};

// Byte-identical to the CFI-ZCT1 report descriptor dump (273 bytes), including
// the 0x80/0x81 test-command reports.
static const uint8_t ds_report_descriptor[] =
{
    0x05, 0x01, 0x09, 0x05, 0xA1, 0x01, 0x85, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x32, 0x09, 0x35,
    0x09, 0x33, 0x09, 0x34, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x95, 0x06, 0x81, 0x02, 0x06,
    0x00, 0xFF, 0x09, 0x20, 0x95, 0x01, 0x81, 0x02, 0x05, 0x01, 0x09, 0x39, 0x15, 0x00, 0x25, 0x07,
    0x35, 0x00, 0x46, 0x3B, 0x01, 0x65, 0x14, 0x75, 0x04, 0x95, 0x01, 0x81, 0x42, 0x65, 0x00, 0x05,
    0x09, 0x19, 0x01, 0x29, 0x0F, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x0F, 0x81, 0x02, 0x06,
    0x00, 0xFF, 0x09, 0x21, 0x95, 0x0D, 0x81, 0x02, 0x06, 0x00, 0xFF, 0x09, 0x22, 0x15, 0x00, 0x26,
    0xFF, 0x00, 0x75, 0x08, 0x95, 0x34, 0x81, 0x02, 0x85, 0x02, 0x09, 0x23, 0x95, 0x2F, 0x91, 0x02,
    0x85, 0x05, 0x09, 0x33, 0x95, 0x28, 0xB1, 0x02, 0x85, 0x08, 0x09, 0x34, 0x95, 0x2F, 0xB1, 0x02,
    0x85, 0x09, 0x09, 0x24, 0x95, 0x13, 0xB1, 0x02, 0x85, 0x0A, 0x09, 0x25, 0x95, 0x1A, 0xB1, 0x02,
    0x85, 0x20, 0x09, 0x26, 0x95, 0x3F, 0xB1, 0x02, 0x85, 0x21, 0x09, 0x27, 0x95, 0x04, 0xB1, 0x02,
    0x85, 0x22, 0x09, 0x40, 0x95, 0x3F, 0xB1, 0x02, 0x85, 0x80, 0x09, 0x28, 0x95, 0x3F, 0xB1, 0x02,
    0x85, 0x81, 0x09, 0x29, 0x95, 0x3F, 0xB1, 0x02, 0x85, 0x82, 0x09, 0x2A, 0x95, 0x09, 0xB1, 0x02,
    0x85, 0x83, 0x09, 0x2B, 0x95, 0x3F, 0xB1, 0x02, 0x85, 0x84, 0x09, 0x2C, 0x95, 0x3F, 0xB1, 0x02,
    0x85, 0x85, 0x09, 0x2D, 0x95, 0x02, 0xB1, 0x02, 0x85, 0xA0, 0x09, 0x2E, 0x95, 0x01, 0xB1, 0x02,
    0x85, 0xE0, 0x09, 0x2F, 0x95, 0x3F, 0xB1, 0x02, 0x85, 0xF0, 0x09, 0x30, 0x95, 0x3F, 0xB1, 0x02,
    0x85, 0xF1, 0x09, 0x31, 0x95, 0x3F, 0xB1, 0x02, 0x85, 0xF2, 0x09, 0x32, 0x95, 0x0F, 0xB1, 0x02,
    0x85, 0xF4, 0x09, 0x35, 0x95, 0x3F, 0xB1, 0x02, 0x85, 0xF5, 0x09, 0x36, 0x95, 0x03, 0xB1, 0x02,
    0xC0,
};

static_assert(sizeof(ds_report_descriptor) == 273, "DualSense report descriptor is 273 bytes");

// Full DualSense composite configuration (227 bytes): audio control (itf 0),
// speaker (itf 1) and microphone (itf 2) streaming interfaces, and HID on
// interface 3 with endpoints 0x84 IN / 0x05 OUT. The audio interfaces are
// claimed and answered but never stream. From PR #1727 (genuine pad layout).
#define DS_HID_INTERFACE 3

static const uint8_t ds_configuration_descriptor[] =
{
    0x09, 0x02, 0xE3, 0x00, 0x04, 0x01, 0x00, 0xC0, 0xFA,
    0x09, 0x04, 0x00, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00,
    0x0A, 0x24, 0x01, 0x00, 0x01, 0x49, 0x00, 0x02, 0x01, 0x02,
    0x0C, 0x24, 0x02, 0x01, 0x01, 0x01, 0x06, 0x04, 0x33, 0x00, 0x00, 0x00,
    0x0C, 0x24, 0x06, 0x02, 0x01, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x09, 0x24, 0x03, 0x03, 0x01, 0x03, 0x04, 0x02, 0x00,
    0x0C, 0x24, 0x02, 0x04, 0x02, 0x04, 0x03, 0x02, 0x03, 0x00, 0x00, 0x00,
    0x09, 0x24, 0x06, 0x05, 0x04, 0x01, 0x03, 0x00, 0x00,
    0x09, 0x24, 0x03, 0x06, 0x01, 0x01, 0x01, 0x05, 0x00,
    0x09, 0x04, 0x01, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00,
    0x09, 0x04, 0x01, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00,
    0x07, 0x24, 0x01, 0x01, 0x01, 0x01, 0x00,
    0x0B, 0x24, 0x02, 0x01, 0x04, 0x02, 0x10, 0x01, 0x80, 0xBB, 0x00,
    0x09, 0x05, 0x01, 0x09, 0xC4, 0x00, 0x04, 0x00, 0x00,
    0x07, 0x25, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x09, 0x04, 0x02, 0x00, 0x00, 0x01, 0x02, 0x00, 0x00,
    0x09, 0x04, 0x02, 0x01, 0x01, 0x01, 0x02, 0x00, 0x00,
    0x07, 0x24, 0x01, 0x06, 0x01, 0x01, 0x00,
    0x0B, 0x24, 0x02, 0x01, 0x01, 0x02, 0x10, 0x01, 0x80, 0xBB, 0x00,
    0x09, 0x05, 0x82, 0x05, 0x62, 0x00, 0x04, 0x00, 0x00,
    0x07, 0x25, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x09, 0x04, 0x03, 0x00, 0x02, 0x03, 0x00, 0x00, 0x00,
    0x09, 0x21, 0x11, 0x01, 0x00, 0x01, 0x22, 0x11, 0x01,
    0x07, 0x05, 0x84, 0x03, 0x40, 0x00, 0x06,
    0x07, 0x05, 0x05, 0x03, 0x40, 0x00, 0x06
};

static_assert(sizeof(ds_configuration_descriptor) == 227, "DualSense composite configuration is 227 bytes");
static_assert(sizeof(ds_report_descriptor) == 0x0111, "HID descriptor in the composite configuration declares 273 bytes");

// ---- Identity bundles ----

struct PS5Identity {
    const char * name;
    bool composite;             // audio interfaces present (claimed, never streamed)
    uint8_t hidInterface;       // interface number of the HID interface
    bool stallSetIdle;          // a real DualSense stalls HID SET_IDLE
    const uint8_t * deviceDescriptor;
    const uint8_t * configurationDescriptor;
    const uint8_t * reportDescriptor;
    const uint8_t * const * strings;
    uint8_t stringCount;
};

static const PS5Identity ps5ThirdPartyIdentity = {
    "third-party (2B81:0101)", false, 0, false,
    ps5tp_device_descriptor, ps5tp_configuration_descriptor, ps5tp_report_descriptor,
    ps5tp_string_descriptors, sizeof(ps5tp_string_descriptors) / sizeof(ps5tp_string_descriptors[0])
};

static const PS5Identity ps5DualSenseIdentity = {
    "DualSense (054C:0CE6)", true, DS_HID_INTERFACE, true,
    ds_device_descriptor, ds_configuration_descriptor, ds_report_descriptor,
    ds_string_descriptors, sizeof(ds_string_descriptors) / sizeof(ds_string_descriptors[0])
};

#endif // _PS5_DESCRIPTORS_H_
