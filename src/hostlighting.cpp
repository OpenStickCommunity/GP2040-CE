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
#include "pixel.h"

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

// Distilled copy of the LED addon's light layout, published at configure time
// (core1) and read by GET_CAPS / SET_BUTTONS (core0)
struct RegisteredLight {
	uint8_t first;
	uint8_t count;
	LightType type;
	int32_t gpioPin;
	int32_t playerIndex;
	int32_t nonButtonIndex;
	uint8_t posX;
	uint8_t posY;
};
static RegisteredLight registeredLights[HOST_LIGHTING_MAX_LEDS];
static uint8_t registeredLightCount = 0;
static volatile bool lightsReady = false;

void HostLighting::registerLights(const Lights & lights) {
	uint8_t count = 0;
	for (const Light & light : lights.AllLights) {
		if (count >= HOST_LIGHTING_MAX_LEDS)
			break;
		// Lights past the protocol's addressable LED space cannot be represented
		if (light.FirstLedIndex + light.LedsPerLight > HOST_LIGHTING_MAX_LEDS)
			continue;
		registeredLights[count].first = (uint8_t)light.FirstLedIndex;
		registeredLights[count].count = light.LedsPerLight;
		registeredLights[count].type = light.Type;
		registeredLights[count].gpioPin = light.GIPOPin;
		registeredLights[count].playerIndex = light.PlayerLightIndex;
		// Slot in the profile's non-button palette; not unique. Page 5 reports it
		// as the case group.
		registeredLights[count].nonButtonIndex = light.NonButtonIndex;
		int32_t x = light.Position.XPosition;
		int32_t y = light.Position.YPosition;
		registeredLights[count].posX = (uint8_t)((x < 0) ? 0 : ((x > 255) ? 255 : x));
		registeredLights[count].posY = (uint8_t)((y < 0) ? 0 : ((y > 255) ? 255 : y));
		count++;
	}
	registeredLightCount = count;
	lightsReady = true;
}

// GPIO action for each canonical button ID, referenced by name so an enum
// renumbering cannot change the wire. Order is the gamepad mask bit order.
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

// Resolves a protocol button ID to its physical LED chain range from the
// registered light layout; {-1, 0} when the control has no light.
static LedRange resolveButton(uint8_t buttonId) {
	LedRange none = { -1, 0 };
	if (!lightsReady)
		return none;

	if (buttonId < HOST_LIGHTING_BUTTON_COUNT) {
		GpioMappingInfo * pinMappings = Storage::getInstance().getProfilePinMappings();
		for (uint8_t i = 0; i < registeredLightCount; i++) {
			const RegisteredLight & light = registeredLights[i];
			if ((light.type != LightType::LightType_ActionButton) ||
					(light.gpioPin < 0) || (light.gpioPin >= (int32_t)NUM_BANK0_GPIOS))
				continue;
			if (pinMappings[light.gpioPin].action == canonicalAction[buttonId])
				return { (int16_t)light.first, light.count };
		}
		return none;
	}

	if ((buttonId >= HOST_LIGHTING_BUTTON_PLED1) && (buttonId <= HOST_LIGHTING_BUTTON_PLED4)) {
		for (uint8_t i = 0; i < registeredLightCount; i++) {
			if (registeredLights[i].playerIndex == (int32_t)(buttonId - HOST_LIGHTING_BUTTON_PLED1))
				return { (int16_t)registeredLights[i].first, registeredLights[i].count };
		}
		return none;
	}

	if (buttonId == HOST_LIGHTING_BUTTON_TURBO) {
		for (uint8_t i = 0; i < registeredLightCount; i++) {
			if (registeredLights[i].type == LightType::LightType_Turbo)
				return { (int16_t)registeredLights[i].first, registeredLights[i].count };
		}
		return none;
	}

	if (buttonId == HOST_LIGHTING_BUTTON_CASE) {
		int16_t first = -1;
		uint16_t total = 0;
		for (uint8_t i = 0; i < registeredLightCount; i++) {
			if (registeredLights[i].type != LightType::LightType_Case)
				continue;
			if ((first < 0) || (registeredLights[i].first < first))
				first = registeredLights[i].first;
			total += registeredLights[i].count;
		}
		if (first < 0)
			return none;
		return { first, (uint8_t)((total > 255) ? 255 : total) };
	}

	return none;
}

