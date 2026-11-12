/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#include "usbfunctionmanager.h"

#include <string.h>

#include "hardware/watchdog.h"
#include "pico/platform.h"
#include "device/dcd.h"

#include "drivermanager.h"
#include "gamepad.h"
#include "inputmodes.h"
#include "system.h"
#include "usbdriver.h"

// Watchdog scratch register for the XInput AUTO host verdict. scratch[5]
// holds System::reboot's BootMode.
#define XINPUT_COMPOSITE_VERDICT_SCRATCH   6
#define XINPUT_COMPOSITE_VERDICT_PC_HOST   0x484C5043 // "HLPC"
#define XINPUT_COMPOSITE_DETECT_MS         4000

// Each function adds interface, HID and IN endpoint descriptors, plus an OUT
// endpoint descriptor if it has one. Endpoints are 64 bytes, polled every
// 1 ms.
#define FUNCTION_INTERFACE_SIZE   (9 + 9 + 7)
#define FUNCTION_OUT_SIZE         7
#define FUNCTION_ENDPOINT_SIZE    CFG_TUD_HID_EP_BUFSIZE
#define FUNCTION_INTERVAL_MS      1

// Extended compatible ID descriptor sizes, a header and one entry per
// interface
#define COMPATIBLE_IDS_HEADER_SIZE    16
#define COMPATIBLE_IDS_FUNCTION_SIZE  24

void USBFunctionManager::addFunction(GPUSBFunction& function) {
	if (functionCount < MAX_FUNCTIONS)
		functions[functionCount++] = &function;
}

void USBFunctionManager::setup() {
	// A boot into another mode clears the XInput AUTO verdict, because the
	// board can then move to a console without losing power. The web
	// configurator keeps it, as only a PC reaches it.
	DriverManager& drivers = DriverManager::getInstance();
	const InputMode mode = drivers.getInputMode();
	if (!drivers.isConfigMode() && (mode != INPUT_MODE_XINPUT))
		watchdog_hw->scratch[XINPUT_COMPOSITE_VERDICT_SCRATCH] = 0;

	for (uint8_t i = 0; i < functionCount; i++)
		functions[i]->setHIDInstance(-1);
	activeCount = 0;
	xinputAutoDetect = false;
	xinputComposite = false;
	const InputModeAttributes* attributes = inputModeAttributes(mode);
	if (drivers.isConfigMode() || (attributes == nullptr) ||
			!(attributes->capabilities & INPUT_MODE_USB_FUNCTIONS))
		return;

	// The mode's own interface is HID in every mode with USB functions except
	// XInput, where it is vendor class
	const bool xinput = (mode == INPUT_MODE_XINPUT);
	firstInstance = xinput ? 0 : 1;
	if (xinput) {
		// The setting and the AUTO verdict are fixed for the boot. AUTO reboots
		// to change the verdict.
		const USBFunctionSetting setting = xinputSetting();
		xinputAutoDetect = (setting == USBFunctionSetting::AUTO);
		xinputComposite = (setting == USBFunctionSetting::ON) || (xinputAutoDetect &&
			(watchdog_hw->scratch[XINPUT_COMPOSITE_VERDICT_SCRATCH] == XINPUT_COMPOSITE_VERDICT_PC_HOST));
		if (!xinputComposite)
			return;
	}
	for (uint8_t i = 0; i < functionCount; i++) {
		if (functions[i]->setting(mode) == USBFunctionSetting::OFF)
			continue;
		// TinyUSB opens at most CFG_TUD_HID HID interfaces. One more would
		// leave its interface unclaimed and fail the configuration.
		if ((firstInstance + activeCount) >= CFG_TUD_HID)
			break;
		functions[i]->setHIDInstance(firstInstance + activeCount);
		active[activeCount++] = functions[i];
	}
}

USBFunctionSetting USBFunctionManager::xinputSetting() {
	USBFunctionSetting combined = USBFunctionSetting::OFF;
	for (uint8_t i = 0; i < functionCount; i++) {
		USBFunctionSetting setting = functions[i]->setting(INPUT_MODE_XINPUT);
		if (setting == USBFunctionSetting::ON)
			return USBFunctionSetting::ON;
		if (setting == USBFunctionSetting::AUTO)
			combined = USBFunctionSetting::AUTO;
	}
	return combined;
}

// Called every pass in XInput AUTO. Once the identity is settled it only
// reads flags.
void __not_in_flash_func(USBFunctionManager::processXInputAutoDetect)(bool consoleAuthSeen) {
	if (!xinputAutoDetect)
		return;

	if (isXInputCompositeActive()) {
		// After the composite has mounted, a bus reset that follows a suspend
		// means a switch moved the board to another host. The board reboots to
		// the stock identity so a console never sees the composite. A port
		// reset on the PC has no suspend before it and keeps the composite.
		if (!compositeMounted) {
			if (get_usb_mounted() && !get_usb_suspended()) {
				compositeMounted = true;
				resetAfterSuspend = false;
			}
			return;
		}
		if (resetAfterSuspend) {
			watchdog_hw->scratch[XINPUT_COMPOSITE_VERDICT_SCRATCH] = 0;
			System::reboot(System::BootMode::GAMEPAD);
		}
		return;
	}

	// Stock identity, decided once per boot
	if (stockSettled)
		return;
	if (consoleAuthSeen) {
		stockSettled = true;
		return;
	}
	if (!get_usb_mounted() || get_usb_suspended()) {
		enumeratedSinceMs = 0;
		return;
	}
	uint32_t now = getMillis();
	if (enumeratedSinceMs == 0) {
		enumeratedSinceMs = now;
		return;
	}
	if ((now - enumeratedSinceMs) >= XINPUT_COMPOSITE_DETECT_MS) {
		watchdog_hw->scratch[XINPUT_COMPOSITE_VERDICT_SCRATCH] = XINPUT_COMPOSITE_VERDICT_PC_HOST;
		System::reboot(System::BootMode::GAMEPAD);
	}
}

