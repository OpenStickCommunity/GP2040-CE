/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>

#define PS3_ENDPOINT_SIZE 64

// Mac OS-X and Linux automatically load the correct drivers.  On
// Windows, even though the driver is supplied by Microsoft, an
// INF file is needed to load the driver.  These numbers need to
// match the INF file.
//#define PS3_VENDOR_ID     0x10C4
//#define PS3_PRODUCT_ID    0x82C0

// DS3 Sixaxis
#define PS3_VENDOR_ID       0x054C
#define PS3_PRODUCT_ID      0x0268

/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

#define ENDPOINT0_SIZE	64
#define PS3_FEATURES_SIZE 48

#define GAMEPAD_INTERFACE	0
#define GAMEPAD_ENDPOINT	1
#define GAMEPAD_SIZE		64

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

// HAT report (4 bits)
#define PS3_HAT_UP        0x00
#define PS3_HAT_UPRIGHT   0x01
#define PS3_HAT_RIGHT     0x02
#define PS3_HAT_DOWNRIGHT 0x03
#define PS3_HAT_DOWN      0x04
#define PS3_HAT_DOWNLEFT  0x05
#define PS3_HAT_LEFT      0x06
#define PS3_HAT_UPLEFT    0x07
#define PS3_HAT_NOTHING   0x08

// Button report (16 bits)
#define PS3_MASK_SQUARE   (1U <<  0)
#define PS3_MASK_CROSS    (1U <<  1)
#define PS3_MASK_CIRCLE   (1U <<  2)
#define PS3_MASK_TRIANGLE (1U <<  3)
#define PS3_MASK_L1       (1U <<  4)
#define PS3_MASK_R1       (1U <<  5)
#define PS3_MASK_L2       (1U <<  6)
#define PS3_MASK_R2       (1U <<  7)
#define PS3_MASK_SELECT   (1U <<  8)
#define PS3_MASK_START    (1U <<  9)
#define PS3_MASK_L3       (1U << 10)
#define PS3_MASK_R3       (1U << 11)
#define PS3_MASK_PS       (1U << 12)
#define PS3_MASK_TP       (1U << 13)

// HID analog sticks only report 8 bits
#define PS3_JOYSTICK_MIN 0x00
#define PS3_JOYSTICK_MID 0x7F
#define PS3_JOYSTICK_MAX 0xFF

#define PS3_CENTER_SIXAXIS 0xFF01

typedef enum {
    PS3_FEATURE_01 = 0x01,
    PS3_FEATURE_EF = 0xEF,
    PS3_GET_PAIRING_INFO = 0xF2,
    PS3_FEATURE_F4 = 0xF4,
    PS3_FEATURE_F5 = 0xF5,
    PS3_FEATURE_F7 = 0xF7,
    PS3_FEATURE_F8 = 0xF8,
} PS3ReportTypes;

typedef enum {
    PS3_PLUGGED = 0x02,
    PS3_UNPLUGGED = 0x03
} PS3PluggedInState;

typedef enum {
    PS3_POWER_CHARGING = 0xEE,
    PS3_POWER_NOT_CHARGING = 0xF1,
    PS3_POWER_SHUTDOWN = 0x01,
    PS3_POWER_DISCHARGING = 0x02,
    PS3_POWER_LOW = 0x03,
    PS3_POWER_HIGH = 0x04,
    PS3_POWER_FULL = 0x05,
} PS3PowerState;

typedef enum {
    PS3_WIRED_RUMBLE = 0x10,
    PS3_WIRED_NO_RUMBLE = 0x12,
    PS3_WIRELESS_RUMBLE = 0x14,
    PS3_WIRELESS_NO_RUMBLE = 0x16,
} PS3WiredState;

typedef struct __attribute((packed, aligned(1)))
{
	// digital buttons, 0 = off, 1 = on
    // 0
    uint8_t reportID;
    // 1
    uint8_t reserved;

    // 2
    uint8_t select_btn : 1;
    uint8_t l3_btn : 1;
    uint8_t r3_btn : 1;
    uint8_t start_btn : 1;

    uint8_t dpad_up : 1;
    uint8_t dpad_right : 1;
    uint8_t dpad_down : 1;
    uint8_t dpad_left : 1;

    // 3
    uint8_t l2_btn : 1;
    uint8_t r2_btn : 1;
    uint8_t l1_btn : 1;
    uint8_t r1_btn : 1;

    uint8_t triangle_btn : 1;
    uint8_t circle_btn : 1;
    uint8_t cross_btn : 1;
    uint8_t square_btn : 1;

    // 4
    uint8_t ps_btn : 1;
    uint8_t tp_btn : 1;
    uint8_t : 6;

    // 5
    uint8_t : 8;
    
    // left and right analog sticks, 0x00 left/up, 0x80 middle, 0xff right/down
    // 6
    uint8_t l_x_axis;
    // 7
    uint8_t l_y_axis;
    // 8
    uint8_t r_x_axis;
    // 9
    uint8_t r_y_axis;
    // 10
    uint8_t : 8;

    // 11
    uint8_t : 8;

    // 12
    uint8_t movePowerStatus : 8;

    // 13
    uint8_t : 8;

    // button analog values for the d-pad.
    // 14
    uint8_t up_axis;
    // 15
    uint8_t right_axis;
    // 16
    uint8_t down_axis;
    // 17
    uint8_t left_axis;

    // button axis, 0x00 = unpressed, 0xff = fully pressed
    // 18
    uint8_t l2_axis;
    // 19
    uint8_t r2_axis;
    // 20
    uint8_t l1_axis;
    // 21
    uint8_t r1_axis;

    // 22
    uint8_t triangle_axis;
    // 23
    uint8_t circle_axis;
    // 24
    uint8_t cross_axis;
    // 25
    uint8_t square_axis;

    // 26
    uint8_t reserved2[3];

    // 29
    uint8_t plugged;
    // 30
    uint8_t powerStatus;

    // 31
    uint8_t rumbleStatus;

    // 32
    uint8_t reserved3[9];

    // 40
    uint16_t accelerometer_x;
    // 42
    uint16_t accelerometer_y;
    // 44
    uint16_t accelerometer_z;
    // 46
    uint16_t gyroscope_z;

    // 48
    uint16_t reserved4;
} PS3Report; // 49 length

