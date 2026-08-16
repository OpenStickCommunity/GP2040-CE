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
// LEDs back to the on-board animations. GET_CAPS serves six forward-only
// pages (identity, runtime state, LED map, animations, positions, light table)
// built from the board's live configuration, so hosts need no per-board
// knowledge. Page 2 answers "where do I write this control" for the eighteen
// canonical controls; page 5 is the full inventory, and where a control owns
// several lights it is page 5 that says so.
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

// v1.1 adds capability page 5, appends fields to page 1, and widens FILL's
// buttons scope to every button light. No command ID and no existing payload
// layout changes, so every field a v1.0 host reads is still where it was.
//
// What a v1.0 host can observe did change in places, and the spec lists every
// case rather than claiming otherwise: page 2 [6] reports the extent of the
// mapped range instead of the sum of the ranges above it, SET_MODE's timeout
// gained a ceiling, FILL's buttons scope widened, and the map fingerprint now
// also moves on profile pin remaps. All are the board describing itself more
// accurately at the same offsets, which is what keeps this a minor bump; the
// changelog in docs/host-lighting.md is the authority.
//
// The caps format byte on page 0 stays at 2: it versions that page's layout,
// which is untouched, and moving it would force every host to re-derive a
// parser for a surface that did not move.
//
// v1.2 adds SET_LIGHT, staging one light by its page 5 ordinal, and lets
// SET_BUTTONS stage the extended IDs on boards that have such lights. Both are
// additive - a new command in a reserved slot and a skip that becomes an apply
// where a light exists - and a host detects them by minor version >= 2.
//
// v1.3 defines reply bytes [5..6] of the per-light staging commands as a
// per-entry outcome mask (bit n set = entry n applied), so a skip names the
// stale entry instead of forcing a page 5 re-walk, and adds SET_LIGHT_RGBW,
// the RGBW pairing SET_RANGE already has. Both are additive - the mask
// occupies bytes v1.2 zero-filled and the command sits in a reserved slot -
// and a host detects them by minor version >= 3.
#define HOST_LIGHTING_PROTOCOL_VERSION_MAJOR 1
#define HOST_LIGHTING_PROTOCOL_VERSION_MINOR 3

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
// Upper bound on the keepalive a host may ask for. The field is sixteen bits,
// so without one a dead host could hold the lights for over a minute. Not
// configurable: a host that wants to idle longer sends PING, which refreshes
// the keepalive and is how the protocol expects a quiet host to behave.
#define HOST_LIGHTING_MAX_TIMEOUT_MS      10000

#define HOST_LIGHTING_FILL_SCOPE_ALL     0x00
#define HOST_LIGHTING_FILL_SCOPE_BUTTONS 0x01
#define HOST_LIGHTING_FILL_SCOPE_CASE    0x02
#define HOST_LIGHTING_FILL_SCOPE_PLEDS   0x03

#define HOST_LIGHTING_TAKEOVER_WHOLE_FRAME 0x00
#define HOST_LIGHTING_TAKEOVER_OVERLAY     0x01

// Button IDs follow GP2040-CE's gamepad bit order - the four dpad masks, then
// the button masks from B1 upward (GamepadState.h). That is deliberately not
// GpioAction's declaration order, whose tail runs A1, A2, L3, R3; the bridge
// between the two is a table, never arithmetic.
//
// 0-17 are the canonical set page 2 can express and are frozen from v1.0.
// 18-19 name A3 and A4, and 30-41 name E1-E12, both added in v1.1: page 2 has
// no slot for them, so they appear only in the light table, and a host learns
// them from there. 20-23 stay permanently unassigned - those mask bits are
// GAMEPAD_MASK_DU..DR, which are the dpad in a second encoding rather than
// four more controls, and giving Up a second ID would be a lasting mistake.
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

// Page 5 - the light table. One record per light, the same twelve bytes on
// every pipeline: a board fills in what it knows and writes the sentinels
// where it does not, so a host parses one way and reads flags to learn what
// is real. Availability is a property of the board, never of the version.
#define HOST_LIGHTING_LIGHT_STRIDE       12
#define HOST_LIGHTING_LIGHTS_PER_PAGE     4

// Light kinds as HLP defines them. Numerically equal to the LED-refactor's
// LightType today, but owned here: that enum exists in only one tree and is
// itself hand-mirrored from pixel.h, so the wire cannot depend on it.
#define HOST_LIGHTING_LIGHT_ACTION    0x00
#define HOST_LIGHTING_LIGHT_CASE      0x01
#define HOST_LIGHTING_LIGHT_TURBO     0x02
#define HOST_LIGHTING_LIGHT_PLAYER1   0x03
#define HOST_LIGHTING_LIGHT_PLAYER4   0x06
#define HOST_LIGHTING_LIGHT_UNKNOWN   0xFF

// Per-record flags, both positive assertions: a set bit is the board vouching
// for something, so a record asserting nothing reads as the weaker case.
// POSITION says the grid coordinates are real - it cannot be inferred from
// (0,0), because positions are origin-normalised so some light always sits
// there. PER_LIGHT says the record was read from a per-light table and
// describes exactly one light; clear means it was rebuilt from per-control
// configuration, where duplicates are structurally invisible.
#define HOST_LIGHTING_LIGHT_FLAG_POSITION     0x01
#define HOST_LIGHTING_LIGHT_FLAG_PER_LIGHT    0x02

// GpioAction travels verbatim as a signed 16-bit value so it never needs an
// HLP allocation when GP2040-CE adds one. This sentinel means no owning action
// exists, which is distinct from the pin's action genuinely being NONE (-10).
#define HOST_LIGHTING_ACTION_NONE     ((int16_t)0x8000)

// Page 1 feature bits. A cleared bit is a promise that the corresponding page
// returns nothing, not merely that it might.
#define HOST_LIGHTING_FEATURE_POSITIONS    (1u << 0)
#define HOST_LIGHTING_FEATURE_LIGHT_TABLE  (1u << 1)

// Page 1 LED-framework byte. CLASSIC is the AnimationStation the released
// firmware ships; REFACTOR is the Lights/Light rewrite on the LED-refactor
// branch. Zero is reserved for "not reported", so a board that never wrote the
// byte cannot be read as naming a framework. Diagnostic only: hosts must branch
// on the feature bits and the per-record flags, never on this, because the two
// differ in more ways than one byte can carry.
#define HOST_LIGHTING_FRAMEWORK_UNREPORTED 0x00
#define HOST_LIGHTING_FRAMEWORK_CLASSIC    0x01
#define HOST_LIGHTING_FRAMEWORK_REFACTOR   0x02

// Which universe page 3's animation index selects. Classic indexes its
// built-in effects; the refactor indexes the user's stored animation profiles.
// Those are genuinely different objects, so a host that assumes one silently
// selects the wrong thing on a board running the other. Naming the universe
// costs a byte and means neither has to pretend to be the other.
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

	// Publishes the LED render rate for GET_CAPS; called by the LED addon, which
	// owns the interval. Reported rather than asserted by the specification,
	// because the frameworks tick at different rates - classic at 100 Hz, the
	// refactor at 40 Hz, both derived from the LED addon's intervalMS - and a
	// host that streams faster than the board renders simply discards the
	// difference.
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

	// Hands the LEDs back to the on-board animations, as RELEASE does. Called
	// when the bus goes away: takeover is session state, and without this it
	// outlives the session that set it, so the next host to send anything at
	// all revives the previous host's frame before saying what it wants.
	void releaseTakeover();
}

#endif