static void stagePixel(uint32_t index, uint32_t colour) {
	stagingPixels[index] = colour;
	stagingValid[index / 32] |= (1u << (index % 32));
}

// Clamped to the staging buffer: the case range is {min first, summed count},
// which can exceed the strip when configured case entries overlap.
static void stageRange(const LedRange & range, uint32_t colour) {
	if (range.first < 0)
		return;

	uint32_t end = (uint32_t)range.first + range.count;
	if (end > HOST_LIGHTING_MAX_LEDS)
		end = HOST_LIGHTING_MAX_LEDS;

	for (uint32_t index = (uint32_t)range.first; index < end; index++)
		stagePixel(index, colour);
}

// The GPIO action behind any stageable button ID, the extended set included;
// GpioAction::NONE for IDs that are not action buttons (the specials, and the
// permanently unassigned 20-23).
static GpioAction actionForButtonId(uint8_t buttonId) {
	if (buttonId < HOST_LIGHTING_BUTTON_COUNT)
		return canonicalAction[buttonId];
	if (buttonId == HOST_LIGHTING_BUTTON_A3)
		return GpioAction::BUTTON_PRESS_A3;
	if (buttonId == HOST_LIGHTING_BUTTON_A4)
		return GpioAction::BUTTON_PRESS_A4;
	if ((buttonId >= HOST_LIGHTING_BUTTON_E1) && (buttonId <= HOST_LIGHTING_BUTTON_E12))
		return (GpioAction)((int)GpioAction::BUTTON_PRESS_E1
			+ (buttonId - HOST_LIGHTING_BUTTON_E1));
	return GpioAction::NONE;
}

// Stages every light whose pin carries the control's action: boards may wire
// several buttons to one action (the Haute42 B16 has two Up and two L3).
// Returns false when the control has no light. Page 2 reports only the first.
static bool stageButton(uint8_t buttonId, uint32_t colour) {
	GpioAction action = actionForButtonId(buttonId);
	if (action == GpioAction::NONE) {
		LedRange range = resolveButton(buttonId);
		if (range.first < 0)
			return false;
		stageRange(range, colour);
		return true;
	}

	if (!lightsReady)
		return false;

	GpioMappingInfo * pinMappings = Storage::getInstance().getProfilePinMappings();
	bool any = false;
	for (uint8_t i = 0; i < registeredLightCount; i++) {
		const RegisteredLight & light = registeredLights[i];
		if ((light.type != LightType::LightType_ActionButton) ||
				(light.gpioPin < 0) || (light.gpioPin >= (int32_t)NUM_BANK0_GPIOS))
			continue;
		if (pinMappings[light.gpioPin].action != action)
			continue;
		LedRange range = { (int16_t)light.first, light.count };
		stageRange(range, colour);
		any = true;
	}
	return any;
}

// GET_CAPS pages: 0 identity, 1 runtime state (poll target), 2 LED map,
// 3 animations, 4 light positions, 5 light table.
// Pages 1, 2 and 5 carry the LED-map fingerprint.

