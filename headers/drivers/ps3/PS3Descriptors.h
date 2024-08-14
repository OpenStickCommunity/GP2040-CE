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
//#define VENDOR_ID     0x10C4
//#define PRODUCT_ID    0x82C0

// DS3 Sixaxis
#define VENDOR_ID       0x054C
#define PRODUCT_ID      0x0268

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
#define PS3_JOYSTICK_MID 0x80
#define PS3_JOYSTICK_MAX 0xFF

typedef struct __attribute((packed, aligned(1)))
{
	// digital buttons, 0 = off, 1 = on
    // 0
    uint8_t reportID;
    // 1
    uint8_t reserved;

    // 2
	uint8_t cross_btn : 1;
	uint8_t circle_btn : 1;
	uint8_t square_btn : 1;
	uint8_t triangle_btn : 1;

	uint8_t l1_btn : 1;
	uint8_t r1_btn : 1;
	uint8_t l2_btn : 1;
	uint8_t r2_btn : 1;

    // 3
	uint8_t select_btn : 1;
    uint8_t start_btn : 1;
	uint8_t l3_btn : 1;
	uint8_t r3_btn : 1;
	
    uint8_t dpad_up : 1;
    uint8_t dpad_down : 1;
    uint8_t dpad_left : 1;
    uint8_t dpad_right : 1;

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
    uint8_t : 8;
    // 10
    uint8_t r_y_axis;

    // 11
    uint8_t : 8;
    uint8_t : 8;
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
    uint8_t reserved2[23];

//
//    // 21
//    uint8_t reserved2[27];
} PS3Report; // 49 length

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
    LSB(VENDOR_ID), MSB(VENDOR_ID),	  // idVendor
    LSB(PRODUCT_ID), MSB(PRODUCT_ID), // idProduct
    0x00, 0x01,  // bcdDevice 1.00
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x00,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
//		18,								  // bLength
//		1,								  // bDescriptorType
//		0x00, 0x02,						  // bcdUSB
//		0,								  // bDeviceClass
//		0,								  // bDeviceSubClass
//		0,								  // bDeviceProtocol
//		ENDPOINT0_SIZE,					  // bMaxPacketSize0
//		LSB(VENDOR_ID), MSB(VENDOR_ID),	  // idVendor
//		LSB(PRODUCT_ID), MSB(PRODUCT_ID), // idProduct
//		0x00, 0x01,						  // bcdDevice
//		1,								  // iManufacturer
//		2,								  // iProduct
//		0,								  // iSerialNumber
//		1								  // bNumConfigurations
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

