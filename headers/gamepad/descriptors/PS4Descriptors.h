/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>

#define PS4_ENDPOINT_SIZE 64

// Mayflash
//#define PS4_VENDOR_ID     0x33df
//#define PS4_PRODUCT_ID    0x0011

// Razer Panthera
#define PS4_VENDOR_ID		0x1532
#define PS4_PRODUCT_ID		0x0401

// Madcatz Fightstick Alpha PS4
//#define PS4_VENDOR_ID       0x0738
//#define PS4_PRODUCT_ID      0x8180

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
#define PS4_HAT_UP        0x00
#define PS4_HAT_UPRIGHT   0x01
#define PS4_HAT_RIGHT     0x02
#define PS4_HAT_DOWNRIGHT 0x03
#define PS4_HAT_DOWN      0x04
#define PS4_HAT_DOWNLEFT  0x05
#define PS4_HAT_LEFT      0x06
#define PS4_HAT_UPLEFT    0x07
#define PS4_HAT_NOTHING   0x0F

// Button report (16 bits)
#define PS4_MASK_SQUARE   (1U <<  0)
#define PS4_MASK_CROSS    (1U <<  1)
#define PS4_MASK_CIRCLE   (1U <<  2)
#define PS4_MASK_TRIANGLE (1U <<  3)
#define PS4_MASK_L1       (1U <<  4)
#define PS4_MASK_R1       (1U <<  5)
#define PS4_MASK_L2       (1U <<  6)
#define PS4_MASK_R2       (1U <<  7)
#define PS4_MASK_SELECT   (1U <<  8)
#define PS4_MASK_START    (1U <<  9)
#define PS4_MASK_L3       (1U << 10)
#define PS4_MASK_R3       (1U << 11)
#define PS4_MASK_PS       (1U << 12)
#define PS4_MASK_TP       (1U << 13)

// PS4 analog sticks only report 8 bits
#define PS4_JOYSTICK_MIN 0x00
#define PS4_JOYSTICK_MID 0x80
#define PS4_JOYSTICK_MAX 0xFF

struct TouchpadXY {
  uint8_t counter : 7;
  uint8_t unpressed : 1;

  // 12 bit X, followed by 12 bit Y
  uint8_t data[3];

  void set_x(uint16_t x) {
    data[0] = x & 0xff;
    data[1] = (data[1] & 0xf0) | ((x >> 8) & 0xf);
  }

  void set_y(uint16_t y) {
    data[1] = (data[1] & 0x0f) | ((y & 0xf) << 4);
    data[2] = y >> 4;
    return;
  }
};

struct TouchpadData {
  TouchpadXY p1;
  TouchpadXY p2;
};

typedef struct __attribute__((packed)) {
  uint8_t report_id;
  uint8_t left_stick_x;
  uint8_t left_stick_y;
  uint8_t right_stick_x;
  uint8_t right_stick_y;

  // 4 bits for the d-pad.
  uint32_t dpad : 4;

  // 14 bits for buttons.
  uint32_t button_west : 1;
  uint32_t button_south : 1;
  uint32_t button_east : 1;
  uint32_t button_north : 1;
  uint32_t button_l1 : 1;
  uint32_t button_r1 : 1;
  uint32_t button_l2 : 1;
  uint32_t button_r2 : 1;
  uint32_t button_select : 1;
  uint32_t button_start : 1;
  uint32_t button_l3 : 1;
  uint32_t button_r3 : 1;
  uint32_t button_home : 1;
  uint32_t button_touchpad : 1;

  // 6 bit report counter.
  uint32_t report_counter : 6;

  uint32_t left_trigger : 8;
  uint32_t right_trigger : 8;

  // 16 bit timing counter
  uint16_t axis_timing;

  uint32_t padding : 8;
  uint8_t mystery[22];
  TouchpadData touchpad_data;
  uint8_t mystery_2[21];
} PS4Report;

static const uint8_t ps4_string_language[]     = { 0x09, 0x04 };
static const uint8_t ps4_string_manufacturer[] = "Open Stick Community";
static const uint8_t ps4_string_product[]      = "GP2040-CE (PS4)";
static const uint8_t ps4_string_version[]      = "1.0";

static const uint8_t *ps4_string_descriptors[] __attribute__((unused)) =
{
	ps4_string_language,
	ps4_string_manufacturer,
	ps4_string_product,
	ps4_string_version
};

