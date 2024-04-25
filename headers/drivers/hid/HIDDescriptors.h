/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>

#define HID_ENDPOINT_SIZE 64

// Mac OS-X and Linux automatically load the correct drivers.  On
// Windows, even though the driver is supplied by Microsoft, an
// INF file is needed to load the driver.  These numbers need to
// match the INF file.
#define VENDOR_ID		0x10C4
#define PRODUCT_ID		0x82C0

/**************************************************************************
 *
 *  Endpoint Buffer Configuration
 *
 **************************************************************************/

#define GAMEPAD_INTERFACE	0
#define GAMEPAD_ENDPOINT	1
#define GAMEPAD_SIZE		64

#define LSB(n) (n & 255)
#define MSB(n) ((n >> 8) & 255)

// HAT report (4 bits)
#define HID_HAT_UP        0x00
#define HID_HAT_UPRIGHT   0x01
#define HID_HAT_RIGHT     0x02
#define HID_HAT_DOWNRIGHT 0x03
#define HID_HAT_DOWN      0x04
#define HID_HAT_DOWNLEFT  0x05
#define HID_HAT_LEFT      0x06
#define HID_HAT_UPLEFT    0x07
#define HID_HAT_NOTHING   0x08

// Button report (16 bits)
#define HID_MASK_SQUARE   (1U <<  0)
#define HID_MASK_CROSS    (1U <<  1)
#define HID_MASK_CIRCLE   (1U <<  2)
#define HID_MASK_TRIANGLE (1U <<  3)
#define HID_MASK_L1       (1U <<  4)
#define HID_MASK_R1       (1U <<  5)
#define HID_MASK_L2       (1U <<  6)
#define HID_MASK_R2       (1U <<  7)
#define HID_MASK_SELECT   (1U <<  8)
#define HID_MASK_START    (1U <<  9)
#define HID_MASK_L3       (1U << 10)
#define HID_MASK_R3       (1U << 11)
#define HID_MASK_PS       (1U << 12)
#define HID_MASK_TP       (1U << 13)

// HID analog sticks only report 8 bits
#define HID_JOYSTICK_MIN 0x00
#define HID_JOYSTICK_MID 0x80
#define HID_JOYSTICK_MAX 0xFF

typedef struct __attribute((packed, aligned(1)))
{
	// digital buttons, 0 = off, 1 = on
	uint8_t button_01 : 1;
	uint8_t button_02 : 1;
	uint8_t button_03 : 1;
	uint8_t button_04 : 1;
	uint8_t button_05 : 1;
	uint8_t button_06 : 1;
	uint8_t button_07 : 1;
	uint8_t button_08 : 1;
	uint8_t button_09 : 1;
	uint8_t button_10 : 1;
	uint8_t button_11 : 1;
	uint8_t button_12 : 1;
	uint8_t button_13 : 1;
	uint8_t button_14 : 1;
	uint8_t button_15 : 1;
	uint8_t button_16 : 1;

	// digital direction, use the dir_* constants(enum) as a hat
	// 8 = center, 0 = up, 1 = up/right, 2 = right, 3 = right/down
	// 4 = down, 5 = down/left, 6 = left, 7 = left/up
	uint8_t direction : 4;
	// TODO: provide the raw directions in case something wants them?
	// four bits would fill this out...
	uint8_t dummy : 4;
} HIDReport;

static const uint8_t hid_string_language[]     = { 0x09, 0x04 };
static const uint8_t hid_string_manufacturer[] = "Open Stick Community";
static const uint8_t hid_string_product[]      = "GP2040-CE (Generic)";
static const uint8_t hid_string_version[]      = "1.0";

static const uint8_t *hid_string_descriptors[] __attribute__((unused)) =
{
	hid_string_language,
	hid_string_manufacturer,
	hid_string_product,
	hid_string_version
};

static const uint8_t hid_device_descriptor[] =
{
	18,								  // bLength
	1,								  // bDescriptorType
	0x00, 0x02,							  // bcdUSB
	0,								  // bDeviceClass
	0,								  // bDeviceSubClass
	0,								  // bDeviceProtocol
	HID_ENDPOINT_SIZE,						  // bMaxPacketSize0
	LSB(VENDOR_ID), MSB(VENDOR_ID),					  // idVendor
	LSB(PRODUCT_ID), MSB(PRODUCT_ID),				  // idProduct
	0x00, 0x01,							  // bcdDevice
	1,								  // iManufacturer
	2,								  // iProduct
	0,								  // iSerialNumber
	1								  // bNumConfigurations
};

static const uint8_t hid_report_descriptor[] =
{
	0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
	0x09, 0x05,        // USAGE (Gamepad)
	0xa1, 0x01,        // COLLECTION (Application)
	// 16 buttons
	0x05, 0x09,        //   USAGE_PAGE (Button)
	0x19, 0x01,        //   USAGE_MINIMUM (Button 1)
	0x29, 0x10,        //   USAGE_MAXIMUM (Button 16)
	0x15, 0x00,        //   LOGICAL_MINIMUM (0)
	0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
	0x95, 0x10,        //   REPORT_COUNT (16)
	0x75, 0x01,        //   REPORT_SIZE (1)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	// hat (dpad)
	0x05, 0x01,        //   USAGE_PAGE (Generic Desktop)
	0x09, 0x39,        //   USAGE (Hat switch)
	0x25, 0x07,        //   LOGICAL_MAXIMUM (7)
	0x95, 0x01,        //   REPORT_COUNT (1)
	0x75, 0x04,        //   REPORT_SIZE (4)
	0x81, 0x42,        //   INPUT (Data,Var,Abs,Null)
	// padding the hat
	0x95, 0x01,        //   REPORT_COUNT (1)
	0x75, 0x04,        //   REPORT_SIZE (4)
	0x81, 0x01,        //   INPUT (Cnst,Ary,Abs)
	0xc0               // END_COLLECTION
};

#define CONFIG1_DESC_SIZE		(9+9+9+7)
static const uint8_t hid_configuration_descriptor[] =
{
	// configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9,						       // bLength;
	2,						       // bDescriptorType;
	LSB(CONFIG1_DESC_SIZE),				       // wTotalLength
	MSB(CONFIG1_DESC_SIZE),
	1,						       // bNumInterfaces
	1,						       // bConfigurationValue
	0,						       // iConfiguration
	0x80,						       // bmAttributes
	50,						       // bMaxPower
	// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,						       // bLength
	4,						       // bDescriptorType
	GAMEPAD_INTERFACE,				       // bInterfaceNumber
	0,						       // bAlternateSetting
	1,						       // bNumEndpoints
	0x03,						       // bInterfaceClass (0x03 = HID)
	0x00,						       // bInterfaceSubClass (0x00 = No Boot)
	0x00,						       // bInterfaceProtocol (0x00 = No Protocol)
	0,						       // iInterface
	// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,						       // bLength
	0x21,						       // bDescriptorType
	0x11, 0x01,					       // bcdHID
	0,						       // bCountryCode
	1,						       // bNumDescriptors
	0x22,						       // bDescriptorType
	sizeof(hid_report_descriptor),			       // wDescriptorLength
	0,
	// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,						       // bLength
	5,						       // bDescriptorType
	GAMEPAD_ENDPOINT | 0x80,			       // bEndpointAddress
	0x03,						       // bmAttributes (0x03=intr)
	GAMEPAD_SIZE, 0,				       // wMaxPacketSize
	1						       // bInterval (1 ms)
};