// LED-map fingerprint: 32-bit FNV-1a over everything pages 2, 4 and 5
// report, so a host polling page 1 knows whether its cached pages are stale.
// Inputs, in order:
//   - each registered light: first LED, count, type, pin, player index,
//     non-button index, grid X, grid Y (omitted until the registry is ready)
//   - the active profile's action for every GPIO pin; light ownership is
//     resolved from the pin map at query time, so a profile switch changes it
//   - LEDOptions: ledsPerButton, ledFormat, ledLayout, brightnessMaximum
// Hashed from the registry, not resolveButton(), which returns only the first
// light per control and carries no positions.
static uint32_t ledMapFingerprint() {
	uint32_t fingerprint = 2166136261u;

	if (lightsReady) {
		for (uint8_t i = 0; i < registeredLightCount; i++) {
			const RegisteredLight & light = registeredLights[i];
			fingerprint = (fingerprint ^ light.first) * 16777619u;
			fingerprint = (fingerprint ^ light.count) * 16777619u;
			fingerprint = (fingerprint ^ (uint32_t)light.type) * 16777619u;
			fingerprint = (fingerprint ^ (uint32_t)light.gpioPin) * 16777619u;
			fingerprint = (fingerprint ^ (uint32_t)light.playerIndex) * 16777619u;
			fingerprint = (fingerprint ^ (uint32_t)light.nonButtonIndex) * 16777619u;
			fingerprint = (fingerprint ^ light.posX) * 16777619u;
			fingerprint = (fingerprint ^ light.posY) * 16777619u;
		}
	}

	GpioMappingInfo * pinMappings = Storage::getInstance().getProfilePinMappings();
	for (uint8_t pin = 0; pin < (uint8_t)NUM_BANK0_GPIOS; pin++)
		fingerprint = (fingerprint ^ (uint32_t)pinMappings[pin].action) * 16777619u;

	// LEDOptions fields reported on page 2
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

	for (uint8_t b = 0; b < HOST_LIGHTING_BUTTON_COUNT; b++) {
		LedRange range = resolveButton(b);
		if (range.first < 0) {
			reply[8 + b * 2] = 0xFF;
			reply[9 + b * 2] = 0;
		} else {
			reply[8 + b * 2] = (uint8_t)range.first;
			reply[9 + b * 2] = range.count;
		}
	}

	for (uint8_t p = 0; p < 4; p++) {
		LedRange range = resolveButton(HOST_LIGHTING_BUTTON_PLED1 + p);
		reply[44 + p] = (range.first < 0) ? 0xFF : (uint8_t)range.first;
	}

	LedRange turbo = resolveButton(HOST_LIGHTING_BUTTON_TURBO);
	reply[48] = (turbo.first < 0) ? 0xFF : (uint8_t)turbo.first;

	LedRange caseRange = resolveButton(HOST_LIGHTING_BUTTON_CASE);
	reply[49] = (caseRange.first < 0) ? 0xFF : (uint8_t)caseRange.first;
	reply[50] = caseRange.count;

	// LED extent: highest LED index in use plus one, from the registry. Not the
	// sum of the ranges above, which misses lights page 2 cannot name, and not
	// derived from the collapsed case range, which can overstate the strip.
	uint32_t ledExtent = 0;
	for (uint8_t i = 0; i < registeredLightCount; i++) {
		uint32_t end = (uint32_t)registeredLights[i].first + registeredLights[i].count;
		if (end > ledExtent)
			ledExtent = end;
	}
	reply[6] = (uint8_t)((ledExtent > 255) ? 255 : ledExtent);

	uint32_t fingerprint = ledMapFingerprint();
	memcpy(&reply[51], &fingerprint, 4);
}

// Render rate in Hz, published by the LED addon; 0 until then (not stated)
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
// [12..18] were added in v1.1; earlier firmware leaves them zero.
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

	// baseProfileIndex is -1 when no profile is selected; reported as 0xFF
	reply[11] = (animationOptions.baseProfileIndex < 0)
		? 0xFF : (uint8_t)animationOptions.baseProfileIndex;

	// Both bits follow the registry, which is populated on the render core during
	// LED setup; a host that reads earlier sees them clear and re-reads.
	uint32_t features = 0;
	if (lightsReady && (registeredLightCount > 0))
		features |= HOST_LIGHTING_FEATURE_POSITIONS | HOST_LIGHTING_FEATURE_LIGHT_TABLE;
	memcpy(&reply[12], &features, 4);

	reply[16] = HOST_LIGHTING_FRAMEWORK_REFACTOR;
	reply[17] = HOST_LIGHTING_ANIM_PROFILES;
	reply[18] = renderRateHz;
}

// Page 3 - on-board animations:
// [3] current profile index (0xFF = none)   [4] profile count
// The count is the profiles that exist, not the capacity.
static void buildCapsAnimations(uint8_t * reply) {
	const AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
	// -1 (no profile selected) reports as the 0xFF sentinel
	reply[3] = (animationOptions.baseProfileIndex < 0)
		? 0xFF : (uint8_t)animationOptions.baseProfileIndex;
	reply[4] = (uint8_t)animationOptions.profiles_count;
}

// Page 4 - per-light grid positions from the registered light layout:
// [3]=total entries, [4]=count here, then count x {first LED, x, y}
// A projection of page 5 that fits 19 entries per reply against page 5's 4.
static void buildCapsPositions(uint8_t * reply, uint8_t startEntry) {
	if (!lightsReady) {
		reply[3] = 0;
		reply[4] = 0;
		return;
	}
	reply[3] = registeredLightCount;
	uint8_t count = 0;
	for (uint8_t i = startEntry;
			(i < registeredLightCount) && (count < HOST_LIGHTING_POSITIONS_PER_PAGE);
			i++, count++) {
		reply[5 + count * 3] = registeredLights[i].first;
		reply[6 + count * 3] = registeredLights[i].posX;
		reply[7 + count * 3] = registeredLights[i].posY;
	}
	reply[4] = count;
}

