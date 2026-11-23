/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#include "lightingreport.h"

#include <stdlib.h>
#include <string.h>

#include "pico/platform.h"
#include "device/dcd.h"

#include "hostlighting.h"
#include "inputmodes.h"
#include "storagemanager.h"

// Report 1 starts with one constant input byte, for hosts that need an input.
// Each value then takes an Ordinal usage, its name's string index and an
// 8-bit output.
#define DESCRIPTOR_HEAD \
	0x05, 0x01,        /* USAGE_PAGE (Generic Desktop)   */ \
	0x09, 0x00,        /* USAGE (Undefined)              */ \
	0xA1, 0x01,        /* COLLECTION (Application)       */ \
	0x15, 0x00,        /*   LOGICAL_MINIMUM (0)          */ \
	0x26, 0xFF, 0x00,  /*   LOGICAL_MAXIMUM (255)        */ \
	0x75, 0x08,        /*   REPORT_SIZE (8)              */ \
	0x95, 0x01,        /*   REPORT_COUNT (1)             */ \
	0x05, 0x0A,        /*   USAGE_PAGE (Ordinal)         */ \
	0x85, 0x01,        /*   REPORT_ID (1)                */ \
	0x09, 0x01,        /*   USAGE (Instance 1)           */ \
	0x81, 0x03         /*   INPUT (Cnst,Var,Abs)         */
#define DESCRIPTOR_END \
	0xC0               /* END_COLLECTION                 */

static const uint8_t descriptorHead[] = { DESCRIPTOR_HEAD };
static const uint8_t descriptorNoLights[] = { DESCRIPTOR_HEAD, DESCRIPTOR_END };

// Sizes of a report ID item, a value's usage and output items, and a string
// index item
#define ITEM_REPORT_ID_SIZE  2
#define ITEM_VALUE_SIZE      5
#define ITEM_STRING_SIZE     2

USBFunctionSetting LightingReport::setting(InputMode mode) {
	const LightingReportOptions& options = Storage::getInstance().getAddonOptions().lightingReportOptions;
	const InputModeAttributes* attributes = inputModeAttributes(mode);
	if (!options.enabled || (attributes == nullptr) || !(attributes->capabilities & INPUT_MODE_USB_FUNCTIONS))
		return USBFunctionSetting::OFF;
	// XInput takes the XInput setting, as its composite identity is PC only
	if (mode != INPUT_MODE_XINPUT)
		return USBFunctionSetting::ON;
	switch (options.xinputMode) {
		case LIGHTING_REPORT_XINPUT_MODE_ON:
			return USBFunctionSetting::ON;
		case LIGHTING_REPORT_XINPUT_MODE_AUTO:
			return USBFunctionSetting::AUTO;
		default:
			return USBFunctionSetting::OFF;
	}
}

// The string index of a value's name, 0 past the named lights
uint8_t LightingReport::valueString(uint16_t value) {
	if (value >= (NAMED_LIGHTS * CHANNELS))
		return 0;
	uint32_t index = STRING_FIRST + value;
	if (index >= STRING_MS_OS)
		index++;
	return (uint8_t)index;
}

void LightingReport::build() {
	const uint16_t lights = HostLighting::getInstance().getLightCount();
	const uint16_t valueCount = lights * CHANNELS;
	const uint16_t reports = (lights + LIGHTS_PER_REPORT - 1) / LIGHTS_PER_REPORT;
	uint16_t length = sizeof(descriptorHead) + 1;
	if (reports > 1)
		length += (reports - 1) * ITEM_REPORT_ID_SIZE;
	for (uint16_t v = 0; v < valueCount; v++)
		length += ITEM_VALUE_SIZE + ((valueString(v) != 0) ? ITEM_STRING_SIZE : 0);

	if (lights == 0) {
		descriptor = (uint8_t*)descriptorNoLights;
		descriptorLength = sizeof(descriptorNoLights);
		return;
	}

	// The SDK's malloc and calloc panic when they fail
	uint8_t* built = (uint8_t*)malloc(length);
	uint8_t* lightValues = (uint8_t*)calloc(valueCount, 1);
	uint16_t at = sizeof(descriptorHead);
	memcpy(built, descriptorHead, at);
	for (uint16_t v = 0; v < valueCount; v++) {
		if ((v != 0) && ((v % (LIGHTS_PER_REPORT * CHANNELS)) == 0)) {
			built[at++] = 0x85;                                   // REPORT_ID
			built[at++] = (uint8_t)(1 + (v / (LIGHTS_PER_REPORT * CHANNELS)));
		}
		built[at++] = 0x0A;                                       // USAGE (Instance v + 1)
		built[at++] = (uint8_t)((v + 1) & 0xFF);
		built[at++] = (uint8_t)((v + 1) >> 8);
		const uint8_t string = valueString(v);
		if (string != 0) {
			built[at++] = 0x79;                                   // STRING_INDEX
			built[at++] = string;
		}
		built[at++] = 0x91;                                       // OUTPUT (Data,Var,Abs)
		built[at++] = 0x02;
	}
	built[at++] = 0xC0;                                           // END_COLLECTION

	values = lightValues;
	lightCount = lights;
	descriptorLength = at;
	descriptor = built;
}

