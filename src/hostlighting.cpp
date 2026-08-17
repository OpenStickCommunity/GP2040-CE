/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#include "hostlighting.h"

#include <string.h>

#include "hardware/watchdog.h"
#include "pico/platform.h"
#include "pico/time.h"
#include "pico/unique_id.h"

#include "BoardConfig.h"
#include "drivermanager.h"
#include "eventmanager.h"
#include "events/GPStorageSaveEvent.h"
#include "storagemanager.h"
#include "system.h"
#include "usbdriver.h"
#include "version.h"
#include "animation.h"
#include "animationstation.h"

// Watchdog scratch register holding the XInput AUTO host verdict (scratch[5]
// belongs to System::reboot's BootMode)
#define HOST_LIGHTING_VERDICT_SCRATCH   6
#define HOST_LIGHTING_VERDICT_PC_HOST   0x484C5043 // "HLPC"
#define HOST_LIGHTING_XINPUT_DETECT_MS  4000

#ifndef BOARD_CONFIG_LABEL
#define BOARD_CONFIG_LABEL "Unknown"
#endif

#define HOST_LIGHTING_CAPS_FORMAT 2

static uint8_t responseBuffer[HOST_LIGHTING_REPORT_SIZE] = {};

// Staged frame + per-pixel validity: written by SET_* on core0, published by COMMIT.
static uint32_t stagingPixels[HOST_LIGHTING_MAX_LEDS] = {};
static uint32_t stagingValid[(HOST_LIGHTING_MAX_LEDS + 31) / 32] = {};

// Live frame shared with the core1 render loop, guarded by a sequence lock:
// core0 increments liveSeq to odd, writes, increments to even; core1 retries
// its copy until it reads the same even value on both sides.
static volatile uint32_t liveSeq = 0;
static uint32_t livePixels[HOST_LIGHTING_MAX_LEDS] = {};
static uint32_t liveValid[(HOST_LIGHTING_MAX_LEDS + 31) / 32] = {};
static volatile bool liveActive = false;
static volatile uint8_t liveTakeover = HOST_LIGHTING_TAKEOVER_WHOLE_FRAME;
static volatile uint16_t liveTimeoutMs = HOST_LIGHTING_DEFAULT_TIMEOUT_MS;
static volatile bool liveApplyBrightness = true;
static volatile uint32_t lastCommandMs = 0;

// Pending on-board animation selection, applied by the render core
static volatile int16_t pendingAnimationMode = -1;

struct LedRange {
	int16_t first;
	uint8_t count;
};

// Per-button LED index fields of LEDOptions, in protocol button ID order
static const int32_t LEDOptions::* const buttonIndexFields[HOST_LIGHTING_BUTTON_COUNT] = {
	&LEDOptions::indexUp, &LEDOptions::indexDown, &LEDOptions::indexLeft, &LEDOptions::indexRight,
	&LEDOptions::indexB1, &LEDOptions::indexB2, &LEDOptions::indexB3, &LEDOptions::indexB4,
	&LEDOptions::indexL1, &LEDOptions::indexR1, &LEDOptions::indexL2, &LEDOptions::indexR2,
	&LEDOptions::indexS1, &LEDOptions::indexS2, &LEDOptions::indexL3, &LEDOptions::indexR3,
	&LEDOptions::indexA1, &LEDOptions::indexA2,
};

// The GPIO action each protocol button ID stands for. Referenced by name so a
// renumbering of the enum cannot silently change what the wire means; the tail
// is a permutation, since protocol order is the gamepad bit order while
// GpioAction declares A1 and A2 ahead of L3 and R3.
static const GpioAction canonicalAction[HOST_LIGHTING_BUTTON_COUNT] = {
	GpioAction::BUTTON_PRESS_UP, GpioAction::BUTTON_PRESS_DOWN,
	GpioAction::BUTTON_PRESS_LEFT, GpioAction::BUTTON_PRESS_RIGHT,
	GpioAction::BUTTON_PRESS_B1, GpioAction::BUTTON_PRESS_B2,
	GpioAction::BUTTON_PRESS_B3, GpioAction::BUTTON_PRESS_B4,
	GpioAction::BUTTON_PRESS_L1, GpioAction::BUTTON_PRESS_R1,
	GpioAction::BUTTON_PRESS_L2, GpioAction::BUTTON_PRESS_R2,
	GpioAction::BUTTON_PRESS_S1, GpioAction::BUTTON_PRESS_S2,
	GpioAction::BUTTON_PRESS_L3, GpioAction::BUTTON_PRESS_R3,
	GpioAction::BUTTON_PRESS_A1, GpioAction::BUTTON_PRESS_A2,
};

// The GPIO a control sits on, for the light table's diagnostic column. A unique
// match or nothing: a board that maps one action across several pins has no
// single answer, and here an honest "unknown" beats picking the first.
static uint8_t pinForAction(GpioAction action) {
	GpioMappingInfo * pinMappings = Storage::getInstance().getProfilePinMappings();
	uint8_t found = 0xFF;

	for (uint8_t pin = 0; pin < (uint8_t)NUM_BANK0_GPIOS; pin++) {
		if (pinMappings[pin].action != action)
			continue;
		if (found != 0xFF)
			return 0xFF;
		found = pin;
	}
	return found;
}

