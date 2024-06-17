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

// HID analog sticks only report 8 bits
#define HID_JOYSTICK_MIN 0x00
#define HID_JOYSTICK_MID 0x80
#define HID_JOYSTICK_MAX 0xFF

typedef struct __attribute((packed, aligned(1)))
{
	// digital buttons, 0 = off, 1 = on
	uint32_t buttons;

	// digital direction, use the dir_* constants(enum) as a hat
	// 8 = center, 0 = up, 1 = up/right, 2 = right, 3 = right/down
	// 4 = down, 5 = down/left, 6 = left, 7 = left/up
	uint8_t direction : 4;
	// four bits would fill this out...
	uint8_t dummy : 4;

	// analogs --- left stick x/y, right stick x/y
	uint8_t l_x_axis;
	uint8_t l_y_axis;
	uint8_t r_x_axis;
	uint8_t r_y_axis;
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
	// 32 buttons
	0x05, 0x09,        //   USAGE_PAGE (Button)
	0x19, 0x01,        //   USAGE_MINIMUM (Button 1)
	0x29, 0x20,        //   USAGE_MAXIMUM (Button 32)
	0x15, 0x00,        //   LOGICAL_MINIMUM (0)
	0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
	0x95, 0x20,        //   REPORT_COUNT (32)
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
	// analogs
	0x05, 0x01,        //   USAGE_PAGE (Generic Desktop)
	0x26, 0xff, 0x00,  //   LOGICAL_MAXIMUM (255)
	0x46, 0xff, 0x00,  //   PHYSICAL_MAXIMUM (255)
	0x09, 0x30,        //   USAGE (X)
	0x09, 0x31,        //   USAGE (Y)
	0x09, 0x32,        //   USAGE (Z)
	0x09, 0x35,        //   USAGE (Rz)
	0x75, 0x08,        //   REPORT_SIZE (8)
	0x95, 0x04,        //   REPORT_COUNT (4)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	// done
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
