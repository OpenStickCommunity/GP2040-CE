/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#pragma once

#include <stdint.h>
#include <pico/unique_id.h>
#include <cstring>

#include "drivers/shared/xgip_protocol.h"

#define XBONE_ENDPOINT_SIZE 64

// Gamepad-only GIP profile

static const uint8_t xbone_string_language[]    = { 0x09, 0x04 };
static const uint8_t xbone_string_manufacturer[] = "Open Stick Community";
static const uint8_t xbone_string_product[]      = "GP2040-CE (Xbox One)";
static const uint8_t xbone_string_version[]      = "1.0";

static const uint8_t *xbone_string_descriptors[] __attribute__((unused)) =
{
	xbone_string_language,
	xbone_string_manufacturer,
	xbone_string_product,
	xbone_string_version
};

static uint8_t uniqueSerial[2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 1] = {};
static const uint8_t xboxSecurityMethod[] = "Xbox Security Method 3, Version 1.00, \xa9 2005 Microsoft Corporation. All rights reserved.";
static const uint8_t xboxOSDescriptor[] = "MSFT100\x20\x00";

static const uint8_t XBOXONE_RUMBLE[] = {0x09, 0x00, 0x00, 0x09, 0x00, 0x0F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x00, 0xFF};
// power-on states and rumble-on with everything disabled
static const uint8_t XBOXONE_POWER_ON[] = {0x06, 0x62, 0x45, 0xb8, 0x77, 0x26, 0x2c, 0x55,
                                 0x53, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f};
static const uint8_t XBOXONE_POWER_ON_SINGLE[] = {0x00};
static const uint8_t XBOXONE_RUMBLE_ON[] = {0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0x00, 0xeb};
static const uint8_t XBOXONE_LED_ON[] = {0x00, 0x01, 0x14}; // 0x01 - LED on, 0x14 - Brightness


static const uint8_t * xbone_get_string_descriptor(int index) {
	if ( index == 3 ) {
		pico_get_unique_board_id_string((char *)uniqueSerial, sizeof(uniqueSerial));
        return uniqueSerial;
	} else if ( index == 4 ) { // security method used
		return xboxSecurityMethod;
	} else if ( index == 0xEE ) { // Microsoft OS descriptor string
		return xboxOSDescriptor;
	}

	return index >= 0 && static_cast<unsigned>(index) < sizeof(xbone_string_descriptors) / sizeof(xbone_string_descriptors[0])
        ? xbone_string_descriptors[index] : nullptr;
}

// MOVE THIS TO XBOX ONE DRIVER
typedef enum
{
    GIP_ACK_RESPONSE                = 0x01,    // Xbox One ACK
    GIP_ANNOUNCE                    = 0x02,    // Xbox One Announce
    GIP_KEEPALIVE                   = 0x03,    // Device status (legacy four-byte body)
    GIP_DEVICE_DESCRIPTOR           = 0x04,    // Xbox One Definition
    GIP_POWER_MODE_DEVICE_CONFIG    = 0x05,    // Xbox One Power Mode Config
    GIP_AUTH                        = 0x06,    // Xbox One Authentication
    GIP_VIRTUAL_KEYCODE             = 0x07,    // XBox One Guide button pressed
    GIP_CMD_RUMBLE                  = 0x09,    // Xbox One Rumble Command
    GIP_CMD_LED_ON                  = 0x0A,    // Xbox One (LED On)
    GIP_FINAL_AUTH                  = 0x1E,    // Extended command, retained by the auth relay
    GIP_INPUT_REPORT                = 0x20,    // Xbox One Input Report
    GIP_HID_REPORT                  = 0x21,    // Device-specific input, not HID descriptor
} XboxOneReport;

typedef struct
{
    GipHeader_t Header;

    uint8_t reserved : 1;
    uint8_t keepAlive : 1;
    uint8_t start : 1;  // menu
    uint8_t back : 1;   // view

    uint8_t a : 1;
    uint8_t b : 1;
    uint8_t x : 1;
    uint8_t y : 1;

    uint8_t dpadUp : 1;
    uint8_t dpadDown : 1;
    uint8_t dpadLeft : 1;
    uint8_t dpadRight : 1;

    uint8_t leftShoulder : 1;
    uint8_t rightShoulder : 1;
    uint8_t leftThumbClick : 1;
    uint8_t rightThumbClick : 1;

    // Core offsets 2/4: unsigned 10-bit triggers, carried in little-endian words.
    uint16_t leftTrigger;
    uint16_t rightTrigger;

    // Core offsets 6/8/10/12: signed 16-bit axes, positive Y points up.
    int16_t leftStickX;
    int16_t leftStickY;
    int16_t rightStickX;
    int16_t rightStickY;

} __attribute__((packed)) XboxOneGamepad_Data_t;

