/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>

#define EGRET_ENDPOINT_SIZE 64

// HAT report (4 bits)
#define EGRET_HAT_UP        0x00
#define EGRET_HAT_UPRIGHT   0x01
#define EGRET_HAT_RIGHT     0x02
#define EGRET_HAT_DOWNRIGHT 0x03
#define EGRET_HAT_DOWN      0x04
#define EGRET_HAT_DOWNLEFT  0x05
#define EGRET_HAT_LEFT      0x06
#define EGRET_HAT_UPLEFT    0x07
#define EGRET_HAT_NOTHING   0x08

// Button report (16 bits)
#define EGRET_MASK_Y       (1U <<  8)
#define EGRET_MASK_B       (1U <<  4)
#define EGRET_MASK_A       (1U <<  2)
#define EGRET_MASK_X       (1U <<  3)
#define EGRET_MASK_R       (1U <<  0)
#define EGRET_MASK_ZR      (1U <<  1)
#define EGRET_MASK_MINUS   (1U <<  7)
#define EGRET_MASK_PLUS    (1U <<  6)
#define EGRET_MASK_HOME    (1U <<  9)

// Switch analog sticks only report 8 bits
#define EGRET_JOYSTICK_MIN 0x00
//#define EGRET_JOYSTICK_MID 0x80
#define EGRET_JOYSTICK_MID 0x7F
#define EGRET_JOYSTICK_MAX 0xFF

typedef struct __attribute((packed, aligned(1)))
{
	uint16_t buttons;
	uint8_t lx;
	uint8_t ly;
} EgretReport;

typedef struct
{
	uint16_t buttons;
	uint8_t lx;
	uint8_t ly;
} EgretOutReport;

static const uint8_t egret_string_language[]     = { 0x09, 0x04 };
static const uint8_t egret_string_manufacturer[] = "";
static const uint8_t egret_string_product[]      = "6B Controller";
static const uint8_t egret_string_version[]      = "1.0";

static const uint8_t *egret_string_descriptors[] __attribute__((unused)) =
{
	egret_string_language,
	egret_string_manufacturer,
	egret_string_product,
	egret_string_version
};

static const uint8_t egret_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass 
    0x00,        // bDeviceProtocol 
    0x40,        // bMaxPacketSize0 64
    0xE4, 0x0A,  // idVendor 0x0AE4
    0x02, 0x07,  // idProduct 0x0702
    0x00, 0x01,  // bcdDevice 2.00
    0x00,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x00,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};

static const uint8_t egret_hid_descriptor[] =
{
	0x09,        // bLength
	0x21,        // bDescriptorType (HID)
	0x11, 0x01,  // bcdHID 1.11
	0x00,        // bCountryCode
	0x01,        // bNumDescriptors
	0x22,        // bDescriptorType[0] (HID)
	0x56, 0x00   // wDescriptorLength[0] 86
};

static const uint8_t egret_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x29, 0x00,  // wTotalLength 41
    0x01,        // bNumInterfaces 1
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0x80,        // bmAttributes
    0x32,        // bMaxPower 100mA

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
    0x11, 0x01,  // bcdHID 1.11
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x65, 0x00,  // wDescriptorLength[0] 101

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

static const uint8_t egret_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x04,        // Usage (Joystick)
    0xA1, 0x01,        // Collection (Application)
    0xA1, 0x02,        //   Collection (Logical)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x05,        //     Report Count (5)
    0x15, 0x00,        //     Logical Minimum (0)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x35, 0x00,        //     Physical Minimum (0)
    0x46, 0xFF, 0x00,  //     Physical Maximum (255)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x04,        //     Report Size (4)
    0x95, 0x01,        //     Report Count (1)
    0x25, 0x07,        //     Logical Maximum (7)
    0x46, 0x3B, 0x01,  //     Physical Maximum (315)
    0x65, 0x14,        //     Unit (System: English Rotation, Length: Centimeter)
    0x09, 0x00,        //     Usage (Undefined)
    0x81, 0x42,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //     Unit (None)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x0A,        //     Report Count (10)
    0x25, 0x01,        //     Logical Maximum (1)
    0x45, 0x01,        //     Physical Maximum (1)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x0A,        //     Usage Maximum (0x0A)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //     Usage Page (Vendor Defined 0xFF00)
    0x75, 0x01,        //     Report Size (1)
    0x95, 0x0A,        //     Report Count (10)
    0x25, 0x01,        //     Logical Maximum (1)
    0x45, 0x01,        //     Physical Maximum (1)
    0x09, 0x01,        //     Usage (0x01)
    0x81, 0x02,        //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              //   End Collection
    0xA1, 0x02,        //   Collection (Logical)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x04,        //     Report Count (4)
    0x46, 0xFF, 0x00,  //     Physical Maximum (255)
    0x26, 0xFF, 0x00,  //     Logical Maximum (255)
    0x09, 0x02,        //     Usage (0x02)
    0x91, 0x02,        //     Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
    0xC0,              //   End Collection
    0xC0,              // End Collection
};