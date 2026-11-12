/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _USBFUNCTIONMANAGER_H_
#define _USBFUNCTIONMANAGER_H_

#include <stdint.h>

#include "gpusbfunction.h"

// Manages the USB functions that follow the input mode's own interface in the
// modes with INPUT_MODE_USB_FUNCTIONS. In XInput it also selects the composite
// identity, 24C6:5BAD with per-interface driver binding, so the functions
// enumerate on Windows. The composite is PC-only because a console
// authenticates the stock 045E:028E identity. With AUTO the stock identity
// boots first, and the board reboots into the composite once a host
// enumerates it without console authentication.
class USBFunctionManager {
public:
	USBFunctionManager(USBFunctionManager const&) = delete;
	void operator=(USBFunctionManager const&) = delete;
	static USBFunctionManager& getInstance()
	{
		static USBFunctionManager instance;
		return instance;
	}

	// TinyUSB opens at most CFG_TUD_HID HID interfaces
	static constexpr uint8_t MAX_FUNCTIONS = CFG_TUD_HID;

	// Called from GP2040::setup() before setup(), in interface order
	void addFunction(GPUSBFunction& function);

	// Called from GP2040::setup() after the functions are added and before USB
	// starts. Clears the XInput AUTO verdict outside XInput and fixes the active
	// functions, their HID instances and the flags below for this boot.
	void setup();

	// Fixed for the boot and read inline by the main loop, so a board with no
	// active function and no XInput AUTO skips the per-pass hooks.
	bool hasActiveFunctions() const { return activeCount != 0; }
	bool isXInputAutoDetect() const { return xinputAutoDetect; } // XInput with an AUTO setting

	// True if XInput uses the composite identity this boot. A function is ON,
	// or one is AUTO and auto-detect recorded a PC host.
	bool isXInputCompositeActive() const { return xinputComposite; }

	// XInput AUTO, called from the XInput driver's process loop. Records a PC
	// host and reboots into the composite when a host enumerates without
	// console authentication. Reboots to the stock identity after a bus reset
	// that follows a suspend of the composite.
	void processXInputAutoDetect(bool consoleAuthSeen);

	// Returns the input mode's configuration descriptor with the active
	// functions appended, or base when none is active
	const uint8_t* getConfigurationDescriptor(const uint8_t* base);

	// The XInput composite's MS OS 1.0 extended compatible ID descriptor.
	// Interface 0 binds XUSB and the functions bind by HID class. Its length
	// is its first field.
	const uint8_t* getXInputCompatibleIDs();

	// Returns the active function with this HID instance, or nullptr. The HID
	// callbacks call it for every instance, including the input mode's own.
	GPUSBFunction* getFunctionForHIDInstance(uint8_t instance) const {
		if ((instance < firstInstance) || ((instance - firstInstance) >= activeCount))
			return nullptr;
		return active[instance - firstInstance];
	}

	// USB session hooks for the active functions (core 0, except usbEvent)
	void usbEvent(uint32_t eventId); // Sets flags only, in interrupt context
	void usbUnmounted();
	void usbSuspended();

	// Called from GP2040::run() on core 0 after tud_task(). Runs every active
	// function.
	void process();

private:
	// Every member has a constant initialiser, so the object is built at
	// compile time and getInstance() needs no first-use guard. The USB
	// interrupt calls it.
	USBFunctionManager() = default;

	// One buffer holds either the configuration descriptor or XInput's
	// compatible IDs, since the host requests them one at a time. It fits the
	// largest base descriptor of a mode with USB functions (XInput's
	// composite, 49 bytes) plus each function's interface with both endpoints.
	static constexpr uint16_t BASE_DESCRIPTOR_MAX = 64;
	static constexpr uint16_t FUNCTION_DESCRIPTOR_MAX = 9 + 9 + 7 + 7;
	static constexpr uint16_t DESCRIPTOR_MAX = BASE_DESCRIPTOR_MAX + (MAX_FUNCTIONS * FUNCTION_DESCRIPTOR_MAX);
	static_assert((16 + (24 * (1 + MAX_FUNCTIONS))) <= DESCRIPTOR_MAX, "compatible IDs fit the buffer");

	// Combined XInput setting of the functions. ON if any is ON, else AUTO if
	// any is AUTO.
	USBFunctionSetting xinputSetting();

	GPUSBFunction* functions[MAX_FUNCTIONS] = {};
	uint8_t functionCount = 0;

	// Functions active this boot, in interface order from HID instance
	// firstInstance
	GPUSBFunction* active[MAX_FUNCTIONS] = {};
	uint8_t activeCount = 0;
	uint8_t firstInstance = 0;
	bool xinputAutoDetect = false;
	bool xinputComposite = false;

	uint8_t descriptor[DESCRIPTOR_MAX] = {};

	// USB bus events, recorded in interrupt context
	volatile bool suspendSinceResume = false;
	volatile bool resetAfterSuspend = false;

	// XInput AUTO state. Whether the composite has mounted, whether the stock
	// identity is settled for this boot, and when the host enumerated the
	// board.
	bool compositeMounted = false;
	bool stockSettled = false;
	uint32_t enumeratedSinceMs = 0;
};

#endif