// Resolves a protocol button ID to its physical LED chain range from the live
// configuration; {-1, 0} when the control has no LED on this board.
static LedRange resolveButton(uint8_t buttonId) {
	const LEDOptions & lo = Storage::getInstance().getLedOptions();
	LedRange none = { -1, 0 };

	if (buttonId < HOST_LIGHTING_BUTTON_COUNT) {
		int32_t index = lo.*buttonIndexFields[buttonId];
		uint32_t perButton = (lo.ledsPerButton > 0) ? lo.ledsPerButton : 1;
		if ((index < 0) || ((uint32_t)index * perButton >= HOST_LIGHTING_MAX_LEDS))
			return none;
		return { (int16_t)(index * perButton), (uint8_t)perButton };
	}

	if ((buttonId >= HOST_LIGHTING_BUTTON_PLED1) && (buttonId <= HOST_LIGHTING_BUTTON_PLED4)) {
		if (lo.pledType != PLED_TYPE_RGB)
			return none;
		const int32_t pledIndexes[4] = { lo.pledIndex1, lo.pledIndex2, lo.pledIndex3, lo.pledIndex4 };
		int32_t index = pledIndexes[buttonId - HOST_LIGHTING_BUTTON_PLED1];
		if ((index < 0) || (index >= HOST_LIGHTING_MAX_LEDS))
			return none;
		return { (int16_t)index, 1 };
	}

	if (buttonId == HOST_LIGHTING_BUTTON_TURBO) {
		const TurboOptions & turbo = Storage::getInstance().getAddonOptions().turboOptions;
		if ((turbo.turboLedType != PLED_TYPE_RGB) || (turbo.turboLedIndex < 0) ||
				(turbo.turboLedIndex >= HOST_LIGHTING_MAX_LEDS))
			return none;
		return { (int16_t)turbo.turboLedIndex, 1 };
	}

	if (buttonId == HOST_LIGHTING_BUTTON_CASE) {
		// The index is rejected outright when it lands outside the addressable
		// space, as every branch above does. Clamping the count instead is not
		// enough: the subtraction below is signed, so an index past the end
		// yields a negative width that becomes a huge one as an unsigned count,
		// and an index of exactly the maximum yields a zero-width range at an
		// address no pixel occupies. Neither is a range, and both would be
		// published to a host through page 2 and the light table.
		if ((lo.caseRGBIndex < 0) || (lo.caseRGBIndex >= HOST_LIGHTING_MAX_LEDS) ||
				(lo.caseRGBCount == 0))
			return none;
		uint32_t count = lo.caseRGBCount;
		if ((uint32_t)lo.caseRGBIndex + count > HOST_LIGHTING_MAX_LEDS)
			count = HOST_LIGHTING_MAX_LEDS - lo.caseRGBIndex;
		return { (int16_t)lo.caseRGBIndex, (uint8_t)count };
	}

	return none;
}

static void stagePixel(uint32_t index, uint32_t colour) {
	stagingPixels[index] = colour;
	stagingValid[index / 32] |= (1u << (index % 32));
}

// Clamped against the staging buffer rather than trusting the caller: a stored
// configuration can name an LED index past the end of the chain, and the ranges
// resolveButton derives from one are only as sound as the values behind them.
// Bounding the write here keeps a bad config a lighting bug instead of a
// memory-corruption bug, for every present and future caller.
static void stageRange(const LedRange & range, uint32_t colour) {
	if (range.first < 0)
		return;

	uint32_t end = (uint32_t)range.first + range.count;
	if (end > HOST_LIGHTING_MAX_LEDS)
		end = HOST_LIGHTING_MAX_LEDS;

	for (uint32_t index = (uint32_t)range.first; index < end; index++)
		stagePixel(index, colour);
}

// GET_CAPS pages are ordered so hosts only ever read forward: page 0 identity
// (fetch once), page 1 runtime state (the cheap poll target), page 2 LED map
// (carries the same fingerprint as page 1, so a fetched map self-certifies
// against the state that prompted it), page 3 on-board animations, page 4
// per-light positions, page 5 the light table.

// FNV-1a over everything the capability pages say about the map, so one value
// certifies pages 2, 4 and 5 together and a host knows from a single poll that
// anything it cached is still true.
//
// The pin map is hashed wholesale rather than through per-control reverse
// lookups. It is both cheaper - one pass instead of one scan per control - and
// the honest scope: a profile switch remaps pins underneath the board without
// touching a single stored LED index, which changes what page 5 reports about
// ownership while every LED range stays exactly as it was.
static uint32_t ledMapFingerprint() {
	uint32_t fingerprint = 2166136261u;

	for (uint8_t id = 0; id <= HOST_LIGHTING_BUTTON_CASE; id++) {
		LedRange range = resolveButton(id);
		fingerprint = (fingerprint ^ (uint8_t)range.first) * 16777619u;
		fingerprint = (fingerprint ^ range.count) * 16777619u;
	}

	GpioMappingInfo * pinMappings = Storage::getInstance().getProfilePinMappings();
	for (uint8_t pin = 0; pin < (uint8_t)NUM_BANK0_GPIOS; pin++)
		fingerprint = (fingerprint ^ (uint32_t)pinMappings[pin].action) * 16777619u;

	// Board shape page 2 carries and the range loop above never reaches
	const LEDOptions & lo = Storage::getInstance().getLedOptions();
	fingerprint = (fingerprint ^ (uint32_t)lo.ledsPerButton) * 16777619u;
	fingerprint = (fingerprint ^ (uint32_t)lo.ledFormat) * 16777619u;
	fingerprint = (fingerprint ^ (uint32_t)lo.ledLayout) * 16777619u;
	fingerprint = (fingerprint ^ (uint32_t)lo.brightnessMaximum) * 16777619u;

	return fingerprint;
}

