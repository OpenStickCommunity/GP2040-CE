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

#define ENDPOINT0_SIZE	64

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

	uint8_t square_btn : 1;
	uint8_t cross_btn : 1;
	uint8_t circle_btn : 1;
	uint8_t triangle_btn : 1;

	uint8_t l1_btn : 1;
	uint8_t r1_btn : 1;
	uint8_t l2_btn : 1;
	uint8_t r2_btn : 1;

	uint8_t select_btn : 1;
	uint8_t start_btn : 1;
	uint8_t l3_btn : 1;
	uint8_t r3_btn : 1;
	
	uint8_t ps_btn : 1;
	uint8_t tp_btn : 1;
//	uint8_t l2_btn_alt : 1;
	
//	uint8_t r2_btn_alt : 1;
	uint8_t : 2;

	// digital direction, use the dir_* constants(enum)
	// 8 = center, 0 = up, 1 = up/right, 2 = right, 3 = right/down
	// 4 = down, 5 = down/left, 6 = left, 7 = left/up

	uint8_t direction;

	// left and right analog sticks, 0x00 left/up, 0x80 middle, 0xff right/down

	uint8_t l_x_axis;
	uint8_t l_y_axis;
	uint8_t r_x_axis;
	uint8_t r_y_axis;

	// Gonna assume these are button analog values for the d-pad.
	// NOTE: NOT EVEN SURE THIS IS RIGHT, OR IN THE CORRECT ORDER
	uint8_t right_axis;
	uint8_t left_axis;
	uint8_t up_axis;
	uint8_t down_axis;

	// button axis, 0x00 = unpressed, 0xff = fully pressed

	uint8_t triangle_axis;
	uint8_t circle_axis;
	uint8_t cross_axis;
	uint8_t square_axis;

	uint8_t l1_axis;
	uint8_t r1_axis;
	uint8_t l2_axis;
	uint8_t r2_axis;
} HIDReport;

static const uint8_t hid_string_language[]     = { 0x09, 0x04 };
static const uint8_t hid_string_manufacturer[] = "Open Stick Community";
static const uint8_t hid_string_product[]      = "GP2040-CE (D-Input)";
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
		0x00, 0x02,						  // bcdUSB
		0,								  // bDeviceClass
		0,								  // bDeviceSubClass
		0,								  // bDeviceProtocol
		ENDPOINT0_SIZE,					  // bMaxPacketSize0
		LSB(VENDOR_ID), MSB(VENDOR_ID),	  // idVendor
		LSB(PRODUCT_ID), MSB(PRODUCT_ID), // idProduct
		0x00, 0x01,						  // bcdDevice
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
	0x15, 0x00,        //   LOGICAL_MINIMUM (0)
	0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
	0x35, 0x00,        //   PHYSICAL_MINIMUM (0)
	0x45, 0x01,        //   PHYSICAL_MAXIMUM (1)
	0x75, 0x01,        //   REPORT_SIZE (1)
	0x95, 0x0e,        //   REPORT_COUNT (13)
	0x05, 0x09,        //   USAGE_PAGE (Button)
	0x19, 0x01,        //   USAGE_MINIMUM (Button 1)
	0x29, 0x0e,        //   USAGE_MAXIMUM (Button 13)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x95, 0x02,        //   REPORT_COUNT (3)
	0x81, 0x01,        //   INPUT (Cnst,Ary,Abs)
	0x05, 0x01,        //   USAGE_PAGE (Generic Desktop)
	0x25, 0x07,        //   LOGICAL_MAXIMUM (7)
	0x46, 0x3b, 0x01,  //   PHYSICAL_MAXIMUM (315)
	0x75, 0x04,        //   REPORT_SIZE (4)
	0x95, 0x01,        //   REPORT_COUNT (1)
	0x65, 0x14,        //   UNIT (Eng Rot:Angular Pos)
	0x09, 0x39,        //   USAGE (Hat switch)
	0x81, 0x42,        //   INPUT (Data,Var,Abs,Null)
	0x65, 0x00,        //   UNIT (None)
	0x95, 0x01,        //   REPORT_COUNT (1)
	0x81, 0x01,        //   INPUT (Cnst,Ary,Abs)
	0x26, 0xff, 0x00,  //   LOGICAL_MAXIMUM (255)
	0x46, 0xff, 0x00,  //   PHYSICAL_MAXIMUM (255)
	0x09, 0x30,        //   USAGE (X)
	0x09, 0x31,        //   USAGE (Y)
	0x09, 0x32,        //   USAGE (Z)
	0x09, 0x35,        //   USAGE (Rz)
	0x75, 0x08,        //   REPORT_SIZE (8)
	0x95, 0x04,        //   REPORT_COUNT (6)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x06, 0x00, 0xff,  //   USAGE_PAGE (Vendor Specific)
	0x09, 0x20,        //   Unknown
	0x09, 0x21,        //   Unknown
	0x09, 0x22,        //   Unknown
	0x09, 0x23,        //   Unknown
	0x09, 0x24,        //   Unknown
	0x09, 0x25,        //   Unknown
	0x09, 0x26,        //   Unknown
	0x09, 0x27,        //   Unknown
	0x09, 0x28,        //   Unknown
	0x09, 0x29,        //   Unknown
	0x09, 0x2a,        //   Unknown
	0x09, 0x2b,        //   Unknown
	0x95, 0x0c,        //   REPORT_COUNT (12)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x0a, 0x21, 0x26,  //   Unknown
	0x95, 0x08,        //   REPORT_COUNT (8)
	0xb1, 0x02,        //   FEATURE (Data,Var,Abs)
	0xc0               // END_COLLECTION
};

