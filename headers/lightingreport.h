/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _LIGHTINGREPORT_H_
#define _LIGHTINGREPORT_H_

#include <stdint.h>

#include "gpusbfunction.h"

// Default for LightingReportOptions.enabled, overridable per board config
#ifndef LIGHTING_REPORT_ENABLED
#define LIGHTING_REPORT_ENABLED 0
#endif

// Default XInput mode, 1 for AUTO and 0 for OFF, as HOST_LIGHTING_XINPUT
#ifndef LIGHTING_REPORT_XINPUT
#define LIGHTING_REPORT_XINPUT 1
#endif

// The lighting report, a USB function with R, G and B output values for each
// light in Host Lighting's light order, each named by a string descriptor. Its
// own add-on sets it (LightingReportOptions). The values show from the first
// report until USB suspends, unmounts or resets, under a Host Lighting session.
class LightingReport : public GPUSBFunction {
public:
	LightingReport(LightingReport const&) = delete;
	void operator=(LightingReport const&) = delete;
	static LightingReport& getInstance()
	{
		static LightingReport instance;
		return instance;
	}

	USBFunctionSetting setting(InputMode mode) override;
	const uint8_t* getReportDescriptor() override;
	uint16_t getReportDescriptorLength() override;
	bool hasOutEndpoint() override { return true; }
	uint16_t getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) override;
	void setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t* buffer, uint16_t bufsize) override;
	const uint16_t* getString(uint8_t index) override;
	void usbEvent(uint32_t eventId) override;
	void usbUnmounted() override;
	void usbSuspended() override;

	// Hides the values after a bus reset
	void process() override;

	// Core 1, from HostLighting::render(). The values are shown while
	// isShowing(). Core 0 makes the sequence odd while it writes them, as
	// Host Lighting's live frame.
	bool isShowing() const { return showing; }
	uint32_t getSequence() const { return sequence; }
	uint16_t getLightCount() const { return lightCount; }
	const uint8_t* getLightValues(uint16_t light) const { return &values[light * CHANNELS]; }

private:
	// Every member has a constant initialiser, so the object is built at
	// compile time and getInstance() needs no first-use guard
	LightingReport() = default;

	// R, G and B per light. A report holds a report ID and the values of up
	// to 21 lights, as TinyUSB's HID buffers hold 64 bytes.
	static constexpr uint8_t CHANNELS = 3;
	static constexpr uint8_t LIGHTS_PER_REPORT = (CFG_TUD_HID_EP_BUFSIZE - 1) / CHANNELS;
	// The value names start above the input modes' own strings and skip the
	// MS OS string descriptor's index. Only whole lights are named.
	static constexpr uint8_t STRING_FIRST = 0x10;
	static constexpr uint8_t STRING_MS_OS = 0xEE;
	static constexpr uint16_t NAMED_LIGHTS = (0x100 - STRING_FIRST - 1) / CHANNELS;

	void build();
	uint8_t valueString(uint16_t value);

	// Built at the first descriptor request in tud_task(), after the LED
	// add-on has registered the lights. Allocated only when the function is
	// active, so never in the web configurator.
	uint8_t* descriptor = nullptr;
	uint16_t descriptorLength = 0;
	uint8_t* values = nullptr;
	uint16_t lightCount = 0;

	volatile uint32_t sequence = 0;
	volatile bool showing = false;
	volatile bool busReset = false; // set in interrupt context

	// A name as a string descriptor, a header and up to 23 UTF-16 characters
	uint16_t name[24] = {};
};

#endif
