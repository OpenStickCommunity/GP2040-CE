/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _HOST_LIGHTING_H_
#define _HOST_LIGHTING_H_

#include <stdint.h>

#include "tusb.h"
#include "enums.pb.h"

// Host Lighting Protocol (HLP): a vendor HID interface through which a host
// drives the board's RGB LEDs. Exposed beside the input interface in XInput,
// Generic, Keyboard and SInput modes when HostLightingOptions enables it.
//
// Wire format, fixed 64-byte reports in both directions:
//   request:  [0]=command  [1]=sequence  [2..]=payload
//   reply:    [0]=command|0x80  [1]=sequence  [2]=status  [3..]=payload
// Staging commands edit an off-screen frame, COMMIT publishes it atomically to
// the render loop, and RELEASE or keepalive expiry returns the LEDs to the
// on-board animations. GET_CAPS serves capability pages built from the live
// configuration. Full reference: docs/host-lighting.md.

// Default for HostLightingOptions.enabled; overridable per board config.
// While disabled, every mode presents its stock USB descriptors.
#ifndef HOST_LIGHTING_ENABLED
#define HOST_LIGHTING_ENABLED 0
#endif

// Default XInput lighting mode: 1 = AUTO, 0 = OFF. In XInput the interface
// needs the composite identity (own VID:PID, MS OS descriptors), which omits
// the console-only interfaces and is therefore PC-only.
#ifndef HOST_LIGHTING_XINPUT
#define HOST_LIGHTING_XINPUT 1
#endif

// Protocol version reported by PING. Minor versions are additive; hosts detect
// a feature by minor >= n. Changelog: docs/host-lighting.md.
//   1.1  page 5 (light table); page 1 fields [12..18]; FILL buttons scope
//        covers every button light; SET_MODE timeout ceiling
//   1.2  SET_LIGHT; SET_BUTTONS stages the extended button IDs
//   1.3  outcome mask in SET_LIGHT replies [5..6]; SET_LIGHT_RGBW
//   1.4  page 6 (control table); page 1 feature bit 2
// The caps format byte on page 0 versions that page's layout only; it stays 2.
#define HOST_LIGHTING_PROTOCOL_VERSION_MAJOR 1
#define HOST_LIGHTING_PROTOCOL_VERSION_MINOR 4

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
#define HOST_LIGHTING_CMD_SET_LIGHT      0x15
#define HOST_LIGHTING_CMD_SET_LIGHT_RGBW 0x16
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
#define HOST_LIGHTING_SET_LIGHT_MAX_ENTRIES  15
#define HOST_LIGHTING_SET_LIGHT_RGBW_MAX_ENTRIES 12
#define HOST_LIGHTING_POSITIONS_PER_PAGE     19
#define HOST_LIGHTING_DEFAULT_TIMEOUT_MS   2000
// Ceiling for the SET_MODE keepalive timeout. A host that idles longer sends
// PING, which refreshes the keepalive.
#define HOST_LIGHTING_MAX_TIMEOUT_MS      10000

#define HOST_LIGHTING_FILL_SCOPE_ALL     0x00
#define HOST_LIGHTING_FILL_SCOPE_BUTTONS 0x01
#define HOST_LIGHTING_FILL_SCOPE_CASE    0x02
#define HOST_LIGHTING_FILL_SCOPE_PLEDS   0x03

#define HOST_LIGHTING_TAKEOVER_WHOLE_FRAME 0x00
#define HOST_LIGHTING_TAKEOVER_OVERLAY     0x01

// Button IDs follow the gamepad mask bit order (GamepadState.h): dpad, then B1
// upward. GpioAction declares its tail in a different order (A1, A2, L3, R3),
// so the mapping is a table, never arithmetic.
//
// 0-17 are the canonical set page 2 reports. 18-19 (A3, A4) and 30-41 (E1-E12)
// have no page 2 slot and are reported by the light table. 20-23 are
// permanently unassigned: those mask bits (GAMEPAD_MASK_DU..DR) are the dpad
// in a second encoding, not further controls.
#define HOST_LIGHTING_BUTTON_COUNT    18
#define HOST_LIGHTING_BUTTON_A3       18
#define HOST_LIGHTING_BUTTON_A4       19
#define HOST_LIGHTING_BUTTON_PLED1    24
#define HOST_LIGHTING_BUTTON_PLED4    27
#define HOST_LIGHTING_BUTTON_TURBO    28
#define HOST_LIGHTING_BUTTON_CASE     29
#define HOST_LIGHTING_BUTTON_E1       30
#define HOST_LIGHTING_BUTTON_E12      41
#define HOST_LIGHTING_BUTTON_NONE     0xFF