// Page 0 - identity, static for the life of the firmware:
// [3] caps format   [4..11] factory-unique board ID
// [12..] two NUL-terminated strings: board label, firmware version
static void buildCapsIdentity(uint8_t * reply) {
	reply[3] = HOST_LIGHTING_CAPS_FORMAT;

	pico_unique_board_id_t uniqueId;
	pico_get_unique_board_id(&uniqueId);
	memcpy(&reply[4], uniqueId.id, 8);

	const char * label = BOARD_CONFIG_LABEL;
	const char * version = GP2040VERSION;
	uint8_t pos = 12;
	while (*label && (pos < HOST_LIGHTING_REPORT_SIZE - 2))
		reply[pos++] = (uint8_t)*label++;
	reply[pos++] = 0;
	while (*version && (pos < HOST_LIGHTING_REPORT_SIZE - 1))
		reply[pos++] = (uint8_t)*version++;
	reply[pos] = 0;
}

// Published by the LED addon, which owns the render interval; zero until it
// has, which a host reads as "not stated" rather than as a rate of nothing
static uint8_t renderRateHz = 0;

void HostLighting::setRenderRate(uint8_t hz) {
	renderRateHz = hz;
}

void HostLighting::releaseTakeover() {
	liveActive = false;
}

// Page 1 - runtime state, everything that changes without replugging:
// [3] current InputMode   [4] profile number   [5] brightness step
// [6] host-assigned player (0 = none)   [7..10] LED-map fingerprint (LE)
// [11] current on-board animation index
// [12..15] feature bitmask (LE)   [16] LED framework   [17] animation namespace
// [18] render rate in Hz (0 = not stated)
// Everything from [12] was appended in v1.1. Replies are zero-filled before they
// are built, so firmware predating those fields reports them as zero, which
// reads correctly as no optional features and nothing stated.
static void buildCapsState(uint8_t * reply) {
	reply[3] = (uint8_t)DriverManager::getInstance().getInputMode();

	const GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
	reply[4] = (uint8_t)gamepadOptions.profileNumber;

	const AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
	reply[5] = (uint8_t)((animationOptions.brightness > 255) ? 255 : animationOptions.brightness);

	uint32_t player = Storage::getInstance().GetProcessedGamepad()->auxState.playerID.value;
	reply[6] = (uint8_t)((player > 255) ? 255 : player);

	uint32_t fingerprint = ledMapFingerprint();
	memcpy(&reply[7], &fingerprint, 4);

	reply[11] = (uint8_t)animationOptions.baseAnimationIndex;

	// This pipeline has no per-light positions, so bit 0 stays clear. Bit 1 is
	// set whenever page 5 has anything to say, which here means the board has at
	// least one control with a light.
	uint32_t features = 0;
	for (uint8_t id = 0; id <= HOST_LIGHTING_BUTTON_CASE; id++) {
		if (resolveButton(id).first >= 0) {
			features |= HOST_LIGHTING_FEATURE_LIGHT_TABLE;
			break;
		}
	}
	memcpy(&reply[12], &features, 4);

	reply[16] = HOST_LIGHTING_FRAMEWORK_CLASSIC;
	reply[17] = HOST_LIGHTING_ANIM_EFFECTS;
	reply[18] = renderRateHz;
}

// Tracks the highest LED index any reported range reaches, for page 2's [6].
// An extent rather than a sum of counts: a sum only equals the strip length
// when every LED belongs to a control this page names, and it undercounts by
// exactly the lights a host most needs to know about on a board that has more
// lights than named controls. An extent is also a number a host can size a
// frame buffer from, which a sum is not.
static void noteExtent(uint32_t & extent, const LedRange & range) {
	if (range.first < 0)
		return;

	uint32_t end = (uint32_t)range.first + range.count;
	if (end > extent)
		extent = end;
}

// Page 2 - LED map; the trailing fingerprint matches page 1's for the same
// map, letting a host confirm a coherent snapshot without re-reading page 1:
// [3] ledsPerButton   [4] LEDFormat   [5] ButtonLayout
// [6] LED count, as the extent of the mapped range   [7] brightness maximum
// [8..43]  per-button {first LED, count} pairs, button IDs 0-17 (0xFF = unmapped)
// [44..47] player LED indexes   [48] turbo LED index   [49..50] case {first, count}
// [51..54] LED-map fingerprint (LE)
static void buildCapsLedMap(uint8_t * reply) {
	const LEDOptions & lo = Storage::getInstance().getLedOptions();
	uint32_t perButton = (lo.ledsPerButton > 0) ? lo.ledsPerButton : 1;

	reply[3] = (uint8_t)perButton;
	reply[4] = (uint8_t)lo.ledFormat;
	reply[5] = (uint8_t)lo.ledLayout;
	reply[7] = (uint8_t)((lo.brightnessMaximum > 255) ? 255 : lo.brightnessMaximum);

	uint32_t ledExtent = 0;
	for (uint8_t b = 0; b < HOST_LIGHTING_BUTTON_COUNT; b++) {
		LedRange range = resolveButton(b);
		if (range.first < 0) {
			reply[8 + b * 2] = 0xFF;
			reply[9 + b * 2] = 0;
		} else {
			reply[8 + b * 2] = (uint8_t)range.first;
			reply[9 + b * 2] = range.count;
		}
		noteExtent(ledExtent, range);
	}

	for (uint8_t p = 0; p < 4; p++) {
		LedRange range = resolveButton(HOST_LIGHTING_BUTTON_PLED1 + p);
		reply[44 + p] = (range.first < 0) ? 0xFF : (uint8_t)range.first;
		noteExtent(ledExtent, range);
	}

	LedRange turbo = resolveButton(HOST_LIGHTING_BUTTON_TURBO);
	reply[48] = (turbo.first < 0) ? 0xFF : (uint8_t)turbo.first;
	noteExtent(ledExtent, turbo);

	LedRange caseRange = resolveButton(HOST_LIGHTING_BUTTON_CASE);
	reply[49] = (caseRange.first < 0) ? 0xFF : (uint8_t)caseRange.first;
	reply[50] = caseRange.count;
	noteExtent(ledExtent, caseRange);

	reply[6] = (uint8_t)((ledExtent > 255) ? 255 : ledExtent);

	uint32_t fingerprint = ledMapFingerprint();
	memcpy(&reply[51], &fingerprint, 4);
}

