/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _GPUSBFUNCTION_H_
#define _GPUSBFUNCTION_H_

#include <stdint.h>

#include "tusb.h"
#include "enums.pb.h"

// A USB function's setting in an input mode. AUTO means PC only. In XInput
// it follows USBFunctionManager's host detection, and the other modes treat
// it as ON.
enum class USBFunctionSetting : uint8_t {
	OFF,
	ON,
	AUTO,
};

// Base class for a USB function, an extra HID interface such as Host Lighting
// or the keyboard. USBFunctionManager adds each active function to the input
// mode's configuration descriptor with the next interface number, HID
// instance and endpoint, and routes its HID requests and USB events to it.
// Its hooks run only while it is active.
//
// To add a USB function:
// - Derive a singleton with constant initialisers for all its members, so
//   getInstance() needs no first-use guard.
// - Add it with USBFunctionManager::addFunction() in GP2040::setup(). The
//   order sets the interface order. A function that hosts bind by device
//   path goes before those they find by usage. Run its own setup after
//   USBFunctionManager::setup(), once its HID instance is known.
// - Raise CFG_TUD_HID in tusb_config.h. Each function takes a HID instance.
// HostLighting is an example.
class GPUSBFunction {
public:
	// HID instance for this boot, or -1 when inactive. Set by
	// USBFunctionManager::setup() before USB starts.
	int getHIDInstance() const { return hidInstance; }
	void setHIDInstance(int instance) { hidInstance = instance; }

	// Setting for this input mode, OFF where the function does not apply
	virtual USBFunctionSetting setting(InputMode mode) = 0;

	// HID report descriptor and its length
	virtual const uint8_t* getReportDescriptor() = 0;
	virtual uint16_t getReportDescriptorLength() = 0;

	// True if the interface has an interrupt OUT endpoint as well as IN
	virtual bool hasOutEndpoint() = 0;

	// HID requests (core 0)
	virtual uint16_t getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) = 0;
	virtual void setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t* buffer, uint16_t bufsize) = 0;

	// USB session hooks (core 0, except usbEvent)
	virtual void reportComplete() {}
	virtual void usbEvent(uint32_t eventId) { (void)eventId; } // Sets flags only, in interrupt context
	virtual void usbUnmounted() {}
	virtual void usbSuspended() {}

	// Core 0 main loop, after tud_task()
	virtual void process() {}

private:
	int hidInstance = -1;
};

#endif