// ---------------------------------------------------------------------------
// Descriptors

const uint8_t* USBFunctionManager::getConfigurationDescriptor(const uint8_t* base) {
	if (activeCount == 0)
		return base;

	// The mode's own interface count and highest endpoint number
	const uint16_t baseLength = (uint16_t)(base[2] | (base[3] << 8));
	uint8_t interfaces = base[4];
	uint8_t endpoint = 0;
	for (uint16_t at = 0; (at + 2) < baseLength; at += base[at]) {
		if (base[at] == 0)
			break;
		if ((base[at + 1] == TUSB_DESC_ENDPOINT) && ((base[at + 2] & 0x0F) > endpoint))
			endpoint = base[at + 2] & 0x0F;
	}
	if (baseLength > DESCRIPTOR_MAX)
		return base;
	memcpy(descriptor, base, baseLength);

	// Each function takes the next interface number and endpoint
	uint16_t length = baseLength;
	for (uint8_t i = 0; i < activeCount; i++) {
		GPUSBFunction& function = *active[i];
		const bool out = function.hasOutEndpoint();
		const uint16_t size = FUNCTION_INTERFACE_SIZE + (out ? FUNCTION_OUT_SIZE : 0);
		if ((length + size) > DESCRIPTOR_MAX)
			break;
		const uint16_t reportLength = function.getReportDescriptorLength();
		endpoint++;
		const uint8_t interfaceDescriptor[] = {
			// Interface, USB 2.0 9.6.5
			9, TUSB_DESC_INTERFACE, interfaces, 0, (uint8_t)(out ? 2 : 1),
			TUSB_CLASS_HID, 0, 0, 0,                           // no boot protocol, no string
			// HID, HID 1.11 6.2.1
			9, HID_DESC_TYPE_HID, 0x11, 0x01, 0, 1,
			HID_DESC_TYPE_REPORT, (uint8_t)(reportLength & 0xFF), (uint8_t)(reportLength >> 8),
			// IN endpoint, then OUT, USB 2.0 9.6.6
			7, TUSB_DESC_ENDPOINT, (uint8_t)(0x80 | endpoint), TUSB_XFER_INTERRUPT,
			(uint8_t)(FUNCTION_ENDPOINT_SIZE & 0xFF), (uint8_t)(FUNCTION_ENDPOINT_SIZE >> 8), FUNCTION_INTERVAL_MS,
			7, TUSB_DESC_ENDPOINT, endpoint, TUSB_XFER_INTERRUPT,
			(uint8_t)(FUNCTION_ENDPOINT_SIZE & 0xFF), (uint8_t)(FUNCTION_ENDPOINT_SIZE >> 8), FUNCTION_INTERVAL_MS,
		};
		memcpy(&descriptor[length], interfaceDescriptor, size);
		length += size;
		interfaces++;
	}

	// Total length and interface count
	descriptor[2] = (uint8_t)(length & 0xFF);
	descriptor[3] = (uint8_t)(length >> 8);
	descriptor[4] = interfaces;
	return descriptor;
}

const uint8_t* USBFunctionManager::getXInputCompatibleIDs() {
	const uint8_t count = 1 + activeCount;
	const uint16_t length = COMPATIBLE_IDS_HEADER_SIZE + (count * COMPATIBLE_IDS_FUNCTION_SIZE);
	memset(descriptor, 0, sizeof(descriptor));
	descriptor[0] = (uint8_t)(length & 0xFF); // dwLength
	descriptor[1] = (uint8_t)(length >> 8);
	descriptor[5] = 0x01;                     // bcdVersion 1.00
	descriptor[6] = 0x04;                     // wIndex 4, extended compatible ID
	descriptor[8] = count;                    // bCount
	for (uint8_t i = 0; i < count; i++) {
		uint8_t* function = &descriptor[COMPATIBLE_IDS_HEADER_SIZE + (i * COMPATIBLE_IDS_FUNCTION_SIZE)];
		function[0] = i;    // bFirstInterfaceNumber
		function[1] = 0x01; // reserved
		// Interface 0 binds the XUSB driver. The functions have no compatible
		// ID and bind by HID class.
		if (i == 0)
			memcpy(&function[2], "XUSB10", 6);
	}
	return descriptor;
}

// ---------------------------------------------------------------------------
// USB session and main loop

// Called from tud_event_hook_cb in interrupt context. Sets flags only.
void USBFunctionManager::usbEvent(uint32_t eventId) {
	if (xinputAutoDetect) {
		switch (eventId) {
			case DCD_EVENT_BUS_RESET:
				if (suspendSinceResume)
					resetAfterSuspend = true;
				suspendSinceResume = false;
				break;
			case DCD_EVENT_SUSPEND:
				suspendSinceResume = true;
				break;
			case DCD_EVENT_RESUME:
			case DCD_EVENT_UNPLUGGED:
				suspendSinceResume = false;
				break;
			default:
				break;
		}
	}
	for (uint8_t i = 0; i < activeCount; i++)
		active[i]->usbEvent(eventId);
}

void USBFunctionManager::usbUnmounted() {
	for (uint8_t i = 0; i < activeCount; i++)
		active[i]->usbUnmounted();
}

void USBFunctionManager::usbSuspended() {
	for (uint8_t i = 0; i < activeCount; i++)
		active[i]->usbSuspended();
}

void __not_in_flash_func(USBFunctionManager::process)() {
	for (uint8_t i = 0; i < activeCount; i++)
		active[i]->process();
}
