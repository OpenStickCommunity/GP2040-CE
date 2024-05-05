#pragma once

#include <stdint.h>
#include "drivers/xboxog/xid/xid_driver.h"

#define XboxOriginalReport USB_XboxGamepad_InReport_t

static const uint8_t xboxoriginal_string_language[]     = { 0x09, 0x04 };
static const uint8_t xboxoriginal_string_manufacturer[] = "";
static const uint8_t xboxoriginal_string_product[]      = "";
static const uint8_t xboxoriginal_string_version[]      = "1.0";

static const uint8_t *xboxoriginal_string_descriptors[] __attribute__((unused)) =
{
	xboxoriginal_string_language,
	xboxoriginal_string_manufacturer,
	xboxoriginal_string_product,
	xboxoriginal_string_version
};

static const uint8_t *xboxoriginal_device_descriptor = (const uint8_t*)&XID_DESC_DEVICE;

static const uint8_t *xboxoriginal_configuration_descriptor = (const uint8_t*)&XID_DESC_CONFIGURATION;