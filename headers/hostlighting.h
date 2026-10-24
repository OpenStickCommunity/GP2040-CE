/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _HOST_LIGHTING_H_
#define _HOST_LIGHTING_H_

#include <stdint.h>

#include "tusb.h"
#include "enums.pb.h"

// Host Lighting Protocol (HLP) 2.0, a vendor HID interface through which a
// host drives the board's RGB LEDs and reads its layout and state. Exposed
// beside the input interface in XInput, Generic, Keyboard and SInput modes
// when HostLightingOptions enables it. See docs/host-lighting.md.

// Default for HostLightingOptions.enabled; overridable per board config.
// While disabled, every mode presents its stock USB descriptors.
#ifndef HOST_LIGHTING_ENABLED
#define HOST_LIGHTING_ENABLED 0
#endif

// Default XInput lighting mode, 1 for AUTO and 0 for OFF (Always On is chosen
// in the web configurator). In XInput the interface needs the composite
// identity (own VID:PID, MS OS descriptors), which omits the console-only
// interfaces and is therefore PC-only.
#ifndef HOST_LIGHTING_XINPUT
#define HOST_LIGHTING_XINPUT 1
#endif

// HID instance of the lighting interface in HID-class modes, where the
// gamepad interface enumerates first as instance 0. In XInput the gamepad
// is vendor-class and lighting is the only HID instance (0).
#define HOST_LIGHTING_HID_INSTANCE 1

// Report descriptor with one 64-byte input and one 64-byte output report and
// no report IDs. Hosts find the interface by usage page 0xFF47; the usage is
// the protocol major version.
static const uint8_t hostlighting_report_descriptor[] __attribute__((unused)) =
{
	0x06, 0x47, 0xFF,  // USAGE_PAGE (Vendor Defined 0xFF47)
	0x09, 0x02,        // USAGE (0x02, protocol major 2)
	0xA1, 0x01,        // COLLECTION (Application)
	0x15, 0x00,        //   LOGICAL_MINIMUM (0)
	0x26, 0xFF, 0x00,  //   LOGICAL_MAXIMUM (255)
	0x75, 0x08,        //   REPORT_SIZE (8)
	0x95, 0x40,        //   REPORT_COUNT (64)
	0x09, 0x01,        //   USAGE (0x01)
	0x81, 0x02,        //   INPUT (Data,Var,Abs)
	0x09, 0x02,        //   USAGE (0x02)
	0x91, 0x02,        //   OUTPUT (Data,Var,Abs)
	0xC0               // END_COLLECTION
};

class AnimationStation;
struct Lights;

namespace HostLighting {
	// Set once in setup() and fixed for the boot. Callers in RAM read them
	// inline, so a board without the interface, or without XInput AUTO,
	// runs none of the per-pass hooks below from flash.
	extern bool interfaceActive;   // the lighting interface is presented
	extern bool xinputAutoDetect;  // XInput with AUTO lighting

	// Core 0, from GP2040::setup() after the add-ons load and before USB
	// starts. Counts the add-on inputs and sets the flags above.
	// expanderInputs is true when the PCF8575 add-on is loaded, so page 10
	// lists its input pins.
	void setup(bool expanderInputs);

	// Modes whose configuration descriptor includes the lighting interface
	bool enabledForMode(InputMode mode);

	// Whether HID instance itf is the lighting interface this boot
	bool isLightingInterface(uint8_t itf);

	// Whether XInput presents the lighting composite identity this boot, with
	// the config ON, or AUTO with a PC verdict recorded by auto-detect
	bool xinputCompositeActive();

	// XInput AUTO, from the XInput driver's process loop. Records a PC
	// verdict and reboots into the composite when a host enumerates without
	// console authentication; reboots back to the stock identity after a
	// bus reset that follows a suspend of the composite.
	void xinputAutoDetectTask(bool consoleAuthSeen);

	// LED add-on (core 1). Publishes the light layout and the render rate.
	// The layout must not change while the lighting interface exists.
	void registerLights(const ::Lights & lights, uint16_t renderRateHz);

	// LED add-on (core 1), each render tick before the frame is shown.
	// Applies host animation and brightness requests, then a live host frame.
	void render(AnimationStation & station, uint32_t * frame, int format);

	const uint8_t * getReportDescriptor();
	uint16_t getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t * buffer, uint16_t reqlen);
	void setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t * buffer, uint16_t bufsize);

	// USB session hooks (core 0, except usbEvent)
	void reportComplete(uint8_t itf);
	void usbEvent(uint32_t eventId); // Sets flags only, in interrupt context
	void usbUnmounted();
	void usbSuspended();

	// Core 0 main loop, after tud_task(), for expiry, events and the reply drain
	void task();
}

#endif
