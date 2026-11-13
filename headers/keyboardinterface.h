/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _KEYBOARDINTERFACE_H_
#define _KEYBOARDINTERFACE_H_

#include <stdint.h>

#include "config.pb.h"
#include "gpusbfunction.h"
#include "drivers/keyboard/KeyboardDescriptors.h"

class GPEvent;

// The keyboard interface, a USB function that adds a HID keyboard in the
// modes that take USB functions, except Keyboard mode. It is set by
// GamepadOptions.keyboardInterfaceEnabled and keyboardInterfaceXInputMode.
// Buttons in the active profile's keyboardButtonMask send their keys and are
// hidden from the input driver. Encoders in Volume mode send volume keys.
class KeyboardInterface : public GPUSBFunction {
public:
	KeyboardInterface(KeyboardInterface const&) = delete;
	void operator=(KeyboardInterface const&) = delete;
	static KeyboardInterface& getInstance()
	{
		static KeyboardInterface instance;
		return instance;
	}

	// Called once from GP2040::setup(), after USBFunctionManager::setup()
	void setup();

	USBFunctionSetting setting(InputMode mode) override;
	const uint8_t* getReportDescriptor() override;
	uint16_t getReportDescriptorLength() override;
	bool hasOutEndpoint() override { return false; }
	bool hasInputStep() override { return true; }
	uint16_t getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) override;
	void setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t* buffer, uint16_t bufsize) override {}
	void usbEvent(uint32_t eventId) override;
	void usbUnmounted() override;
	void usbSuspended() override;

	// Takes masked buttons out of the gamepad state
	void processInputs(Gamepad* gamepad) override;

	// Sends a report when the keys change
	void process() override;

	// Whether the button of this KeyboardMapping field sends its key in place
	// of its controller input. False while the interface is not active.
	bool sendsKey(uint32_t KeyboardMapping::* code) const;

private:
	// Every member has a constant initialiser, so the object is built at
	// compile time and getInstance() needs no first-use guard
	KeyboardInterface() = default;

	void loadProfile();
	void buildReport();
	void handleMappingChange(GPEvent* e);
	void handleEncoder(GPEvent* e);

	// Active profile's keyboard mapping, and its masked buttons that have a key
	const KeyboardMapping* mapping = nullptr;
	uint32_t buttonMask = 0;

	// Masked buttons currently pressed
	uint32_t keys = 0;
	bool keysChanged = false;
	volatile bool busReset = false; // set in interrupt context

	uint8_t volumeEncoders = 0; // one bit per encoder in Volume mode
	int8_t volumeSteps = 0;     // volume steps not yet sent
	int8_t volume = 0;          // volume key in the report: 1 up, -1 down

	// Current report and the last one sent
	KeyboardReport report = {};
	bool reportPending = false;
	uint8_t sentKeycode[sizeof(KeyboardReport::keycode)] = {};
	uint8_t sentMultimedia = 0;
};

#endif