// How many on-board animations a host may actually select. AnimationStation
// counts the custom theme as an extra effect once the user has configured one
// (see its constructor in animationstation.cpp), and the board's own hotkeys
// will cycle onto it, so reporting the bare TOTAL_EFFECTS hides an animation
// the board really has and makes SET_ANIMATION refuse a valid index. Derived
// from stored config rather than read from AnimationStation because HLP holds
// no handle on the LED addon by design; keep this in step with that constructor.
static uint8_t onBoardAnimationCount() {
	const AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
	return (uint8_t)(TOTAL_EFFECTS + (animationOptions.hasCustomTheme ? 1 : 0));
}

// Page 3 - on-board animation selection (the lighting shown outside host
// control): [3] current animation index   [4] number of animations
static void buildCapsAnimations(uint8_t * reply) {
	const AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
	reply[3] = (uint8_t)animationOptions.baseAnimationIndex;
	reply[4] = onBoardAnimationCount();
}

// Page 4 - per-light grid positions, for spatially aware host effects.
// Reply: [3]=total entries, [4]=count here, then count x {first LED, x, y}.
// The classic render pipeline has no per-light positions, so it reports zero
// entries; hosts fall back to the layout enum from page 2.
static void buildCapsPositions(uint8_t * reply, uint8_t startEntry) {
	(void)startEntry;
	reply[3] = 0;
	reply[4] = 0;
}

// Page 5 - the light table: one twelve-byte record per light, the same layout
// on every pipeline, so a host parses one way and reads the flags and sentinels
// to learn which fields carry real data on this board.
//
// This pipeline keeps no per-light records - all it knows is a scalar config
// slot per control. Rather than answer with an empty page, which would leave
// every host maintaining a page 2 inference path forever for the firmware most
// people actually run, it rebuilds those slots into records and leaves the
// PER_LIGHT flag clear. That absence carries the honest part: these rows are
// per-control, so a board wiring two buttons to one action cannot be
// represented here, and each action is definitional rather than read back
// from a live light.
//
// Reply: [3] total records   [4] start entry, echoed   [5] count in this reply
//        [6] record stride   [7..54] records   [60..63] LED-map fingerprint
// The echo makes a paged walk self-identifying, which page 4 is not, and the
// fingerprint lets a host notice the map changing underneath a multi-read walk.
// Every control that has a light, in protocol ID order. This list IS the
// page 5 record order, so a light's ordinal - the index SET_LIGHT stages by -
// is its position here.
static uint8_t collectLightIds(uint8_t * ids) {
	uint8_t total = 0;
	for (uint8_t id = 0; id < HOST_LIGHTING_BUTTON_COUNT; id++) {
		if (resolveButton(id).first >= 0)
			ids[total++] = id;
	}
	for (uint8_t id = HOST_LIGHTING_BUTTON_PLED1; id <= HOST_LIGHTING_BUTTON_CASE; id++) {
		if (resolveButton(id).first >= 0)
			ids[total++] = id;
	}
	return total;
}

static void buildCapsLights(uint8_t * reply, uint8_t startEntry) {
	uint8_t ids[HOST_LIGHTING_BUTTON_COUNT + 6];
	uint8_t total = collectLightIds(ids);

	reply[3] = total;
	reply[4] = startEntry;
	reply[6] = HOST_LIGHTING_LIGHT_STRIDE;

	uint8_t count = 0;
	for (uint8_t i = startEntry;
			(i < total) && (count < HOST_LIGHTING_LIGHTS_PER_PAGE);
			i++, count++) {
		const uint8_t id = ids[i];
		const LedRange range = resolveButton(id);
		uint8_t * record = &reply[7 + count * HOST_LIGHTING_LIGHT_STRIDE];

		uint8_t type = HOST_LIGHTING_LIGHT_ACTION;
		uint8_t player = 0xFF;
		uint8_t pin = 0xFF;
		int16_t action = HOST_LIGHTING_ACTION_NONE;

		if (id < HOST_LIGHTING_BUTTON_COUNT) {
			action = (int16_t)canonicalAction[id];
			pin = pinForAction(canonicalAction[id]);
		} else if (id <= HOST_LIGHTING_BUTTON_PLED4) {
			type = (uint8_t)(HOST_LIGHTING_LIGHT_PLAYER1 + (id - HOST_LIGHTING_BUTTON_PLED1));
			player = (uint8_t)(id - HOST_LIGHTING_BUTTON_PLED1);
		} else if (id == HOST_LIGHTING_BUTTON_TURBO) {
			// The turbo light is addressed by its LED index, never by a pin: the
			// action named here is the turbo button, a different thing entirely
			type = HOST_LIGHTING_LIGHT_TURBO;
			action = (int16_t)GpioAction::BUTTON_PRESS_TURBO;
		} else {
			type = HOST_LIGHTING_LIGHT_CASE;
		}

		record[0]  = (uint8_t)range.first;
		record[1]  = range.count;
		record[2]  = type;
		record[3]  = id;
		record[4]  = pin;
		record[5]  = (uint8_t)(action & 0xFF);
		record[6]  = (uint8_t)((action >> 8) & 0xFF);
		record[7]  = player;
		record[8]  = 0xFF; // no case-group index exists in this pipeline
		record[9]  = 0;    // no positions either, so the POSITION flag stays clear
		record[10] = 0;
		// Both flags clear: this pipeline has no per-light table and no
		// positions, so it asserts neither. The record is rebuilt from
		// per-control configuration, which is exactly what a clear PER_LIGHT
		// tells a host.
		record[11] = 0;
	}
	reply[5] = count;

	uint32_t fingerprint = ledMapFingerprint();
	memcpy(&reply[60], &fingerprint, 4);
}