static const uint8_t hid_hid_descriptor[] =
{
		0x09,								 // bLength
		0x21,								 // bDescriptorType (HID)
		0x11, 0x01,							 // bcdHID 1.11
		0x00,								 // bCountryCode
		0x01,								 // bNumDescriptors
		0x22,								 // bDescriptorType[0] (HID)
		sizeof(hid_report_descriptor), 0x00, // wDescriptorLength[0] 90
};

#define CONFIG1_DESC_SIZE		(9+9+9+7)
static const uint8_t hid_configuration_descriptor[] =
{
	    // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
	9,						       // bLength;
	2,						       // bDescriptorType;
	LSB(CONFIG1_DESC_SIZE),        // wTotalLength
	MSB(CONFIG1_DESC_SIZE),
	1,	                           // bNumInterfaces
	1,	                           // bConfigurationValue
	0,	                           // iConfiguration
	0x80,                          // bmAttributes
	50,	                           // bMaxPower
		// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
	9,				               // bLength
	4,				               // bDescriptorType
	GAMEPAD_INTERFACE,             // bInterfaceNumber
	0,				               // bAlternateSetting
	1,				               // bNumEndpoints
	0x03,			               // bInterfaceClass (0x03 = HID)
	0x00,			               // bInterfaceSubClass (0x00 = No Boot)
	0x00,			               // bInterfaceProtocol (0x00 = No Protocol)
	0,				               // iInterface
		// HID interface descriptor, HID 1.11 spec, section 6.2.1
	9,							   // bLength
	0x21,						   // bDescriptorType
	0x11, 0x01,					   // bcdHID
	0,							   // bCountryCode
	1,							   // bNumDescriptors
	0x22,						   // bDescriptorType
	sizeof(hid_report_descriptor), // wDescriptorLength
	0,
		// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,						 	   // bLength
	5,						       // bDescriptorType
	GAMEPAD_ENDPOINT | 0x80,       // bEndpointAddress
	0x03,					       // bmAttributes (0x03=intr)
	GAMEPAD_SIZE, 0,		       // wMaxPacketSize
	1						       // bInterval (1 ms)
};
