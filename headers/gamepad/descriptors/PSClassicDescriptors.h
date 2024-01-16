/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#pragma once

#include <stdint.h>

#define PSCLASSIC_ENDPOINT_SIZE 64

// Button report (16 bits)
#define PSCLASSIC_MASK_TRIANGLE   (1U <<  0)
#define PSCLASSIC_MASK_CIRCLE     (1U <<  1)
#define PSCLASSIC_MASK_CROSS      (1U <<  2)
#define PSCLASSIC_MASK_SQUARE     (1U <<  3)
#define PSCLASSIC_MASK_L2         (1U <<  4)
#define PSCLASSIC_MASK_R2         (1U <<  5)
#define PSCLASSIC_MASK_L1         (1U <<  6)
#define PSCLASSIC_MASK_R1         (1U <<  7)
#define PSCLASSIC_MASK_START      (1U <<  8)
#define PSCLASSIC_MASK_SELECT     (1U <<  9)

#define PSCLASSIC_MASK_UP_LEFT    0x0000
#define PSCLASSIC_MASK_UP         0x0400
#define PSCLASSIC_MASK_UP_RIGHT   0x0800
#define PSCLASSIC_MASK_LEFT       0x1000
#define PSCLASSIC_MASK_CENTER     0x1400
#define PSCLASSIC_MASK_RIGHT      0x1800
#define PSCLASSIC_MASK_DOWN_LEFT  0x2000
#define PSCLASSIC_MASK_DOWN       0x2400
#define PSCLASSIC_MASK_DOWN_RIGHT 0x2800

#define PSCLASSIC_JOYSTICK_MID  0x7f

typedef struct __attribute((packed, aligned(1)))
{
    uint16_t buttons;  // 10 buttons (1 bit each)
} PSClassicReport;

static const uint8_t psclassic_string_language[]     = { 0x09, 0x04 };
static const uint8_t psclassic_string_manufacturer[] = "Sony Interactive Entertainment";
static const uint8_t psclassic_string_product[]      = "Controller";
static const uint8_t psclassic_string_version[]      = { };

static const uint8_t *psclassic_string_descriptors[] __attribute__((unused)) =
{
	psclassic_string_language,
	psclassic_string_manufacturer,
	psclassic_string_product,
	psclassic_string_version
};

static const uint8_t psclassic_device_descriptor[] =
{
    0x12,        // bLength
    0x01,        // bDescriptorType (Device)
    0x00, 0x02,  // bcdUSB 2.00
    0x00,        // bDeviceClass (Use class information in the Interface Descriptors)
    0x00,        // bDeviceSubClass 
    0x00,        // bDeviceProtocol 
    0x40,        // bMaxPacketSize0 64
    0x4C, 0x05,  // idVendor 0x054C
    0xDA, 0x0C,  // idProduct 0x0CDA
    0x00, 0x01,  // bcdDevice 2.00
    0x01,        // iManufacturer (String Index)
    0x02,        // iProduct (String Index)
    0x00,        // iSerialNumber (String Index)
    0x01,        // bNumConfigurations 1
};

static const uint8_t psclassic_hid_descriptor[] =
{
    0x09,        // bLength
    0x21,        // bDescriptorType (HID)
    0x11, 0x01,  // bcdHID 1.11
    0x00,        // bCountryCode
    0x01,        // bNumDescriptors
    0x22,        // bDescriptorType[0] (HID)
    0x31, 0x00,  // wDescriptorLength[0] 49
};

static const uint8_t psclassic_configuration_descriptor[] =
{
    0x09,        // bLength
    0x02,        // bDescriptorType (Configuration)
    0x22, 0x00,  // wTotalLength 34
    0x01,        // bNumInterfaces 1
    0x01,        // bConfigurationValue
    0x00,        // iConfiguration (String Index)
    0xA0,        // bmAttributes Remote Wakeup
    0x32,        // bMaxPower 100mA

    0x09,        // bLength
    0x04,        // bDescriptorType (Interface)
    0x00,        // bInterfaceNumber 0
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
    0x31, 0x00,  // wDescriptorLength[0] 49

    0x07,        // bLength
    0x05,        // bDescriptorType (Endpoint)
    0x81,        // bEndpointAddress (IN/D2H)
    0x03,        // bmAttributes (Interrupt)
    0x40, 0x00,  // wMaxPacketSize 64
    0x0A,        // bInterval 10 (unit depends on device speed)
};

static const uint8_t psclassic_report_descriptor[] =
{
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x0A,        //   Report Count (10)
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x0A,        //   Usage Maximum (0x0A)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x02,        //   Logical Maximum (2)
    0x35, 0x00,        //   Physical Minimum (0)
    0x45, 0x02,        //   Physical Maximum (2)
    0x75, 0x02,        //   Report Size (2)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0x75, 0x01,        //   Report Size (1)
    0x95, 0x02,        //   Report Count (2)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    0xC0,              // End Collection
};