// Page 5 - light table. Fixed 12-byte records; a field the board cannot supply
// carries a sentinel, and the per-record flags say which fields are real.
#define HOST_LIGHTING_LIGHT_STRIDE       12
#define HOST_LIGHTING_LIGHTS_PER_PAGE     4

// Light kinds on the wire. Numerically equal to LightType today, but defined
// here so the wire does not depend on an internal enum.
#define HOST_LIGHTING_LIGHT_ACTION    0x00
#define HOST_LIGHTING_LIGHT_CASE      0x01
#define HOST_LIGHTING_LIGHT_TURBO     0x02
#define HOST_LIGHTING_LIGHT_PLAYER1   0x03
#define HOST_LIGHTING_LIGHT_PLAYER4   0x06
#define HOST_LIGHTING_LIGHT_UNKNOWN   0xFF

// Per-record flags. A set bit asserts the field is real; a clear bit asserts
// nothing. POSITION: the grid coordinates are valid (not inferable from
// (0,0), which is a legal position). PER_LIGHT: the record describes exactly
// one light.
#define HOST_LIGHTING_LIGHT_FLAG_POSITION     0x01
#define HOST_LIGHTING_LIGHT_FLAG_PER_LIGHT    0x02

// Page 6 - control table. One 6-byte record per GPIO pin that carries an
// action, keyed by pin (unique, where a button ID is not). Joins to page 5 on
// the pin column. Eight records per reply.
#define HOST_LIGHTING_CONTROL_STRIDE      6
#define HOST_LIGHTING_CONTROLS_PER_PAGE   8
// Per-record flag: a light is bound to this control
#define HOST_LIGHTING_CONTROL_FLAG_LIT    0x01
// Page header flag: lit bits are resolved per light, so two pins sharing an
// action can differ. Always set.
#define HOST_LIGHTING_CONTROLS_FLAG_PER_LIGHT 0x01

// GpioAction is sent verbatim as int16. This sentinel means no owning action,
// which is distinct from GpioAction::NONE (-10).
#define HOST_LIGHTING_ACTION_NONE     ((int16_t)0x8000)

// Page 1 feature bits. A clear bit guarantees the page returns no records.
// Bits 0-1 follow the light registry, which is populated during LED setup;
// bit 2 is set from boot because the control table is built from the pin map.
#define HOST_LIGHTING_FEATURE_POSITIONS      (1u << 0)
#define HOST_LIGHTING_FEATURE_LIGHT_TABLE    (1u << 1)
#define HOST_LIGHTING_FEATURE_CONTROL_TABLE  (1u << 2)

// Page 1 LED framework byte. Diagnostic only: hosts branch on the feature bits
// and record flags. The firmware reports REFACTOR. CLASSIC was reported by
// pre-release fork builds and stays defined because the value is frozen on the
// wire. 0 = not reported.
#define HOST_LIGHTING_FRAMEWORK_UNREPORTED 0x00
#define HOST_LIGHTING_FRAMEWORK_CLASSIC    0x01
#define HOST_LIGHTING_FRAMEWORK_REFACTOR   0x02

// Page 1 animation namespace: what a page 3 / SET_ANIMATION index selects.
// The firmware reports PROFILES (the stored animation profiles). EFFECTS was
// reported by pre-release fork builds and stays defined because the value is
// frozen on the wire. 0 = not reported.
#define HOST_LIGHTING_ANIM_EFFECTS    0x01
#define HOST_LIGHTING_ANIM_PROFILES   0x02

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

struct Lights;

namespace HostLighting {
	// Modes whose configuration descriptor includes the lighting interface
	bool enabledForMode(InputMode mode);

	// Whether the given HID instance is the lighting interface in the active mode
	bool isLightingInterface(uint8_t itf);

	// The HID instance index the lighting interface occupies in the active mode
	uint8_t lightingInstance();

	// Publishes the LED layout for GET_CAPS; called by the LED addon whenever
	// its lights are (re)configured
	void registerLights(const ::Lights & lights);

	// Whether XInput mode should present the lighting composite identity this
	// boot: config ON, or AUTO with a PC-host verdict recorded by auto-detect
	bool xinputCompositeActive();

	// XInput AUTO detection, called from the XInput driver's process loop. If the
	// host sends no console auth within the detection window after enumeration,
	// records a PC verdict in a watchdog scratch register and reboots into the
	// composite. The verdict survives soft reboots and clears on power loss.
	void xinputAutoDetectTask(bool consoleAuthSeen);

	// Publishes the LED render rate in Hz (GET_CAPS page 1 [18]); called by the
	// LED addon, which owns the interval.
	void setRenderRate(uint8_t hz);

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

	// Returns the LEDs to the on-board animations, as RELEASE does. Called on USB
	// unmount and suspend so a takeover does not outlive its session.
	void releaseTakeover();
}

#endif