bool HostLighting::enabledForMode(InputMode mode) {
	const HostLightingOptions & options = Storage::getInstance().getAddonOptions().hostLightingOptions;
	if (!options.enabled)
		return false;
	if ((mode == INPUT_MODE_GENERIC) || (mode == INPUT_MODE_KEYBOARD))
		return true;
	if (mode == INPUT_MODE_XINPUT)
		return xinputCompositeActive();
	return false;
}

bool HostLighting::xinputCompositeActive() {
	const HostLightingOptions & options = Storage::getInstance().getAddonOptions().hostLightingOptions;
	if (!options.enabled)
		return false;
	if (options.xinputMode == HOST_LIGHTING_XINPUT_MODE_ON)
		return true;
	if (options.xinputMode == HOST_LIGHTING_XINPUT_MODE_AUTO)
		return watchdog_hw->scratch[HOST_LIGHTING_VERDICT_SCRATCH] == HOST_LIGHTING_VERDICT_PC_HOST;
	return false;
}

void HostLighting::xinputAutoDetectTask(bool consoleAuthSeen) {
	// The decision is final for the session, so the frequent process-loop calls
	// reduce to a single branch once it is made
	static bool settled = false;
	if (settled)
		return;

	const HostLightingOptions & options = Storage::getInstance().getAddonOptions().hostLightingOptions;
	if (!options.enabled || (options.xinputMode != HOST_LIGHTING_XINPUT_MODE_AUTO) ||
			xinputCompositeActive() || consoleAuthSeen) {
		settled = true;
		return;
	}

	static uint32_t enumeratedSinceMs = 0;
	if (!get_usb_mounted() || get_usb_suspended()) {
		enumeratedSinceMs = 0;
		return;
	}

	uint32_t now = to_ms_since_boot(get_absolute_time());
	if (enumeratedSinceMs == 0) {
		enumeratedSinceMs = now;
		return;
	}
	if ((now - enumeratedSinceMs) >= HOST_LIGHTING_XINPUT_DETECT_MS) {
		watchdog_hw->scratch[HOST_LIGHTING_VERDICT_SCRATCH] = HOST_LIGHTING_VERDICT_PC_HOST;
		System::reboot(System::BootMode::GAMEPAD);
	}
}

uint8_t HostLighting::lightingInstance() {
	// XInput's gamepad interface is vendor-class, leaving lighting as the only
	// HID interface; HID-class modes enumerate their gamepad as instance 0 first
	return (DriverManager::getInstance().getInputMode() == INPUT_MODE_XINPUT) ? 0 : HOST_LIGHTING_HID_INSTANCE;
}

bool HostLighting::isLightingInterface(uint8_t itf) {
	return (itf == lightingInstance()) &&
		enabledForMode(DriverManager::getInstance().getInputMode());
}

const uint8_t * HostLighting::getReportDescriptor() {
	return hostlighting_report_descriptor;
}

uint16_t HostLighting::getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t * buffer, uint16_t reqlen) {
	uint16_t size = (reqlen < HOST_LIGHTING_REPORT_SIZE) ? reqlen : HOST_LIGHTING_REPORT_SIZE;
	memcpy(buffer, responseBuffer, size);
	return size;
}

static void publishFrame() {
	liveSeq = liveSeq + 1; // odd: write in progress
	__mem_fence_release();
	memcpy(livePixels, stagingPixels, sizeof(livePixels));
	memcpy(liveValid, stagingValid, sizeof(liveValid));
	liveActive = true;
	__mem_fence_release();
	liveSeq = liveSeq + 1; // even: stable
}

// Shared by SET_LIGHT and SET_LIGHT_RGBW: stages one light per entry by its
// page 5 ordinal. On this pipeline the ordinal space is the collected control
// list, rebuilt here so the numbering always matches what page 5 most
// recently reported. The reply carries the counts in [3..4] and a per-entry
// outcome mask in [5..6] (bit n set = entry n applied), so a skip names the
// stale entry instead of forcing a page 5 re-walk.
static void stageLightEntries(const uint8_t * buffer, uint8_t entries,
		bool withWhite, uint8_t * responseBuffer) {
	uint8_t ids[HOST_LIGHTING_BUTTON_COUNT + 6];
	uint8_t total = collectLightIds(ids);
	uint8_t stride = withWhite ? 5 : 4;
	uint8_t applied = 0, skipped = 0;
	uint16_t mask = 0;
	for (uint8_t e = 0; e < entries; e++) {
		const uint8_t * entry = &buffer[3 + e * stride];
		if (entry[0] >= total) {
			skipped++;
			continue;
		}
		uint32_t colour = ((uint32_t)entry[1] << 16) |
			((uint32_t)entry[2] << 8) | entry[3];
		if (withWhite)
			colour |= (uint32_t)entry[4] << 24;
		stageRange(resolveButton(ids[entry[0]]), colour);
		mask |= (uint16_t)1 << e;
		applied++;
	}
	responseBuffer[3] = applied;
	responseBuffer[4] = skipped;
	responseBuffer[5] = (uint8_t)(mask & 0xFF);
	responseBuffer[6] = (uint8_t)(mask >> 8);
}

