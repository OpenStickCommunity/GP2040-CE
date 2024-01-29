/*.
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2022 Ann(@alirin222)
 */

#pragma once

#include <stdint.h>

#define MDMINI_ENDPOINT_SIZE 64

// HAT1 report (8 bits)
#define MDMINI_MASK_LEFT    0x00
#define MDMINI_MASK_RIGHT   0xff

// HAT2 report (8 bits)
#define MDMINI_MASK_UP      0x00
#define MDMINI_MASK_DOWN    0xff

// Buttons (16 bits)
#define MDMINI_MASK_A       0x0040
#define MDMINI_MASK_B       0x0020
#define MDMINI_MASK_C       0x0200
#define MDMINI_MASK_X       0x0080
#define MDMINI_MASK_Y       0x0010
#define MDMINI_MASK_Z       0x0100
#define MDMINI_MASK_MODE    0x1000
#define MDMINI_MASK_START   0x2000

typedef struct __attribute((packed, aligned(1)))
{
	uint8_t id;
	uint8_t notuse1;
	uint8_t notuse2;
	uint8_t lx;
	uint8_t ly;
	uint16_t buttons;
	uint8_t notuse3;
} MDMiniReport;

static const uint8_t mdmini_string_language[]    = { 0x09, 0x04 };
static const uint8_t mdmini_string_manfacturer[] = "SEGA CORP.";
static const uint8_t mdmini_string_product[]     = "MEGA DRIVE mini GAMEPAD";
static const uint8_t mdmini_string_version[]     = "1.0";

static const uint8_t *mdmini_string_descriptors[] __attribute__((unused)) =
{
	mdmini_string_language,
	mdmini_string_manfacturer,
	mdmini_string_product,
	mdmini_string_version
};

static const uint8_t mdmini_device_descriptor[] =
{
	0x12,       // bLength
	0x01,       // bDescriptorType (Device)
	0x00, 0x02, // bcdUSB 2.00
	0x00,       // bDeviceClass
	0x00,       // bDeviceSubClass
	0x00,       // bDeviceProtocol
	0x40,       // bMaxPacketSize0 64
	0xA3, 0x0C, // idVendor 0x0CA3 "Sega Corp."
	0x24, 0x00, // idProduct 0x0024
	0x07, 0x02, // bcdDevice
	0x00,       // iManufacturer (String Index)
	0x02,       // iProduct (String Index)
	0x00,       // iSerialNumber (String Index)
	0x01,       // bNumConfigurations 1
};

static const uint8_t mdmini_hid_descriptor[] =
{
	0x09,        // bLength
	0x21,        // bDescriptorType (HID)
	0x11, 0x01,  // bcdHID 1.11
	0x00,        // bCountryCode
	0x01,        // bNumDescriptors
	0x22,        // bDescriptorType[0] (HID)
	0x65, 0x00,  // wDescriptorLength[0] 101
};

static const uint8_t mdmini_configuration_descriptor[] =
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
	0x0A,        // bInterval 10 (unit depends on device speed)

	0x07,        // bLength
	0x05,        // bDescriptorType (Endpoint)
	0x81,        // bEndpointAddress (IN/D2H)
	0x03,        // bmAttributes (Interrupt)
	0x40, 0x00,  // wMaxPacketSize 64
	0x0A,        // bInterval 10 (unit depends on device speed)
};

static const uint8_t mdmini_report_descriptor[] =
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