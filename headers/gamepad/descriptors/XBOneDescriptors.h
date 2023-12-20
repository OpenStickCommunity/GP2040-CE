/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2023 OpenStickCommunity (gp2040-ce.info)
 */

#pragma once

#include <stdint.h>
#include <pico/unique_id.h>
#include <cstring>

#define XBONE_ENDPOINT_SIZE 64

// 0x80 = std. device
// 

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

static uint8_t uniqueSerial[] = "012345678ABCDEFGH";
static const uint8_t xboxSecurityMethod[] = "Xbox Security Method 3, Version 1.00, \xa9 2005 Microsoft Corporation. All rights reserved.";
static const uint8_t xboxOSDescriptor[] = "MSFT100\x20\x00";

static const uint8_t * xbone_get_string_descriptor(int index) {
	if ( index == 3 ) {
		// Generate a serial number from the pico's unique ID
		pico_unique_board_id_t id;
		pico_get_unique_board_id(&id);
		memcpy(uniqueSerial, (uint8_t*)&id, PICO_UNIQUE_BOARD_ID_SIZE_BYTES);
        return uniqueSerial;
	} else if ( index == 4 ) { // security method used
		return xboxSecurityMethod;
	} else if ( index == 0xEE ) { // ONLY WINDOWS DOES THIS??
		return xboxOSDescriptor;
	}

	return xbone_string_descriptors[index];
}

// MOVE THIS TO XBOX ONE DRIVER
typedef enum
{
    GIP_ACK_RESPONSE                = 0x01,    // Xbox One ACK
    GIP_ANNOUNCE                    = 0x02,    // Xbox One Announce
    GIP_KEEPALIVE                   = 0x03,    // Xbox One Keep-Alive
    GIP_DEVICE_DESCRIPTOR           = 0x04,    // Xbox One Definition
    GIP_POWER_MODE_DEVICE_CONFIG    = 0x05,    // Xbox One Power Mode Config
    GIP_AUTH                        = 0x06,    // Xbox One Authentication
    GIP_VIRTUAL_KEYCODE             = 0x07,    // XBox One Guide button pressed
    GIP_CMD_RUMBLE                  = 0x09,    // Xbox One Rumble Command
    GIP_CMD_WAKEUP                  = 0x0A,    // Xbox One (Wake-up Maybe?)
    GIP_FINAL_AUTH                  = 0x1E,    // Xbox One (Final auth?)
    GIP_INPUT_REPORT                = 0x20,    // Xbox One Input Report
    GIP_HID_REPORT                  = 0x21,    // Xbox One HID Report
} XboxOneReport;

typedef struct
{
    uint8_t command;
    uint8_t client : 4;
    uint8_t needsAck : 1;
    uint8_t internal : 1;
    uint8_t chunkStart : 1;
    uint8_t chunked : 1;
    uint8_t sequence;
    uint8_t length;
} __attribute__((packed)) GipHeader_t;

#define GIP_HEADER(packet, cmd, isInternal, seq) \
    packet->Header.command = cmd;                \
    packet->Header.internal = isInternal;        \
    packet->Header.sequence = seq;               \
    packet->Header.client = 0;                   \
    packet->Header.needsAck = 0;                 \
    packet->Header.chunkStart = 0;               \
    packet->Header.chunked = 0;                  \
    packet->Header.length = sizeof(*packet) - sizeof(GipHeader_t);

typedef struct
{
    GipHeader_t Header;

    uint8_t sync : 1;
    uint8_t guide : 1;
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

    uint16_t leftTrigger;
    uint16_t rightTrigger;

    int16_t leftStickX;
    int16_t leftStickY;
    int16_t rightStickX;
    int16_t rightStickY;

    uint8_t reserved[18]; // 18-byte padding at the end
} __attribute__((packed)) XboxOneGamepad_Data_t;

typedef struct {
    GipHeader_t Header;
    uint8_t sync : 1;
    uint8_t guide : 1;
    uint8_t start : 1;  // menu
    uint8_t back : 1;   // view
} __attribute__((packed)) XboxOneInputHeader_Data_t;

static const uint8_t xbone_device_qualifier[] =
{
    0x0A,         // bLength
    0x06,         // bDescriptorType (Qualifier Type)
    0x00, 0x02,   // bcdUSB 2.00
    0xFF,         // bDeviceClass
    0xFF,         // bDeviceSubClass
    0xFF,         // bDeviceProtocol
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
	0xFF,	      // bDeviceSubClass
	0xFF,	      // bDeviceProtocol
	0x40,	      // bMaxPacketSize0 64
	0x6F, 0x0E, // idVendor 0x045E = Xbox One  0x0E6F = SuperPDP  0x0079 = MagicBootS
	0xA4, 0x02, // idProduct 0x02A4 = SuperPDP Gamepad  0x02EA = Xbox One S  0x02D1 = Xbox One  0x2DD = Xbox One v2  0x1894 = MagicBootS
	0x01, 0x01, // bcdDevice 1.01?
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
	0x02,        // bEndpointAddress (OUT/H2D)
	0x03,        // bmAttributes (Interrupt)
	0x40, 0x00,  // wMaxPacketSize 64
	0x01,        // bInterval 1 (unit depends on device speed)
};

static uint8_t const * xbone_configuration_descriptor_cb(uint8_t index)
{
  return xbone_configuration_descriptor;
}