void HostLighting::setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t * buffer, uint16_t bufsize) {
	// Reports arrive via the OUT endpoint (type 0) or a SET_REPORT control request
	if ((report_type != HID_REPORT_TYPE_INVALID) && (report_type != HID_REPORT_TYPE_OUTPUT))
		return;

	if (bufsize < 2)
		return;

	uint8_t command = buffer[0];
	uint8_t sequence = buffer[1];
	uint8_t status = HOST_LIGHTING_STATUS_OK;
	bool doReboot = false;
	System::BootMode rebootMode = System::BootMode::GAMEPAD;

	memset(responseBuffer, 0, sizeof(responseBuffer));
	responseBuffer[0] = command | HOST_LIGHTING_RESPONSE_FLAG;
	responseBuffer[1] = sequence;

	switch (command) {
		case HOST_LIGHTING_CMD_PING:
			responseBuffer[3] = 'G';
			responseBuffer[4] = 'P';
			responseBuffer[5] = 'H';
			responseBuffer[6] = 'L';
			responseBuffer[7] = HOST_LIGHTING_PROTOCOL_VERSION_MAJOR;
			responseBuffer[8] = HOST_LIGHTING_PROTOCOL_VERSION_MINOR;
			break;

		case HOST_LIGHTING_CMD_GET_CAPS:
			if (bufsize < 3) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			if (buffer[2] == 0)
				buildCapsIdentity(responseBuffer);
			else if (buffer[2] == 1)
				buildCapsState(responseBuffer);
			else if (buffer[2] == 2)
				buildCapsLedMap(responseBuffer);
			else if (buffer[2] == 3)
				buildCapsAnimations(responseBuffer);
			else if (buffer[2] == 4)
				buildCapsPositions(responseBuffer, (bufsize > 3) ? buffer[3] : 0);
			else if (buffer[2] == 5)
				buildCapsLights(responseBuffer, (bufsize > 3) ? buffer[3] : 0);
			else
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
			break;

		case HOST_LIGHTING_CMD_SET_MODE: {
			if (bufsize < 6) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			if (buffer[2] > HOST_LIGHTING_TAKEOVER_OVERLAY) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			uint16_t timeoutMs = (uint16_t)buffer[3] | ((uint16_t)buffer[4] << 8);
			if (timeoutMs == 0)
				timeoutMs = HOST_LIGHTING_DEFAULT_TIMEOUT_MS;
			if (timeoutMs < 100)
				timeoutMs = 100;
			// Clamped rather than refused: a host asking for longer than we are
			// willing to hold the lights still gets a working session, just a
			// shorter leash than it asked for
			if (timeoutMs > HOST_LIGHTING_MAX_TIMEOUT_MS)
				timeoutMs = HOST_LIGHTING_MAX_TIMEOUT_MS;
			liveTakeover = buffer[2];
			liveTimeoutMs = timeoutMs;
			liveApplyBrightness = (buffer[5] != 0);
			break;
		}

		case HOST_LIGHTING_CMD_SET_BUTTONS: {
			if (bufsize < 3) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			uint8_t entries = buffer[2];
			if ((entries == 0) || (entries > HOST_LIGHTING_BUTTONS_MAX_ENTRIES) ||
					(bufsize < (uint16_t)(3 + entries * 4))) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			uint8_t applied = 0, skipped = 0;
			for (uint8_t e = 0; e < entries; e++) {
				const uint8_t * entry = &buffer[3 + e * 4];
				LedRange range = resolveButton(entry[0]);
				if (range.first < 0) {
					skipped++;
					continue;
				}
				stageRange(range, ((uint32_t)entry[1] << 16) | ((uint32_t)entry[2] << 8) | entry[3]);
				applied++;
			}
			responseBuffer[3] = applied;
			responseBuffer[4] = skipped;
			break;
		}

		case HOST_LIGHTING_CMD_SET_LIGHT: {
			if (bufsize < 3) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			uint8_t entries = buffer[2];
			if ((entries == 0) || (entries > HOST_LIGHTING_SET_LIGHT_MAX_ENTRIES) ||
					(bufsize < (uint16_t)(3 + entries * 4))) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			stageLightEntries(buffer, entries, false, responseBuffer);
			break;
		}

		case HOST_LIGHTING_CMD_SET_LIGHT_RGBW: {
			// As SET_LIGHT with a white component; boards whose colour format
			// has no white channel simply ignore the fifth byte
			if (bufsize < 3) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			uint8_t entries = buffer[2];
			if ((entries == 0) || (entries > HOST_LIGHTING_SET_LIGHT_RGBW_MAX_ENTRIES) ||
					(bufsize < (uint16_t)(3 + entries * 5))) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			stageLightEntries(buffer, entries, true, responseBuffer);
			break;
		}

		case HOST_LIGHTING_CMD_SET_RANGE: {
			if (bufsize < 4) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			uint8_t start = buffer[2];
			uint8_t count = buffer[3];
			if ((count == 0) || (count > HOST_LIGHTING_RANGE_MAX_PIXELS) ||
					((uint16_t)start + count > HOST_LIGHTING_MAX_LEDS) ||
					(bufsize < (uint16_t)(4 + count * 3))) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			for (uint8_t i = 0; i < count; i++) {
				const uint8_t * rgb = &buffer[4 + i * 3];
				stagePixel((uint32_t)start + i, ((uint32_t)rgb[0] << 16) | ((uint32_t)rgb[1] << 8) | rgb[2]);
			}
			break;
		}

		case HOST_LIGHTING_CMD_SET_RANGE_RGBW: {
			// As SET_RANGE with a white component; boards whose colour format
			// has no white channel simply ignore the fourth byte
			if (bufsize < 4) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			uint8_t start = buffer[2];
			uint8_t count = buffer[3];
			if ((count == 0) || (count > HOST_LIGHTING_RANGE_RGBW_MAX_PIXELS) ||
					((uint16_t)start + count > HOST_LIGHTING_MAX_LEDS) ||
					(bufsize < (uint16_t)(4 + count * 4))) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			for (uint8_t i = 0; i < count; i++) {
				const uint8_t * rgbw = &buffer[4 + i * 4];
				stagePixel((uint32_t)start + i,
					((uint32_t)rgbw[3] << 24) | ((uint32_t)rgbw[0] << 16) |
					((uint32_t)rgbw[1] << 8) | rgbw[2]);
			}
			break;
		}

		case HOST_LIGHTING_CMD_FILL: {
			if (bufsize < 6) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			uint32_t colour = ((uint32_t)buffer[3] << 16) | ((uint32_t)buffer[4] << 8) | buffer[5];
			switch (buffer[2]) {
				case HOST_LIGHTING_FILL_SCOPE_ALL:
					for (uint32_t i = 0; i < HOST_LIGHTING_MAX_LEDS; i++)
						stagePixel(i, colour);
					break;
				case HOST_LIGHTING_FILL_SCOPE_BUTTONS:
					for (uint8_t b = 0; b < HOST_LIGHTING_BUTTON_COUNT; b++)
						stageRange(resolveButton(b), colour);
					break;
				case HOST_LIGHTING_FILL_SCOPE_CASE:
					stageRange(resolveButton(HOST_LIGHTING_BUTTON_CASE), colour);
					break;
				case HOST_LIGHTING_FILL_SCOPE_PLEDS:
					for (uint8_t p = 0; p < 4; p++)
						stageRange(resolveButton(HOST_LIGHTING_BUTTON_PLED1 + p), colour);
					break;
				default:
					status = HOST_LIGHTING_STATUS_INVALID_ARG;
					break;
			}
			break;
		}

		case HOST_LIGHTING_CMD_CLEAR:
			memset(stagingPixels, 0, sizeof(stagingPixels));
			memset(stagingValid, 0, sizeof(stagingValid));
			break;

		case HOST_LIGHTING_CMD_COMMIT:
			publishFrame();
			break;

		case HOST_LIGHTING_CMD_RELEASE:
			liveActive = false;
			break;

		case HOST_LIGHTING_CMD_SET_ANIMATION: {
			// Selects the on-board animation shown outside host control; the
			// render core applies it, and the choice persists like the hotkeys'
			if (bufsize < 3) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			if (buffer[2] >= onBoardAnimationCount()) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
			if (animationOptions.baseAnimationIndex != buffer[2]) {
				animationOptions.baseAnimationIndex = buffer[2];
				EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(false));
			}
			pendingAnimationMode = buffer[2];
			break;
		}

		case HOST_LIGHTING_CMD_SET_INPUT_MODE: {
			// Guarded by a magic payload; persists the mode and reboots into it
			if ((bufsize < 7) || (buffer[3] != 'M') || (buffer[4] != 'O') ||
					(buffer[5] != 'D') || (buffer[6] != 'E')) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			InputMode inputMode = (InputMode)buffer[2];
			switch (inputMode) {
				case INPUT_MODE_XINPUT:
				case INPUT_MODE_SWITCH:
				case INPUT_MODE_PS3:
				case INPUT_MODE_KEYBOARD:
				case INPUT_MODE_PS4:
				case INPUT_MODE_XBONE:
				case INPUT_MODE_MDMINI:
				case INPUT_MODE_NEOGEO:
				case INPUT_MODE_PCEMINI:
				case INPUT_MODE_EGRET:
				case INPUT_MODE_ASTRO:
				case INPUT_MODE_PSCLASSIC:
				case INPUT_MODE_XBOXORIGINAL:
				case INPUT_MODE_PS5:
				case INPUT_MODE_GENERIC:
				case INPUT_MODE_SWITCH_PRO:
				case INPUT_MODE_P5GENERAL:
					break;
				default:
					// INPUT_MODE_CONFIG is entered via REBOOT_WEBCONFIG instead
					status = HOST_LIGHTING_STATUS_INVALID_ARG;
					break;
			}
			if (status != HOST_LIGHTING_STATUS_OK)
				break;
			GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
			if (gamepadOptions.inputMode != inputMode) {
				gamepadOptions.inputMode = inputMode;
				// Save and restart from the main loop rather than USB context
				EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true, true));
			} else {
				doReboot = true;
				rebootMode = System::BootMode::GAMEPAD;
			}
			break;
		}

		case HOST_LIGHTING_CMD_REBOOT_WEBCONFIG:
			// Guarded by a magic payload so a stray report cannot reboot the board
			if ((bufsize < 6) || (buffer[2] != 'W') || (buffer[3] != 'E') ||
					(buffer[4] != 'B') || (buffer[5] != 'C')) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			doReboot = true;
			rebootMode = System::BootMode::WEBCONFIG;
			break;

		case HOST_LIGHTING_CMD_REBOOT_BOOTSEL:
			// Guarded by a magic payload so a stray report cannot reboot the board
			if ((bufsize < 6) || (buffer[2] != 'B') || (buffer[3] != 'O') ||
					(buffer[4] != 'O') || (buffer[5] != 'T')) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			doReboot = true;
			rebootMode = System::BootMode::USB;
			break;

		default:
			status = HOST_LIGHTING_STATUS_UNSUPPORTED;
			break;
	}

	responseBuffer[2] = status;

	// Every valid command is a keepalive for the takeover timeout
	if (status == HOST_LIGHTING_STATUS_OK)
		lastCommandMs = to_ms_since_boot(get_absolute_time());

	uint8_t instance = HostLighting::lightingInstance();
	if (tud_hid_n_ready(instance)) {
		tud_hid_n_report(instance, 0, responseBuffer, sizeof(responseBuffer));
	}

	if (doReboot) {
		// The reply may not flush before reboot; hosts detect success by the
		// requested identity appearing on the bus
		System::reboot(rebootMode);
	}
}