// Resolves a light's owner: button ID, pin action and pin. The action is read
// from the active profile at query time, not cached at registration, because a
// profile switch remaps pins without re-registering the lights.
static void resolveLight(const RegisteredLight & light, uint8_t & buttonId,
		int16_t & action, uint8_t & pin) {
	buttonId = HOST_LIGHTING_BUTTON_NONE;
	action = HOST_LIGHTING_ACTION_NONE;
	pin = 0xFF;

	switch (light.type) {
		case LightType::LightType_Case:
			buttonId = HOST_LIGHTING_BUTTON_CASE;
			return;
		case LightType::LightType_Turbo:
			// The stored pin is the turbo LED's driver pin, not a mapped GPIO, so no
			// pin is reported; the action is the turbo button's
			buttonId = HOST_LIGHTING_BUTTON_TURBO;
			action = (int16_t)GpioAction::BUTTON_PRESS_TURBO;
			return;
		case LightType::LightType_Player1Light:
		case LightType::LightType_Player2Light:
		case LightType::LightType_Player3Light:
		case LightType::LightType_Player4Light:
			if ((light.playerIndex >= 0) && (light.playerIndex < 4))
				buttonId = (uint8_t)(HOST_LIGHTING_BUTTON_PLED1 + light.playerIndex);
			return;
		default:
			break;
	}

	if ((light.gpioPin < 0) || (light.gpioPin >= (int32_t)NUM_BANK0_GPIOS))
		return;

	pin = (uint8_t)light.gpioPin;
	GpioMappingInfo * pinMappings = Storage::getInstance().getProfilePinMappings();
	const GpioAction pinAction = pinMappings[light.gpioPin].action;
	action = (int16_t)pinAction;

	for (uint8_t id = 0; id < HOST_LIGHTING_BUTTON_COUNT; id++) {
		if (canonicalAction[id] == pinAction) {
			buttonId = id;
			return;
		}
	}

	// A3, A4 and E1-E12 have no page 2 slot. Any other action keeps the NONE ID
	// and is identified by its action field.
	if (pinAction == GpioAction::BUTTON_PRESS_A3)
		buttonId = HOST_LIGHTING_BUTTON_A3;
	else if (pinAction == GpioAction::BUTTON_PRESS_A4)
		buttonId = HOST_LIGHTING_BUTTON_A4;
	else if ((pinAction >= GpioAction::BUTTON_PRESS_E1) &&
			(pinAction <= GpioAction::BUTTON_PRESS_E12))
		buttonId = (uint8_t)(HOST_LIGHTING_BUTTON_E1 +
			((int)pinAction - (int)GpioAction::BUTTON_PRESS_E1));
}

// Page 5 - light table: one twelve-byte record per light.
//
// Reply: [3] total records   [4] start entry, echoed   [5] count in this reply
//        [6] record stride   [7..54] records   [60..63] LED-map fingerprint
// The start echo identifies each reply of a paged walk (first LED is not a
// unique key); the fingerprint detects the map changing mid-walk.
static void buildCapsLights(uint8_t * reply, uint8_t startEntry) {
	reply[6] = HOST_LIGHTING_LIGHT_STRIDE;

	// Written before the early return so an empty reply still carries it
	const uint32_t fingerprint = ledMapFingerprint();
	memcpy(&reply[60], &fingerprint, 4);

	if (!lightsReady) {
		reply[3] = 0;
		reply[4] = startEntry;
		reply[5] = 0;
		return;
	}

	reply[3] = registeredLightCount;
	reply[4] = startEntry;

	uint8_t count = 0;
	for (uint8_t i = startEntry;
			(i < registeredLightCount) && (count < HOST_LIGHTING_LIGHTS_PER_PAGE);
			i++, count++) {
		const RegisteredLight & light = registeredLights[i];
		uint8_t * record = &reply[7 + count * HOST_LIGHTING_LIGHT_STRIDE];

		uint8_t buttonId, pin;
		int16_t action;
		resolveLight(light, buttonId, action, pin);

		// A light with no owning pin can carry an out-of-range type: report UNKNOWN
		const uint8_t type = ((unsigned)light.type <= HOST_LIGHTING_LIGHT_PLAYER4)
			? (uint8_t)light.type : HOST_LIGHTING_LIGHT_UNKNOWN;

		record[0]  = light.first;
		record[1]  = light.count;
		record[2]  = type;
		record[3]  = buttonId;
		record[4]  = pin;
		record[5]  = (uint8_t)(action & 0xFF);
		record[6]  = (uint8_t)((action >> 8) & 0xFF);
		record[7]  = (light.playerIndex >= 0) ? (uint8_t)light.playerIndex : 0xFF;
		record[8]  = (light.nonButtonIndex >= 0) ? (uint8_t)light.nonButtonIndex : 0xFF;
		record[9]  = light.posX;
		record[10] = light.posY;
		// Records come from the per-light table and carry real coordinates
		record[11] = HOST_LIGHTING_LIGHT_FLAG_POSITION | HOST_LIGHTING_LIGHT_FLAG_PER_LIGHT;
	}
	reply[5] = count;
}

