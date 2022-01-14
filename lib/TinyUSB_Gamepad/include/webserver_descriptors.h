/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#pragma once

#include <stdint.h>
#include "tusb.h"

enum
{
	STRID_LANGID = 0,
	STRID_MANUFACTURER,
	STRID_PRODUCT,
	STRID_SERIAL,
	STRID_INTERFACE,
	STRID_MAC
};

static const uint8_t webserver_string_language[]     = { 0x09, 0x04 };
static const uint8_t webserver_string_manufacturer[] = "TinyUSB";
static const uint8_t webserver_string_product[]      = "USB Webserver";
static const uint8_t webserver_string_version[]      = "1.0";

static const uint8_t *webserver_string_descriptors[] =
{
	webserver_string_language,
	webserver_string_manufacturer,
	webserver_string_product,
	webserver_string_version
};

static const tusb_desc_device_t webserver_device_descriptor =
{
	.bLength            = sizeof(tusb_desc_device_t),
	.bDescriptorType    = TUSB_DESC_DEVICE,
	.bcdUSB             = 0x0200,

	// Use Interface Association Descriptor (IAD) device class
	.bDeviceClass       = TUSB_CLASS_MISC,
	.bDeviceSubClass    = MISC_SUBCLASS_COMMON,
	.bDeviceProtocol    = MISC_PROTOCOL_IAD,

	.bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

	.idVendor           = 0xCAFE,
	.idProduct          = 0x4001,
	.bcdDevice          = 0x0101,

	.iManufacturer      = 0x01,
	.iProduct           = 0x02,
	.iSerialNumber      = 0x03,

	.bNumConfigurations = 0x02 // multiple configurations
};

#define MAIN_CONFIG_TOTAL_LEN    (TUD_CONFIG_DESC_LEN + TUD_RNDIS_DESC_LEN)
#define ALT_CONFIG_TOTAL_LEN     (TUD_CONFIG_DESC_LEN + TUD_CDC_ECM_DESC_LEN)
#define CONFIG_TOTAL_LEN         (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN)

#define EPNUM_HID         0x01
#define EPNUM_NET_NOTIF   0x81
#define EPNUM_NET_OUT     0x02
#define EPNUM_NET_IN      0x82

static uint8_t const rndis_configuration[] =
{
	// Config number (index+1), interface count, string index, total length, attribute, power in mA
	TUD_CONFIG_DESCRIPTOR(1, 2, 0, MAIN_CONFIG_TOTAL_LEN, 0, 100),

	// Interface number, string index, EP notification address and size, EP data address (out, in) and size.
	TUD_RNDIS_DESCRIPTOR(0, STRID_INTERFACE, EPNUM_NET_NOTIF, 8, EPNUM_NET_OUT, EPNUM_NET_IN, CFG_TUD_NET_ENDPOINT_SIZE),
};

static uint8_t const ecm_configuration[] =
{
	// Config number (index+1), interface count, string index, total length, attribute, power in mA
	TUD_CONFIG_DESCRIPTOR(2, 2, 0, ALT_CONFIG_TOTAL_LEN, 0, 100),

	// Interface number, description string index, MAC address string index, EP notification address and size, EP data address (out, in), and size, max segment size.
	TUD_CDC_ECM_DESCRIPTOR(0, STRID_INTERFACE, STRID_MAC, EPNUM_NET_NOTIF, 64, EPNUM_NET_OUT, EPNUM_NET_IN, CFG_TUD_NET_ENDPOINT_SIZE, CFG_TUD_NET_MTU),
};

// Configuration array: RNDIS and CDC-ECM
// - Windows only works with RNDIS
// - MacOS only works with CDC-ECM
// - Linux will work on both
// Note index is Num-1x
static uint8_t const * const net_configuration_arr[] =
{
	rndis_configuration,
	ecm_configuration,
};
