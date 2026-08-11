/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _HOST_LIGHTING_H_
#define _HOST_LIGHTING_H_

#include <stdint.h>

#include "tusb.h"
#include "enums.pb.h"

// Host Lighting Protocol (HLP): lets a host application drive the board's RGB
// LEDs over a dedicated vendor HID interface, alongside the regular input
// interface, in XInput, Generic and Keyboard modes (runtime-gated by
// HostLightingOptions in the stored config).
//
// Wire protocol, in fixed 64-byte reports both directions:
//   request:  [0]=command  [1]=sequence  [2..]=payload
//   reply:    [0]=command|0x80  [1]=sequence  [2]=status  [3..]=payload
// Staging commands (SET_BUTTONS/SET_RANGE/SET_RANGE_RGBW/FILL/CLEAR) edit an
// off-screen frame that COMMIT publishes atomically to the render loop;
// RELEASE - or the host going quiet past the keepalive timeout - hands the
// LEDs back to the on-board animations. GET_CAPS serves five forward-only
// pages (identity, runtime state, LED map, animations, positions) built from
// the board's live configuration, so hosts need no per-board knowledge.
// Management commands select the on-board idle animation (SET_ANIMATION) and,
// magic-guarded, switch input mode or reboot into webconfig or the bootloader.
//
// Config default for enabling the add-on, overridable per board. Disabled by
// default like every other addon; while disabled all modes present
// byte-identical descriptors to stock firmware.
#ifndef HOST_LIGHTING_ENABLED
#define HOST_LIGHTING_ENABLED 0
#endif

// Config default for the XInput lighting mode (AUTO when 1, OFF when 0).
// XInput support carries extra weight: the device re-identifies as a composite
// (custom VID:PID + MS OS descriptors bind XUSB via compatible ID) and drops
// the console-only interfaces, so it is PC-only while the composite is active.
#ifndef HOST_LIGHTING_XINPUT
#define HOST_LIGHTING_XINPUT 1
#endif

#define HOST_LIGHTING_PROTOCOL_VERSION_MAJOR 1
#define HOST_LIGHTING_PROTOCOL_VERSION_MINOR 0

// All transfers are fixed-size reports: [0]=command, [1]=sequence, [2..63]=payload.
// Replies echo the sequence and set bit 7 of the command byte.
#define HOST_LIGHTING_REPORT_SIZE     64
#define HOST_LIGHTING_RESPONSE_FLAG   0x80

// Command IDs are grouped by function with room to grow; new commands are
// assigned within the range matching their purpose. 0x00 is never a command
// and bit 7 is the reply flag, so the command space is 0x01-0x7F.
// 0x01-0x0F: session and discovery
#define HOST_LIGHTING_CMD_PING           0x01
#define HOST_LIGHTING_CMD_GET_CAPS       0x02
#define HOST_LIGHTING_CMD_SET_MODE       0x03
// 0x10-0x2F: frame staging
#define HOST_LIGHTING_CMD_SET_BUTTONS    0x10
#define HOST_LIGHTING_CMD_SET_RANGE      0x11
#define HOST_LIGHTING_CMD_SET_RANGE_RGBW 0x12
#define HOST_LIGHTING_CMD_FILL           0x13
#define HOST_LIGHTING_CMD_CLEAR          0x14
// 0x30-0x3F: frame lifecycle
#define HOST_LIGHTING_CMD_COMMIT         0x30
#define HOST_LIGHTING_CMD_RELEASE        0x31
// 0x40-0x4F: board features
#define HOST_LIGHTING_CMD_SET_ANIMATION  0x40
// 0x70-0x7F: privileged management, magic-guarded
#define HOST_LIGHTING_CMD_SET_INPUT_MODE   0x7B
#define HOST_LIGHTING_CMD_REBOOT_WEBCONFIG 0x7C
#define HOST_LIGHTING_CMD_REBOOT_BOOTSEL   0x7F