bool HostLighting::enabledForMode(InputMode mode) {
	const HostLightingOptions & options = Storage::getInstance().getAddonOptions().hostLightingOptions;
	if (!options.enabled)
		return false;
	if ((mode == INPUT_MODE_GENERIC) || (mode == INPUT_MODE_KEYBOARD) || (mode == INPUT_MODE_SINPUT))
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
	// Decided once per boot; later calls return immediately
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

// SET_LIGHT / SET_LIGHT_RGBW: stages one light per entry by page 5 ordinal,
// which is the registry index. Reply: [3] applied   [4] skipped
// [5..6] outcome mask LE (bit n set = entry n applied).
static void stageLightEntries(const uint8_t * buffer, uint8_t entries,
		bool withWhite, uint8_t * responseBuffer) {
	uint8_t stride = withWhite ? 5 : 4;
	uint8_t applied = 0, skipped = 0;
	uint16_t mask = 0;
	for (uint8_t e = 0; e < entries; e++) {
		const uint8_t * entry = &buffer[3 + e * stride];
		if (!lightsReady || (entry[0] >= registeredLightCount)) {
			skipped++;
			continue;
		}
		const RegisteredLight & light = registeredLights[entry[0]];
		LedRange range = { (int16_t)light.first, light.count };
		uint32_t colour = ((uint32_t)entry[1] << 16) |
			((uint32_t)entry[2] << 8) | entry[3];
		if (withWhite)
			colour |= (uint32_t)entry[4] << 24;
		stageRange(range, colour);
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
			// An over-long timeout is clamped, not rejected
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
				uint32_t colour = ((uint32_t)entry[1] << 16) | ((uint32_t)entry[2] << 8) | entry[3];
				if (stageButton(entry[0], colour))
					applied++;
				else
					skipped++;
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
					// Every button light in the registry, including lights on the
					// extended controls page 2 cannot name
					if (lightsReady) {
						for (uint8_t i = 0; i < registeredLightCount; i++) {
							if (registeredLights[i].type != LightType::LightType_ActionButton)
								continue;
							LedRange range = { (int16_t)registeredLights[i].first,
								registeredLights[i].count };
							stageRange(range, colour);
						}
					}
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
			// Selects the on-board animation profile shown outside host control
			if (bufsize < 3) { status = HOST_LIGHTING_STATUS_INVALID_ARG; break; }
			AnimationOptions & animationOptions = Storage::getInstance().getAnimationOptions();
			if (buffer[2] >= animationOptions.profiles_count) {
				status = HOST_LIGHTING_STATUS_INVALID_ARG;
				break;
			}
			if (animationOptions.baseProfileIndex != buffer[2]) {
				animationOptions.baseProfileIndex = buffer[2];
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
				case INPUT_MODE_SINPUT:
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

// Host pixels are stored as 0xWWRRGGBB and converted with RGB::value(), so
// host colours match the on-board animations. Exception: value() maps r==g==b
// to the white emitter and ignores W, so when the host supplies W on a GRBW or
// RGBW chain the channel word is composed directly. Keep both compositions in
// step with the GRBW/RGBW paths of RGB::value() (animation.h).
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