const uint8_t* LightingReport::getReportDescriptor() {
	if (descriptor == nullptr)
		build();
	return descriptor;
}

uint16_t LightingReport::getReportDescriptorLength() {
	if (descriptor == nullptr)
		build();
	return descriptorLength;
}

uint16_t LightingReport::getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
	// The constant input byte, the only input report
	if ((report_type != HID_REPORT_TYPE_INPUT) || (report_id != 1) || (reqlen == 0))
		return 0;
	buffer[0] = 0;
	return 1;
}

void LightingReport::setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t* buffer, uint16_t bufsize) {
	// From the OUT endpoint the report ID leads the values. SET_REPORT passes
	// it apart.
	if (report_id == 0) {
		if (bufsize == 0)
			return;
		report_id = buffer[0];
		buffer++;
		bufsize--;
	}
	if ((report_type != HID_REPORT_TYPE_OUTPUT) || (report_id == 0))
		return;
	const uint32_t first = (uint32_t)(report_id - 1) * LIGHTS_PER_REPORT;
	if (first >= lightCount)
		return;
	const uint16_t count = (uint16_t)(((lightCount - first) < LIGHTS_PER_REPORT) ? (lightCount - first) : LIGHTS_PER_REPORT);
	if (bufsize < (count * CHANNELS))
		return;

	sequence = sequence + 1;
	__mem_fence_release();
	// A report after the values were hidden starts from every light off
	if (!showing)
		memset(values, 0, lightCount * CHANNELS);
	memcpy(&values[first * CHANNELS], buffer, count * CHANNELS);
	__mem_fence_release();
	sequence = sequence + 1;
	showing = true;
}

// A value's name, its light's name from Host Lighting and the channel
const uint16_t* LightingReport::getString(uint8_t index) {
	if ((lightCount == 0) || (index < STRING_FIRST) || (index == STRING_MS_OS))
		return nullptr;
	const uint16_t value = (uint16_t)(index - STRING_FIRST - ((index > STRING_MS_OS) ? 1 : 0));
	if ((value >= (lightCount * CHANNELS)) || (value >= (NAMED_LIGHTS * CHANNELS)))
		return nullptr;

	static const char channels[CHANNELS] = { 'R', 'G', 'B' };
	char text[sizeof(name) / sizeof(name[0])];
	HostLighting::getInstance().getLightName(value / CHANNELS, text, sizeof(text) - 2);
	size_t length = strlen(text);
	text[length++] = ' ';
	text[length++] = channels[value % CHANNELS];
	for (size_t i = 0; i < length; i++)
		name[1 + i] = (uint8_t)text[i];
	name[0] = (uint16_t)((TUSB_DESC_STRING << 8) | (2 * length + 2));
	return name;
}

// Interrupt context, so before tud_task() has applied the reports queued
// ahead of the reset. process() hides the values after them.
void LightingReport::usbEvent(uint32_t eventId) {
	if (eventId == DCD_EVENT_BUS_RESET)
		busReset = true;
}

// Called every pass after tud_task(). A bus reset hides the values, as an
// unmount does.
void __not_in_flash_func(LightingReport::process)() {
	if (!busReset)
		return;
	busReset = false;
	showing = false;
}

void LightingReport::usbUnmounted() {
	showing = false;
}

void LightingReport::usbSuspended() {
	showing = false;
}