typedef struct {
    GipHeader_t Header;
    uint8_t reserved : 1;
    uint8_t keepAlive : 1;
    uint8_t start : 1;  // menu
    uint8_t back : 1;   // view
} __attribute__((packed)) XboxOneInputHeader_Data_t;

typedef struct
{
    GipHeader_t Header;
    uint8_t reserved;
    uint8_t mode;
    uint8_t brightness;
} __attribute__((packed)) XboxOneLED_Data_t;

static const uint8_t xbone_device_qualifier[] =
{
    0x0A,         // bLength
    0x06,         // bDescriptorType (Qualifier Type)
    0x00, 0x02,   // bcdUSB 2.00
    0xFF,         // bDeviceClass
    0x47,         // bDeviceSubClass (GIP)
    0xD0,         // bDeviceProtocol
    0x40,         // bMaxPacketSize0 64
    0x01,         // bNumConfigurations
    0x00          // bReserved
};

static const uint8_t xbone_device_descriptor[] =
{
    0x12,       // bLength
	0x01,       // bDescriptorType (Device)
	0x00, 0x02, // bcdUSB 2.00
	0xFF,	      // bDeviceClass
	0x47,	      // bDeviceSubClass (GIP)
	0xD0,	      // bDeviceProtocol
	0x40,	      // bMaxPacketSize0 64
	0x6F, 0x0E, // idVendor 0x045E = Xbox One  0x0E6F = SuperPDP  0x0079 = MagicBootS
	0xA4, 0x02, // idProduct 0x02A4 = SuperPDP Gamepad  0x02EA = Xbox One S  0x02D1 = Xbox One  0x2DD = Xbox One v2  0x1894 = MagicBootS
	0x01, 0x01, // bcdDevice 1.01, separate from GIP firmware version
	0x01,       // iManufacturer (String Index)
	0x02,       // iProduct (String Index)
	0x03,       // iSerialNumber (String Index)
	0x01,       // bNumConfigurations 1
};


static const uint8_t xbone_configuration_descriptor[] =
{
	0x09,        // bLength
	0x02,        // bDescriptorType (Configuration)
	0x20, 0x00,  // wTotalLength 32
	0x01,        // bNumInterfaces 1
	0x01,        // bConfigurationValue
	0x00,        // iConfiguration (String Index)
	0xA0,        // bmAttributes (USB_CONFIG_ATTRIBUTE_RESERVED | USB_CONFIG_ATTRIBUTE_REMOTEWAKEUP)
	0xFA,        // bMaxPower 500mA

	0x09,        // bLength
	0x04,        // bDescriptorType (Interface)
	0x00,        // bInterfaceNumber 0
	0x00,        // bAlternateSetting
	0x02,        // bNumEndpoints 2
	0xFF,        // bInterfaceClass
	0x47,        // bInterfaceSubClass
	0xD0,        // bInterfaceProtocol
	0x00,        // iInterface (String Index)

	0x07,        // bLength
	0x05,        // bDescriptorType (Endpoint)
	0x81,        // bEndpointAddress (IN/D2H)
	0x03,        // bmAttributes (Interrupt)
	0x40, 0x00,  // wMaxPacketSize 64
	0x01,        // bInterval 1 (unit depends on device speed)

	0x07,        // bLength
	0x05,        // bDescriptorType (Endpoint)
	0x01,        // bEndpointAddress (OUT/H2D), paired with IN 0x81
	0x03,        // bmAttributes (Interrupt)
	0x40, 0x00,  // wMaxPacketSize 64
	0x01,        // bInterval 1 (unit depends on device speed)
};

static uint8_t const * xbone_configuration_descriptor_cb(uint8_t index)
{
  return xbone_configuration_descriptor;
}
