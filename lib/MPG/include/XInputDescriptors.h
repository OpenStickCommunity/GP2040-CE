/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef XINPUT_DESCRIPTORS_H_
#define XINPUT_DESCRIPTORS_H_

#include <stdint.h>

#define XINPUT_REPORT_SIZE 20
#define XINPUT_UNKNOWN_DESCRIPTOR_SIZE 16

// Buttons 1 (8 bits)
#define XBOX_MASK_UP    (1U << 0)
#define XBOX_MASK_DOWN  (1U << 1)
#define XBOX_MASK_LEFT  (1U << 2)
#define XBOX_MASK_RIGHT (1U << 3)
#define XBOX_MASK_START (1U << 4)
#define XBOX_MASK_BACK  (1U << 5)
#define XBOX_MASK_LS    (1U << 6)
#define XBOX_MASK_RS    (1U << 7)

// Buttons 2 (8 bits)
#define XBOX_MASK_LB    (1U << 0)
#define XBOX_MASK_RB    (1U << 1)
#define XBOX_MASK_HOME  (1U << 2)
//#define UNUSED        (1U << 3)
#define XBOX_MASK_A     (1U << 4)
#define XBOX_MASK_B     (1U << 5)
#define XBOX_MASK_X     (1U << 6)
#define XBOX_MASK_Y     (1U << 7)

typedef struct {
	uint8_t report_id;
	uint8_t report_size;
	uint8_t buttons1;
	uint8_t buttons2;
	uint8_t lt;
	uint8_t rt;
	int16_t lx;
	int16_t ly;
	int16_t rx;
	int16_t ry;
	uint8_t _reserved[6];
} XInputReport;

static const char xinput_string_manfacturer[] = "Microsoft";
static const char xinput_string_product[] = "XInput STANDARD GAMEPAD";
static const char xinput_string_version[] = "1.0";

static const char *xinput_string_descriptors[] =
{
	(const char[]){0x09, 0x04},
	xinput_string_manfacturer,
	xinput_string_product,
	xinput_string_version
};

static const uint8_t xinput_device_descriptor[] =
{
	0x12,       // bLength
	0x01,       // bDescriptorType (Device)
	0x00, 0x02, // bcdUSB 2.00
	0xFF,	      // bDeviceClass
	0xFF,	      // bDeviceSubClass
	0xFF,	      // bDeviceProtocol
	0x40,	      // bMaxPacketSize0 64
	0x5E, 0x04, // idVendor 0x045E
	0x8E, 0x02, // idProduct 0x028E
	0x14, 0x01, // bcdDevice 2.14
	0x01,       // iManufacturer (String Index)
	0x02,       // iProduct (String Index)
	0x03,       // iSerialNumber (String Index)
	0x01,       // bNumConfigurations 1
};

static const uint8_t xinput_configuration_descriptor[] =
{
	//Configuration Descriptor:
	0x09,       // bLength
	0x02,       // bDescriptorType
	0x30, 0x00, // wTotalLength   (48 bytes)
	0x01,       // bNumInterfaces
	0x01,       // bConfigurationValue
	0x00,       // iConfiguration
	0x80,       // bmAttributes   (Bus-powered Device)
	0xFA,       // bMaxPower      (500 mA)

	//Interface Descriptor:
	0x09,       // bLength
	0x04,       // bDescriptorType
	0x00,       // bInterfaceNumber
	0x00,       // bAlternateSetting
	0x02,       // bNumEndPoints
	0xFF,       // bInterfaceClass      (Vendor specific)
	0x5D,       // bInterfaceSubClass
	0x01,       // bInterfaceProtocol
	0x00,       // iInterface

	//Unknown HID Descriptor:
	0x10,       // bLength
	0x21,       // bDescriptorType (HID)
	0x10, 0x01, // bcdHID 1.10
	0x01,	      // bCountryCode
	0x24,	      // bNumDescriptors
	0x81,	      // bDescriptorType[0] (Unknown 0x81)
	0x14, 0x03, // wDescriptorLength[0] 788
	0x00,       // bDescriptorType[1] (Unknown 0x00)
	0x03, 0x13, // wDescriptorLength[1] 4867
	0x02,       // bDescriptorType[2] (Unknown 0x02)
	0x00, 0x03, // wDescriptorLength[2] 768
	0x00,       // bDescriptorType[3] (Unknown 0x00)

	//Endpoint Descriptor:
	0x07,       // bLength
	0x05,       // bDescriptorType
	0x81,       // bEndpointAddress  (IN endpoint 1)
	0x03,       // bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
	0x20, 0x00, // wMaxPacketSize    (1 x 32 bytes)
	0x01,       // bInterval         (1 frames)

	//Endpoint Descriptor:
	0x07,       // bLength
	0x05,       // bDescriptorType
	0x02,       // bEndpointAddress  (OUT endpoint 2)
	0x03,       // bmAttributes      (Transfer: Interrupt / Synch: None / Usage: Data)
	0x20, 0x00, // wMaxPacketSize    (1 x 32 bytes)
	0x08,       // bInterval         (8 frames)
};

#endif
