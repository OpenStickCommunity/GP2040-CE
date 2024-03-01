/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#pragma once

#include <stdint.h>

#define NEOGEO_ENDPOINT_SIZE 64

// HAT report (4 bits)
#define NEOGEO_HAT_UP        0x00
#define NEOGEO_HAT_UPRIGHT   0x01
#define NEOGEO_HAT_RIGHT     0x02
#define NEOGEO_HAT_DOWNRIGHT 0x03
#define NEOGEO_HAT_DOWN      0x04
#define NEOGEO_HAT_DOWNLEFT  0x05
#define NEOGEO_HAT_LEFT      0x06
#define NEOGEO_HAT_UPLEFT    0x07
#define NEOGEO_HAT_NOTHING   0x0f

// Button report (16 bits)
#define NEOGEO_MASK_A       (1U <<  0)
#define NEOGEO_MASK_B       (1U <<  1)
#define NEOGEO_MASK_R1      (1U <<  2)
#define NEOGEO_MASK_C       (1U <<  3)
#define NEOGEO_MASK_D       (1U <<  4)
#define NEOGEO_MASK_R2      (1U <<  5)
#define NEOGEO_MASK_L1      (1U <<  6)
#define NEOGEO_MASK_L2      (1U <<  7)
#define NEOGEO_MASK_OPTIONS (1U <<  9)
#define NEOGEO_MASK_SELECT  (1U <<  10)
#define NEOGEO_MASK_START   (1U <<  11)
#define NEOGEO_JOYSTICK_MID  0x7f

typedef struct __attribute((packed, aligned(1)))
{
	uint16_t buttons;
	uint8_t hat;
	uint8_t const0;
	uint8_t const1;
	uint8_t const2;
	uint8_t const3;
	uint8_t const4;
	uint8_t const5;
	uint8_t const6;
	uint8_t const7;
	uint8_t const8;
	uint8_t const9;
	uint8_t const10;
	uint8_t const11;
	uint8_t const12;
	uint8_t const13;
	uint8_t const14;
	uint8_t const15;
	uint8_t const16;
	uint8_t const17;
} NeogeoReport;

typedef struct
{
	uint16_t buttons;
	uint8_t hat;
	uint8_t const0;
	uint8_t const1;
	uint8_t const2;
	uint8_t const3;
	uint8_t const4;
	uint8_t const5;
	uint8_t const6;
	uint8_t const7;
	uint8_t const8;
	uint8_t const9;
	uint8_t const10;
	uint8_t const11;
	uint8_t const12;
	uint8_t const13;
	uint8_t const14;
	uint8_t const15;
	uint8_t const16;
	uint8_t const17;
} NeogeoOutReport;

static const uint8_t neogeo_string_language[]     = { 0x09, 0x04 };
static const uint8_t neogeo_string_manufacturer[] = { 0x4a, 0x4a };
static const uint8_t neogeo_string_product[]      = { };
static const uint8_t neogeo_string_version[]      = { 0x20, 0x31, 0x2e, 0x31, 0x30 };

static const uint8_t *neogeo_string_descriptors[] __attribute__((unused)) =
{
	neogeo_string_language,
	neogeo_string_manufacturer,
	neogeo_string_product,
	neogeo_string_version
};

static const uint8_t neogeo_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x10, 0x01,  // bcdUSB 1.10
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass 
    0x00,        // bDeviceProtocol 
    0x40,        // bMaxPacketSize0 64
    0xBC, 0x20,  // idVendor 0x20BC
    0x00, 0x55,  // idProduct 0x5500
    0x00, 0x02,  // bcdDevice 4.00
    0x01,        // iManufacturer (String Index)
    0x00,        // iProduct (String Index)
    0x00,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};

static const uint8_t neogeo_hid_descriptor[] =
{
	0x09,        // bLength
	0x21,        // bDescriptorType (HID)
	0x11, 0x01,  // bcdHID 1.11
	0x00,        // bCountryCode
	0x01,        // bNumDescriptors
	0x22,        // bDescriptorType[0] (HID)
	0x56, 0x00   // wDescriptorLength[0] 86
};

static const uint8_t neogeo_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x42, 0x00,  // wTotalLength 66
    0x02,        // bNumInterfaces 2
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
    0x11, 0x01,  // bcdHID 1.11
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x96, 0x00,  // wDescriptorLength[0] 150

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x02,        // bEndpointAddress (OUT/H2D)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x01,        // bInterval 10 (unit depends on device speed)

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x01,        // bInterval 10 (unit depends on device speed)

    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x01,        // bInterfaceNumber 1
    0x00,        // bAlternateSetting
    0x01,        // bNumEndpoints 1
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
    0x83,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x20, 0x00,  // wMaxPacketSize 32
    0x01,        // bInterval 10 (unit depends on device speed)
};

static const uint8_t neogeo_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x01,        //   Physical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0F,        //   Report Count (15)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0F,        //   Usage Maximum (0x0F)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x25, 0x07,        //   Logical Maximum (7)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x75, 0x04,        //   Report Size (4)
    0x95, 0x01,        //   Report Count (1)
    0x65, 0x14,        //   Unit (System: English Rotation, Length: Centimeter)
    0x09, 0x39,        //   Usage (Hat switch)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    0x65, 0x00,        //   Unit (None)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x46, 0xFF, 0x00,  //   Physical Maximum (255)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x32,        //   Usage (Z)
    0x09, 0x35,        //   Usage (Rz)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x02,        //   Usage Page (Sim Ctrls)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x09, 0xC5,        //   Usage (Brake)
    0x09, 0xC4,        //   Usage (Accelerator)
    0x95, 0x02,        //   Report Count (2)
    0x75, 0x08,        //   Report Size (8)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x06, 0x00, 0xFF,  //   Usage Page (Vendor Defined 0xFF00)
    0x26, 0xFF, 0x03,  //   Logical Maximum (1023)
    0x46, 0xFF, 0x03,  //   Physical Maximum (1023)
    0x09, 0x20,        //   Usage (0x20)
    0x09, 0x21,        //   Usage (0x21)
    0x09, 0x22,        //   Usage (0x22)
    0x09, 0x23,        //   Usage (0x23)
    0x09, 0x24,        //   Usage (0x24)
    0x09, 0x25,        //   Usage (0x25)
    0x75, 0x10,        //   Report Size (16)
    0x95, 0x06,        //   Report Count (6)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x08,        //   Usage Page (LEDs)
    0x09, 0x43,        //   Usage (Slow Blink On Time)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0xFF, 0x00,  //   Physical Maximum (255)
    0x75, 0x08,        //   Report Size (8)
    0x95, 0x01,        //   Report Count (1)
    0x91, 0x82,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)
    0x09, 0x44,        //   Usage (Slow Blink Off Time)
    0x91, 0x82,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)
    0x09, 0x45,        //   Usage (Fast Blink On Time)
    0x91, 0x82,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)
    0x09, 0x46,        //   Usage (Fast Blink Off Time)
    0x91, 0x82,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Volatile)
    0xC0,              // End Collection
};