//	0x05, 0x01,        // USAGE_PAGE (Generic Desktop)
//	0x09, 0x05,        // USAGE (Gamepad)
//	0xa1, 0x01,        // COLLECTION (Application)
//	0x15, 0x00,        //   LOGICAL_MINIMUM (0)
//	0x25, 0x01,        //   LOGICAL_MAXIMUM (1)
//	0x35, 0x00,        //   PHYSICAL_MINIMUM (0)
//	0x45, 0x01,        //   PHYSICAL_MAXIMUM (1)
//	0x75, 0x01,        //   REPORT_SIZE (1)
//	0x95, 0x0e,        //   REPORT_COUNT (13)
//	0x05, 0x09,        //   USAGE_PAGE (Button)
//	0x19, 0x01,        //   USAGE_MINIMUM (Button 1)
//	0x29, 0x0e,        //   USAGE_MAXIMUM (Button 13)
//	0x81, 0x02,        //   INPUT (Data,Var,Abs)
//	0x95, 0x02,        //   REPORT_COUNT (3)
//	0x81, 0x01,        //   INPUT (Cnst,Ary,Abs)
//	0x05, 0x01,        //   USAGE_PAGE (Generic Desktop)
//	0x25, 0x07,        //   LOGICAL_MAXIMUM (7)
//	0x46, 0x3b, 0x01,  //   PHYSICAL_MAXIMUM (315)
//	0x75, 0x04,        //   REPORT_SIZE (4)
//	0x95, 0x01,        //   REPORT_COUNT (1)
//	0x65, 0x14,        //   UNIT (Eng Rot:Angular Pos)
//	0x09, 0x39,        //   USAGE (Hat switch)
//	0x81, 0x42,        //   INPUT (Data,Var,Abs,Null)
//	0x65, 0x00,        //   UNIT (None)
//	0x95, 0x01,        //   REPORT_COUNT (1)
//	0x81, 0x01,        //   INPUT (Cnst,Ary,Abs)
//	0x26, 0xff, 0x00,  //   LOGICAL_MAXIMUM (255)
//	0x46, 0xff, 0x00,  //   PHYSICAL_MAXIMUM (255)
//	0x09, 0x30,        //   USAGE (X)
//	0x09, 0x31,        //   USAGE (Y)
//	0x09, 0x32,        //   USAGE (Z)
//	0x09, 0x35,        //   USAGE (Rz)
//	0x75, 0x08,        //   REPORT_SIZE (8)
//	0x95, 0x04,        //   REPORT_COUNT (6)
//	0x81, 0x02,        //   INPUT (Data,Var,Abs)
//	0x06, 0x00, 0xff,  //   USAGE_PAGE (Vendor Specific)
//	0x09, 0x20,        //   Unknown
//	0x09, 0x21,        //   Unknown
//	0x09, 0x22,        //   Unknown
//	0x09, 0x23,        //   Unknown
//	0x09, 0x24,        //   Unknown
//	0x09, 0x25,        //   Unknown
//	0x09, 0x26,        //   Unknown
//	0x09, 0x27,        //   Unknown
//	0x09, 0x28,        //   Unknown
//	0x09, 0x29,        //   Unknown
//	0x09, 0x2a,        //   Unknown
//	0x09, 0x2b,        //   Unknown
//	0x95, 0x0c,        //   REPORT_COUNT (12)
//	0x81, 0x02,        //   INPUT (Data,Var,Abs)
//	0x0a, 0x21, 0x26,  //   Unknown
//	0x95, 0x08,        //   REPORT_COUNT (8)
//	0xb1, 0x02,        //   FEATURE (Data,Var,Abs)
//	0xc0               // END_COLLECTION
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

//		0x09,								 // bLength
//		0x21,								 // bDescriptorType (HID)
//		0x11, 0x01,							 // bcdHID 1.11
//		0x00,								 // bCountryCode
//		0x01,								 // bNumDescriptors
//		0x22,								 // bDescriptorType[0] (HID)
//		sizeof(ps3_report_descriptor), 0x00, // wDescriptorLength[0] 90
};

#define CONFIG1_DESC_SIZE		(9+9+9+7)
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

//	    // configuration descriptor, USB spec 9.6.3, page 264-266, Table 9-10
//	9,						       // bLength;
//	2,						       // bDescriptorType;
//	LSB(CONFIG1_DESC_SIZE),        // wTotalLength
//	MSB(CONFIG1_DESC_SIZE),
//	1,	                           // bNumInterfaces
//	1,	                           // bConfigurationValue
//	0,	                           // iConfiguration
//	0x80,                          // bmAttributes
//	50,	                           // bMaxPower
//		// interface descriptor, USB spec 9.6.5, page 267-269, Table 9-12
//	9,				               // bLength
//	4,				               // bDescriptorType
//	GAMEPAD_INTERFACE,             // bInterfaceNumber
//	0,				               // bAlternateSetting
//	1,				               // bNumEndpoints
//	0x03,			               // bInterfaceClass (0x03 = HID)
//	0x00,			               // bInterfaceSubClass (0x00 = No Boot)
//	0x00,			               // bInterfaceProtocol (0x00 = No Protocol)
//	0,				               // iInterface
//		// HID interface descriptor, HID 1.11 spec, section 6.2.1
//	9,							   // bLength
//	0x21,						   // bDescriptorType
//	0x11, 0x01,					   // bcdHID
//	0,							   // bCountryCode
//	1,							   // bNumDescriptors
//	0x22,						   // bDescriptorType
//	sizeof(ps3_report_descriptor), // wDescriptorLength
//	0,
//		// endpoint descriptor, USB spec 9.6.6, page 269-271, Table 9-13
//	7,						 	   // bLength
//	5,						       // bDescriptorType
//	GAMEPAD_ENDPOINT | 0x80,       // bEndpointAddress
//	0x03,					       // bmAttributes (0x03=intr)
//	GAMEPAD_SIZE, 0,		       // wMaxPacketSize
//	1						       // bInterval (1 ms)
};