// Host pixels are stored as 0xWWRRGGBB and convert through the framework's
// RGB::value(), so host colours and on-board animations render identically -
// with one carve-out. value()'s achromatic shortcut routes r==g==b colours to
// the white emitter and never reads W, which would render a host's
// subtractive white (0,0,0,W) black on a white-format chain. When the host
// supplied a white byte on such a chain, compose the channel word directly
// with value()'s own per-channel arithmetic instead; with no white byte the
// shortcut applies as always, so RGB-only hosts render exactly as before.
// Formats without a white channel take value() regardless, which ignores W.
// The compositions below must be kept byte-for-byte in step with the general
// GRBW/RGBW paths of RGB::value() in headers/animationstation/animation.h.
static uint32_t convertHostPixel(uint32_t stored, LEDFormat format, float brightnessX) {
	uint8_t r = (uint8_t)(stored >> 16), g = (uint8_t)(stored >> 8);
	uint8_t b = (uint8_t)stored, w = (uint8_t)(stored >> 24);
	if (w != 0) {
		if (format == LED_FORMAT_GRBW)
			return ((uint32_t)(g * brightnessX) << 24)
				| ((uint32_t)(r * brightnessX) << 16)
				| ((uint32_t)(b * brightnessX) << 8)
				| (uint32_t)(w * brightnessX);
		if (format == LED_FORMAT_RGBW)
			return ((uint32_t)(r * brightnessX) << 24)
				| ((uint32_t)(g * brightnessX) << 16)
				| ((uint32_t)(b * brightnessX) << 8)
				| (uint32_t)(w * brightnessX);
	}
	RGB colour(r, g, b, w);
	return colour.value(format, brightnessX);
}