#define HOST_LIGHTING_STATUS_OK           0x00
#define HOST_LIGHTING_STATUS_UNSUPPORTED  0x01
#define HOST_LIGHTING_STATUS_INVALID_ARG  0x02

// Addressable LED space and per-report payload capacities
#define HOST_LIGHTING_MAX_LEDS              100
#define HOST_LIGHTING_RANGE_MAX_PIXELS       20
#define HOST_LIGHTING_RANGE_RGBW_MAX_PIXELS  15
#define HOST_LIGHTING_BUTTONS_MAX_ENTRIES    15
#define HOST_LIGHTING_POSITIONS_PER_PAGE     19
#define HOST_LIGHTING_DEFAULT_TIMEOUT_MS   2000

#define HOST_LIGHTING_FILL_SCOPE_ALL     0x00
#define HOST_LIGHTING_FILL_SCOPE_BUTTONS 0x01
#define HOST_LIGHTING_FILL_SCOPE_CASE    0x02
#define HOST_LIGHTING_FILL_SCOPE_PLEDS   0x03

#define HOST_LIGHTING_TAKEOVER_WHOLE_FRAME 0x00
#define HOST_LIGHTING_TAKEOVER_OVERLAY     0x01

// Button IDs: 0-17 in GP2040-CE canonical order, then addressable specials
#define HOST_LIGHTING_BUTTON_COUNT    18
#define HOST_LIGHTING_BUTTON_PLED1    24
#define HOST_LIGHTING_BUTTON_PLED4    27
#define HOST_LIGHTING_BUTTON_TURBO    28
#define HOST_LIGHTING_BUTTON_CASE     29

// The HID instance index the lighting interface occupies in HID-class modes
// (the gamepad interface enumerates first as instance 0). In XInput mode the
// gamepad interface is vendor-class, so lighting is the only HID instance (0);
// use HostLighting::lightingInstance() for the active mode's value.
#define HOST_LIGHTING_HID_INSTANCE    1

// Vendor-defined report descriptor: one 64-byte input and one 64-byte output
// report, no report IDs. Hosts identify the interface by this usage page.
static const uint8_t hostlighting_report_descriptor[] __attribute__((unused)) =
{
	0x06, 0x47, 0xFF,  // USAGE_PAGE (Vendor Defined 0xFF47)
	0x09, 0x4C,        // USAGE (0x4C)
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

namespace HostLighting {
	// Modes whose configuration descriptor includes the lighting interface
	bool enabledForMode(InputMode mode);

	// Whether the given HID instance is the lighting interface in the active mode
	bool isLightingInterface(uint8_t itf);

	// The HID instance index the lighting interface occupies in the active mode
	uint8_t lightingInstance();

	// Whether XInput mode should present the lighting composite identity this
	// boot: config ON, or AUTO with a PC-host verdict recorded by auto-detect
	bool xinputCompositeActive();

	// XInput AUTO detection tick, called from the XInput driver's process loop.
	// A host that enumerates the (console-identical) stock identity but sends no
	// console auth traffic within the detection window is a PC: record the
	// verdict in a watchdog scratch register and reboot into the composite.
	// The verdict survives soft reboots and clears on power loss (unplug).
	void xinputAutoDetectTask(bool consoleAuthSeen);

	const uint8_t * getReportDescriptor();

	uint16_t getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t * buffer, uint16_t reqlen);
	void setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t * buffer, uint16_t bufsize);

	// Called from the LED render loop (core1) just before the frame is shown.
	// While the host holds a fresh takeover, replaces frame[0..ledCount) with the
	// host frame, converted to the strip's colour format. `format` is the LEDFormat
	// enum value; `brightnessX` is the board's current brightness scale.
	void applyToFrame(uint32_t * frame, uint32_t ledCount, float brightnessX, int format);

	// Consumes a pending SET_ANIMATION request; returns the requested on-board
	// animation index, or -1 when none is pending. Called from the LED render
	// loop (core1), which owns animation selection.
	int16_t takeLocalAnimationRequest();
}

#endif