static const uint8_t ps4_device_descriptor[] =
{
	18,								  // bLength
	1,								  // bDescriptorType
	0x00, 0x02,						  // bcdUSB
	0,								  // bDeviceClass
	0,								  // bDeviceSubClass
	0,								  // bDeviceProtocol
	ENDPOINT0_SIZE,					  // bMaxPacketSize0
	LSB(PS4_VENDOR_ID), MSB(PS4_VENDOR_ID),	  // idVendor
	LSB(PS4_PRODUCT_ID), MSB(PS4_PRODUCT_ID), // idProduct
	0x00, 0x01,						  // bcdDevice
	1,								  // iManufacturer
	2,								  // iProduct
	0,								  // iSerialNumber
	1								  // bNumConfigurations
};

static const uint8_t ps4_report_descriptor[] =
{
	0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
	0x09, 0x05,        // Usage (Game Pad)
	0xA1, 0x01,        // Collection (Application)
	0x85, 0x01,        //   Report ID (1)
	0x09, 0x30,        //   Usage (X)
	0x09, 0x31,        //   Usage (Y)
	0x09, 0x32,        //   Usage (Z)
	0x09, 0x35,        //   Usage (Rz)
	0x15, 0x00,        //   Logical Minimum (0)
	0x26, 0xFF, 0x00,  //   Logical Maximum (255)
	0x75, 0x08,        //   Report Size (8)
	0x95, 0x04,        //   Report Count (4)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

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
	0x09, 0x20,        //   Usage (0x20)
	0x75, 0x06,        //   Report Size (6)
	0x95, 0x01,        //   Report Count (1)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

	0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
	0x09, 0x33,        //   Usage (Rx)
	0x09, 0x34,        //   Usage (Ry)
	0x15, 0x00,        //   Logical Minimum (0)
	0x26, 0xFF, 0x00,  //   Logical Maximum (255)
	0x75, 0x08,        //   Report Size (8)
	0x95, 0x02,        //   Report Count (2)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

	0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
	0x09, 0x21,        //   Usage (0x21)
	0x95, 0x36,        //   Report Count (54)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)

	0x85, 0x05,        //   Report ID (5)
	0x09, 0x22,        //   Usage (0x22)
	0x95, 0x1F,        //   Report Count (31)
	0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

	0x85, 0x03,        //   Report ID (3)
	0x0A, 0x21, 0x27,  //   Usage (0x2721)
	0x95, 0x2F,        //   Report Count (47)
	0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0,              // End Collection

	0x06, 0xF0, 0xFF,  // Usage Page (Vendor Defined 0xFFF0)
	0x09, 0x40,        // Usage (0x40)
	0xA1, 0x01,        // Collection (Application)
	0x85, 0xF0,        //   Report ID (-16) AUTH F0
	0x09, 0x47,        //   Usage (0x47)
	0x95, 0x3F,        //   Report Count (63)
	0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x85, 0xF1,        //   Report ID (-15) AUTH F1
	0x09, 0x48,        //   Usage (0x48)
	0x95, 0x3F,        //   Report Count (63)
	0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x85, 0xF2,        //   Report ID (-14) AUTH F2
	0x09, 0x49,        //   Usage (0x49)
	0x95, 0x0F,        //   Report Count (15)
	0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0x85, 0xF3,        //   Report ID (-13) Auth F3 (Reset)
	0x0A, 0x01, 0x47,  //   Usage (0x4701)
	0x95, 0x07,        //   Report Count (7)
	0xB1, 0x02,        //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0,              // End Collection
};

static const uint8_t ps4_hid_descriptor[] =
{
	0x09,								 // bLength
	0x21,								 // bDescriptorType (HID)
	0x11, 0x01,							 // bcdHID 1.11
	0x00,								 // bCountryCode
	0x01,								 // bNumDescriptors
	0x22,								 // bDescriptorType[0] (HID)
	sizeof(ps4_report_descriptor), 0x00, // wDescriptorLength[0] 90
};

#define CONFIG1_DESC_SIZE		(9+9+9+7)
static const uint8_t ps4_configuration_descriptor[] =
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
	sizeof(ps4_report_descriptor), // wDescriptorLength
	0,
		// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
	7,						 	   // bLength
	5,						       // bDescriptorType
	GAMEPAD_ENDPOINT | 0x80,       // bEndpointAddress
	0x03,					       // bmAttributes (0x03=intr)
	GAMEPAD_SIZE, 0,		       // wMaxPacketSize
	1						       // bInterval (1 ms)
};