void HostLighting::applyToFrame(uint32_t * frame, uint32_t ledCount, float brightnessX, int format) {
	if (!liveActive)
		return;

	uint32_t now = to_ms_since_boot(get_absolute_time());
	if ((now - lastCommandMs) > liveTimeoutMs)
		return; // host went quiet: let the on-board animations show

	// Colour conversion is soft-float on the RP2040, so the converted frame is
	// cached and only rebuilt when the host publishes or brightness/format
	// changes; steady-state render ticks reduce to a word copy
	static uint32_t convertedPixels[HOST_LIGHTING_MAX_LEDS];
	static uint32_t convertedValid[(HOST_LIGHTING_MAX_LEDS + 31) / 32];
	static uint32_t convertedSeq = 0xFFFFFFFF;
	static uint32_t convertedBrightnessBits = 0;
	static int convertedFormat = -1;

	float brightness = liveApplyBrightness ? brightnessX : 1.0f;
	uint32_t brightnessBits;
	memcpy(&brightnessBits, &brightness, sizeof(brightnessBits));

	uint32_t seqNow = liveSeq;
	if ((seqNow != convertedSeq) || (seqNow & 1) ||
			(brightnessBits != convertedBrightnessBits) || (format != convertedFormat)) {
		uint32_t local[HOST_LIGHTING_MAX_LEDS];
		uint32_t seqBefore, seqAfter;
		do {
			seqBefore = liveSeq;
			__mem_fence_acquire();
			memcpy(local, livePixels, sizeof(local));
			memcpy(convertedValid, liveValid, sizeof(convertedValid));
			__mem_fence_acquire();
			seqAfter = liveSeq;
		} while ((seqBefore != seqAfter) || (seqBefore & 1));

		for (uint32_t i = 0; i < HOST_LIGHTING_MAX_LEDS; i++)
			convertedPixels[i] = convertHostPixel(local[i],
				static_cast<LEDFormat>(format), brightness);
		convertedSeq = seqAfter;
		convertedBrightnessBits = brightnessBits;
		convertedFormat = format;
	}

	if (ledCount > HOST_LIGHTING_MAX_LEDS)
		ledCount = HOST_LIGHTING_MAX_LEDS;

	if (liveTakeover == HOST_LIGHTING_TAKEOVER_OVERLAY) {
		// Untouched pixels keep the animation underneath
		for (uint32_t i = 0; i < ledCount; i++) {
			if (convertedValid[i / 32] & (1u << (i % 32)))
				frame[i] = convertedPixels[i];
		}
	} else {
		memcpy(frame, convertedPixels, ledCount * sizeof(uint32_t));
	}
}

int16_t HostLighting::takeLocalAnimationRequest() {
	int16_t pending = pendingAnimationMode;
	if (pending >= 0)
		pendingAnimationMode = -1;
	return pending;
}