typedef struct __attribute((packed, aligned(1)))
{
    // 0
    uint8_t reserved;

    // 1
    uint8_t rightMotorDuration;
    // 2
    uint8_t rightMotorPower;

    // 3
    uint8_t leftMotorDuration;
    // 4
    uint8_t leftMotorPower;

    // 5
    uint8_t reserved2[4];

    // 9
    uint8_t playerLED : 4;
    uint8_t : 4;

    // 10
    uint8_t reserved3[38];
} PS3Features; // 48 length

typedef struct __attribute((packed, aligned(1)))
{
    uint8_t reserved[2];
    uint8_t deviceAddress[7]; // leading zero followed by address
    uint8_t hostAddress[7]; // leading zero followed by address
    uint8_t reserved1;
} PS3BTInfo;

static const uint8_t ps3_string_language[]     = { 0x09, 0x04 };
static const uint8_t ps3_string_manufacturer[] = "Open Stick Community";
static const uint8_t ps3_string_product[]      = "GP2040-CE (PS3)";
static const uint8_t ps3_string_version[]      = "1.0";

static const uint8_t *ps3_string_descriptors[] __attribute__((unused)) =
{
	ps3_string_language,
	ps3_string_manufacturer,
	ps3_string_product,
	ps3_string_version
};

static const uint8_t ps3_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass 
    0x00,        // bDeviceProtocol 
    0x40,        // bMaxPacketSize0 64
    LSB(PS3_VENDOR_ID), MSB(PS3_VENDOR_ID),	  // idVendor
    LSB(PS3_PRODUCT_ID), MSB(PS3_PRODUCT_ID), // idProduct
    0x00, 0x01,  // bcdDevice 1.00
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x00,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};

static const uint8_t ps3_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Physical)
    0xA1, 0x02,        //   Collection (Application)
    0x85, 0x01,        //     Report ID (1)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x01,        //     Report Count (1)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
                       //     NOTE: reserved byte
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x13,        //     Report Count (19)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x35, 0x00,        //     Physical Minimum (0)
    0x45, 0x01,        //     Physical Maximum (1)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x13,        //     Usage Maximum (0x13)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x0D,        //     Report Count (13)
    0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
    0x81, 0x03,        //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
                       //     NOTE: 32 bit integer, where 0:18 are buttons and 19:31 are reserved
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x01,        //     Usage (Pointer)
    0xA1, 0x00,        //     Collection (Undefined)
    0x75, 0x08,        //       Report Size (8)
    0x95, 0x04,        //       Report Count (4)
    0x35, 0x00,        //       Physical Minimum (0)
    0x46, 0xFF, 0x00,  //       Physical Maximum (255)
    0x09, 0x30,        //       Usage (X)
    0x09, 0x31,        //       Usage (Y)
    0x09, 0x32,        //       Usage (Z)
    0x09, 0x35,        //       Usage (Rz)
    0x81, 0x02,        //       Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
                       //       NOTE: four joysticks
    0xC0,              //     End Collection
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x27,        //     Report Count (39)
    0x09, 0x01,        //     Usage (Pointer)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xA1, 0x02,        //   Collection (Application)
    0x85, 0x02,        //     Report ID (2)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xA1, 0x02,        //   Collection (Application)
    0x85, 0xEE,        //     Report ID (238)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xA1, 0x02,        //   Collection (Application)
    0x85, 0xEF,        //     Report ID (239)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x30,        //     Report Count (48)
    0x09, 0x01,        //     Usage (Pointer)
    0xB1, 0x02,        //     Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};

static const uint8_t ps3_hid_descriptor[] =
{
    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.17
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x94, 0x00,  // wDescriptorLength[0] 148
};

static const uint8_t ps3_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x29, 0x00,  // wTotalLength 41
    0x01,        // bNumInterfaces 1
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0x80,        // bmAttributes
    0xFA,        // bMaxPower 500mA

    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x00,        // bInterfaceNumber 0
    0x00,        // bAlternateSetting
    0x02,        // bNumEndpoints 2
    0x03,        // bInterfaceClass
    0x00,        // bInterfaceSubClass
    0x00,        // bInterfaceProtocol
    0x00,        // iInterface (String Index)

    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.17
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x94, 0x00,  // wDescriptorLength[0] 148

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x02,        // bEndpointAddress (OUT/H2D)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x01,        // bInterval 1 (unit depends on device speed)
};
