/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#include "hostlighting.h"

#include <algorithm>
#include <string.h>

#include "pico/platform.h"
#include "pico/unique_id.h"
#include "device/dcd.h"

#include "BoardConfig.h"
#include "drivermanager.h"
#include "eventmanager.h"
#include "helper.h"
#include "peripheralmanager.h"
#include "rebootmodes.h"
#include "storagemanager.h"
#include "system.h"
#include "version.h"
#include "animationstation.h"
#include "pixel.h"
#include "NeoPico.h"
#include "inputmodes.h"
#include "keyboardinterface.h"
#include "addons/he_trigger.h"

namespace {

// Wire constants, in the order of the tables in docs/host-lighting.md.

constexpr uint8_t PROTOCOL_MAJOR = 2;
constexpr uint8_t PROTOCOL_MINOR = 0;
constexpr uint8_t REPLY_FLAG = 0x80;
constexpr uint8_t EVENT_MARKER = 0x80;

// Request flags
constexpr uint8_t FLAG_COMMIT_AFTER = 1 << 0;
constexpr uint8_t FLAG_NO_REPLY = 1 << 1;
constexpr uint8_t FLAG_KEEPALIVE = 1 << 2;
constexpr uint8_t FLAG_HOLDER = 1 << 3;
// Bits 4-6 hold the claim generation the sender acts under, 0 for none
constexpr uint8_t FLAG_GENERATION_SHIFT = 4;
constexpr uint8_t FLAG_GENERATION = 7 << FLAG_GENERATION_SHIFT;
constexpr uint8_t FLAGS_DEFINED = FLAG_COMMIT_AFTER | FLAG_NO_REPLY | FLAG_KEEPALIVE | FLAG_HOLDER | FLAG_GENERATION;

// Status
constexpr uint8_t STATUS_OK = 0;
constexpr uint8_t STATUS_UNSUPPORTED = 1;
constexpr uint8_t STATUS_INVALID = 2;
constexpr uint8_t STATUS_STALE = 3;
constexpr uint8_t STATUS_CLAIMED = 4;

// Commands
constexpr uint8_t CMD_HELLO = 0x01;
constexpr uint8_t CMD_GET_PAGE = 0x02;
constexpr uint8_t CMD_CLAIM = 0x03;
constexpr uint8_t CMD_CONFIGURE = 0x04;
constexpr uint8_t CMD_SUBSCRIBE = 0x05;
constexpr uint8_t CMD_STAGE = 0x10;
constexpr uint8_t CMD_UNSTAGE = 0x11;
constexpr uint8_t CMD_FILL = 0x12;
constexpr uint8_t CMD_CLEAR = 0x13;
constexpr uint8_t CMD_COMMIT = 0x20;
constexpr uint8_t CMD_RELEASE = 0x21;
constexpr uint8_t CMD_SET_PROFILE = 0x30;
constexpr uint8_t CMD_SET_ANIMATION = 0x31;
constexpr uint8_t CMD_SET_ANIMATION_SPEED = 0x32;
constexpr uint8_t CMD_SET_BRIGHTNESS = 0x33;
constexpr uint8_t CMD_SET_KEYBOARD_MAPPING = 0x34;
constexpr uint8_t CMD_SET_INPUT_MODE = 0x70;
constexpr uint8_t CMD_REBOOT = 0x71;
// Staging and lifecycle commands refresh the keepalive of a takeover and
// of an unsaved brightness step
constexpr uint8_t CMD_KEEPALIVE_FIRST = 0x10;
constexpr uint8_t CMD_KEEPALIVE_LAST = 0x2F;

// HELLO reply
constexpr uint8_t HELLO_MAGIC = 3;
constexpr uint8_t HELLO_MAJOR = 7;
constexpr uint8_t HELLO_MINOR = 8;
constexpr uint8_t HELLO_BOARD_ID = 9;
constexpr uint8_t HELLO_QUEUE_LIMIT = 17;
constexpr uint8_t HELLO_FLAGS = 18;
constexpr uint8_t HELLO_COMMANDS = 19;
constexpr uint8_t HELLO_PAGES = 35;
constexpr uint8_t HELLO_ADDRESSING = 39;
constexpr uint8_t HELLO_PIXEL_FORMATS = 41;
constexpr uint8_t HELLO_EVENTS = 43;
constexpr uint8_t HELLO_EVENTS_ENABLED = 45;

// CLAIM request and reply; the reply echoes the action and token, then
// shows the holder as page 1's Holder group, as a status 4 reply does
constexpr uint8_t CLAIM_ACTION = 3;
constexpr uint8_t CLAIM_TOKEN = 4;
constexpr uint8_t CLAIM_NAME = 8;
constexpr uint8_t CLAIM_FLAGS = 24;
constexpr uint8_t CLAIM_HOLDER = 8;
constexpr uint8_t CLAIMED_HOLDER = 3;
constexpr uint8_t HOLDER_SIZE = 22;
// The claim generation steps 1 to GENERATION_MAX and back to 1
constexpr uint8_t GENERATION_MAX = 7;
constexpr uint8_t CLAIM_TAKE_OVER = 1;
constexpr uint8_t CLAIM_GIVE_UP = 2;
constexpr uint8_t CLAIM_EXCLUSIVE = 1 << 0;
constexpr uint8_t CLAIM_FLAGS_DEFINED = CLAIM_EXCLUSIVE;
// What an exclusive claim lets through from a request without HOLDER and the
// current generation, these commands always and these others unless sent
// with these flags
constexpr uint8_t EXCLUSIVE_OPEN_COMMANDS[] = { CMD_CLAIM };
constexpr uint8_t EXCLUSIVE_OPEN_UNFLAGGED[] = { CMD_HELLO, CMD_GET_PAGE, CMD_SUBSCRIBE };
constexpr uint8_t EXCLUSIVE_REFUSED_FLAGS = FLAG_COMMIT_AFTER | FLAG_KEEPALIVE;

// CONFIGURE request and reply, the same layout
constexpr uint8_t CONFIGURE_TAKEOVER = 3;
constexpr uint8_t CONFIGURE_TIMEOUT = 4;
constexpr uint8_t CONFIGURE_BRIGHTNESS = 6;
constexpr uint8_t TAKEOVER_OVERLAY = 1;
constexpr uint16_t TIMEOUT_MIN_MS = 100;
constexpr uint16_t TIMEOUT_MAX_MS = 10000;

// Pages
constexpr uint8_t PAGE_IDENTITY = 0;
constexpr uint8_t PAGE_STATE = 1;
constexpr uint8_t PAGE_INPUT_MODES = 2;
constexpr uint8_t PAGE_NAMES = 3;
constexpr uint8_t PAGE_BOOT_HOLDS = 4;
constexpr uint8_t PAGE_SUMMARY = 8;
constexpr uint8_t PAGE_PROFILES = 9;
constexpr uint8_t PAGE_CONTROLS = 10;
constexpr uint8_t PAGE_KEYBOARD_MAPPING = 11;
constexpr uint8_t PAGE_LIGHTS = 12;
constexpr uint8_t PAGE_ANIMATIONS = 13;

// GET_PAGE request and the reply header
constexpr uint8_t GET_PAGE_PAGE = 3;
constexpr uint8_t GET_PAGE_START = 4;
constexpr uint8_t GET_PAGE_TOKEN = 6;
constexpr uint8_t GET_PAGE_MASK = 10;
constexpr uint8_t HEADER_PAGE = 3;
constexpr uint8_t HEADER_START = 4;
constexpr uint8_t HEADER_TOTAL = 6;
constexpr uint8_t HEADER_COUNT = 8;
constexpr uint8_t HEADER_STRIDE = 9;
constexpr uint8_t HEADER_GROUPS = 10;
constexpr uint8_t HEADER_RECORDS = 12;
constexpr uint8_t HEADER_TOKEN = 60;
constexpr uint8_t RECORD_AREA = 48;

// Field group widths, in bit order
constexpr uint8_t IDENTITY_GROUPS = 8;
constexpr uint8_t IDENTITY_STRING_MAX = 48;
constexpr uint8_t STATE_GROUPS[] = { 10, HOLDER_SIZE, 4 };
constexpr uint8_t INPUT_MODE_GROUPS[] = { 2, 22, 3, 3, 3 };
constexpr uint8_t NAME_GROUPS[] = { 3, 40 };
constexpr uint8_t BOOT_HOLD_GROUPS[] = { 4, 11 };
constexpr uint8_t SUMMARY_GROUPS[] = { 30, 8 };
constexpr uint8_t PROFILE_GROUPS[] = { 18 };
constexpr uint8_t CONTROL_GROUPS[] = { 4, 4, 6, 4 };
constexpr uint8_t KEYBOARD_MAPPING_GROUPS[] = { 3, 1 };
constexpr uint8_t LIGHT_GROUPS[] = { 4, 2, 3, 4, 2, 1 };
constexpr uint8_t ANIMATION_GROUPS[] = { 4, 3 };
constexpr uint8_t INPUT_MODE_NAME_SIZE = 22;
constexpr uint8_t NAME_SIZE = 40;
// Page 3 enums
constexpr uint8_t NAMES_GPIO_ACTION = 0;
constexpr uint8_t NAMES_LIGHT_TYPE = 1;
constexpr uint8_t NAMES_AUTH_TYPE = 2;
constexpr uint8_t NAMES_DEVICE_TYPE = 3;
constexpr uint8_t NAMES_DPAD_MODE = 4;
constexpr uint8_t NAMES_BUTTON_LAYOUT = 5;
constexpr uint8_t NAMES_BUTTON_LAYOUT_RIGHT = 6;
constexpr uint8_t NAMES_BUTTON_LAYOUT_ORIENTATION = 7;
constexpr uint8_t NAMES_NON_PRESSED_EFFECT = 8;
constexpr uint8_t NAMES_PRESSED_EFFECT = 9;
constexpr uint8_t PROFILE_LABEL_SIZE = 16;

// Staging
constexpr uint8_t ADDRESSING_LED_RUN = 0;
constexpr uint8_t ADDRESSING_LIGHT_RUN = 1;
constexpr uint8_t ADDRESSING_LIGHT = 4;
constexpr uint8_t ADDRESSING_PIN = 5;
constexpr uint8_t ADDRESSING_ACTION = 6;
constexpr uint8_t ADDRESSING_KIND = 7;
constexpr uint16_t ADDRESSING_DEFINED = (1 << 0) | (1 << 1) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7);
constexpr uint8_t PIXEL_RGB = 0;
constexpr uint8_t PIXEL_RGBW = 1;
constexpr uint8_t PIXEL_RGB565 = 2;
constexpr uint8_t PIXEL_BYTES[] = { 3, 4, 2 };
constexpr uint8_t RUN_FIRST = 4;
constexpr uint8_t RUN_COUNT = 6;
constexpr uint8_t RUN_DATA = 7;
constexpr uint8_t ENTRY_COUNT = 4;
constexpr uint8_t ENTRY_DATA = 5;
constexpr uint8_t OUTCOME_BITS = 32;
constexpr uint8_t STAGE_APPLIED = 3;
constexpr uint8_t STAGE_SKIPPED = 4;
constexpr uint8_t STAGE_OUTCOME = 5;

// SET_ANIMATION_SPEED request; the three speeds are consecutive
constexpr uint8_t SET_ANIMATION_SPEED_INDEX = 3;
constexpr uint8_t SET_ANIMATION_SPEED_IDLE = 5;
constexpr uint8_t SET_ANIMATION_SPEED_PRESSED = 6;
constexpr uint8_t SET_ANIMATION_SPEED_CASE = 7;
constexpr uint8_t SPEED_UNCHANGED = 0xFF;

// SET_BRIGHTNESS request
constexpr uint8_t SET_BRIGHTNESS_STEP = 3;
constexpr uint8_t SET_BRIGHTNESS_SAVE = 4;
constexpr uint8_t BRIGHTNESS_SAVED = 0xFF;

// SET_KEYBOARD_MAPPING request; the codes are consecutive
constexpr uint8_t SET_KEYBOARD_MAPPING_PROFILE = 3;
constexpr uint8_t SET_KEYBOARD_MAPPING_FIRST = 4;
constexpr uint8_t SET_KEYBOARD_MAPPING_COUNT = 6;
constexpr uint8_t SET_KEYBOARD_MAPPING_CODES = 7;

// Page 1 State flags
constexpr uint8_t STATE_LIVE = 1 << 0;
constexpr uint8_t STATE_UNSAVED = 1 << 1;
constexpr uint8_t STATE_IDLE = 1 << 2;

// Page 2 Input modes: flags, capabilities
constexpr uint8_t INPUT_MODE_LIT = 1 << 0;
constexpr uint8_t INPUT_MODE_DETECTED = 1 << 1;
constexpr uint8_t INPUT_MODE_ACCEPTED = 1 << 2;
constexpr uint8_t INPUT_MODE_CAP_PLAYER = 1 << 0;
constexpr uint8_t INPUT_MODE_CAP_KEYBOARD_CODES = 1 << 1;

// Page 4 Boot holds: kinds (1 and 2 as REBOOT targets), flags, sources
constexpr uint8_t HOLD_INPUT_MODE = 0;
constexpr uint8_t HOLD_WEB_CONFIGURATOR = 1;
constexpr uint8_t HOLD_UF2_BOOTLOADER = 2;
constexpr uint8_t HOLD_LOCKED = 1 << 0;
constexpr uint8_t HOLD_BUTTONS = 0;
constexpr uint8_t HOLD_PINS = 1;
constexpr uint8_t HOLD_ACTIONS = 3;

// Page 8 Summary flags
constexpr uint8_t SUMMARY_WHITE_CHANNEL = 1 << 0;
constexpr uint8_t SUMMARY_USB_HOST = 1 << 1;

// Page 11 Keyboard mapping flags
constexpr uint8_t KEYBOARD_MAPPING_SENDS_KEY = 1 << 0;

// Events
constexpr uint16_t EVENT_STATE_CHANGED = 1 << 0;
constexpr uint16_t EVENTS_DEFINED = EVENT_STATE_CHANGED;

// Page 10 keys. GPIO pin p is key p; add-on input n is its base + n
constexpr uint8_t KEY_TRIGGER = 0x80;  // Hall-effect trigger
constexpr uint8_t KEY_EXPANDER = 0xC0; // PCF8575 pin
constexpr uint16_t KEY_END = 0x100;

// None values, never equal to a key
constexpr uint8_t PIN_NONE = 0xFF;
constexpr uint16_t ACTION_NONE = 0x8000;
constexpr uint16_t INDEX_NONE = 0xFFFF;
constexpr uint8_t SLOT_NONE = 0xFF;
constexpr uint8_t VALUE_NONE = 0xFF; // an 8-bit enumerated value, input mode or profile
constexpr uint16_t ANIMATION_OFF = 0xFFFF;

// A gap this long between two clock checks is a core-0 stall (a settings
// flash write stops core 0 and core 1 for hundreds of milliseconds); it is
// not counted toward the keepalive or the lease.
constexpr uint32_t STALL_US = 20000;

// While subscribed, page 1 fields changed outside the request path are
// compared at this interval, so their event follows within it
constexpr uint32_t WATCH_INTERVAL_US = 10000;

inline void put16(uint8_t* p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
inline void put32(uint8_t* p, uint32_t v) { put16(p, (uint16_t)v); put16(p + 2, (uint16_t)(v >> 16)); }
inline uint16_t get16(const uint8_t* p) { return (uint16_t)(p[0] | (p[1] << 8)); }
inline uint32_t get32(const uint8_t* p) { return get16(p) | ((uint32_t)get16(p + 2) << 16); }
inline void setBit(uint8_t* bitmap, unsigned bit) { bitmap[bit / 8] |= (uint8_t)(1u << (bit % 8)); }

} // namespace

// ---------------------------------------------------------------------------
// Lights

uint16_t HostLighting::lightCount() {
	if (lights == nullptr)
		return 0;
	size_t n = lights->AllLights.size();
	return (uint16_t)std::min<size_t>(n, 0xFFFF);
}

static uint8_t gridCoord(int v) {
	return (uint8_t)std::clamp(v, 0, 255);
}

// LEDs of a light below the limit as [first, end), false when none
bool HostLighting::lightLeds(const Light& light, uint16_t& first, uint16_t& end) {
	if (light.FirstLedIndex >= LED_LIMIT)
		return false;
	uint32_t last = light.FirstLedIndex + light.LedsPerLight;
	first = (uint16_t)light.FirstLedIndex;
	end = (uint16_t)std::min<uint32_t>(last, LED_LIMIT);
	return end > first;
}

// ---------------------------------------------------------------------------
// Frames

static bool whiteChain() {
	LEDFormat_Proto format = Storage::getInstance().getLedOptions().ledFormat;
	return (format == LEDFormat_Proto_LED_FORMAT_GRBW) || (format == LEDFormat_Proto_LED_FORMAT_RGBW);
}

// A wire pixel as stored. On a chain with a white emitter an RGB or RGB565
// grey is stored as W alone, as RGB::value() maps it for the animations.
static uint32_t decodePixel(const uint8_t* p, uint8_t format, bool white) {
	uint8_t r, g, b;
	if (format == PIXEL_RGBW)
		return ((uint32_t)p[3] << 24) | ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | p[2];
	if (format == PIXEL_RGB) {
		r = p[0];
		g = p[1];
		b = p[2];
	} else {
		uint16_t v = get16(p);
		uint8_t r5 = (uint8_t)(v >> 11), g6 = (uint8_t)((v >> 5) & 0x3F), b5 = (uint8_t)(v & 0x1F);
		r = (uint8_t)((r5 << 3) | (r5 >> 2));
		g = (uint8_t)((g6 << 2) | (g6 >> 4));
		b = (uint8_t)((b5 << 3) | (b5 >> 2));
	}
	if (white && (r == g) && (g == b))
		return (uint32_t)r << 24;
	return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

void HostLighting::stageLed(uint16_t led, const uint32_t* pixel) {
	if (pixel != nullptr) {
		stagedPixels[led] = *pixel;
		stagedBits[led / 32] |= 1u << (led % 32);
	} else {
		stagedPixels[led] = 0;
		stagedBits[led / 32] &= ~(1u << (led % 32));
	}
}

// Every LED of the light below the limit; false when it has none
bool HostLighting::stageLight(uint16_t ordinal, const uint32_t* pixel) {
	uint16_t first, end;
	if ((ordinal >= lightCount()) || !lightLeds(lights->AllLights[ordinal], first, end))
		return false;
	for (uint16_t led = first; led < end; led++)
		stageLed(led, pixel);
	return true;
}

void HostLighting::clearStaged() {
	memset(stagedPixels, 0, sizeof(stagedPixels));
	memset(stagedBits, 0, sizeof(stagedBits));
}

// ---------------------------------------------------------------------------
// Session takeover, keepalive, claim and subscription (core 0)

void HostLighting::raiseEvent() {
	if (subscription & EVENT_STATE_CHANGED)
		eventPending = true;
}

// Every end of a takeover clears the staged frame, as CLEAR does
void HostLighting::endTakeover() {
	clearStaged();
	if (liveActive) {
		liveActive = false;
		raiseEvent();
	}
}

void HostLighting::publish() {
	liveSeq = liveSeq + 1;
	__mem_fence_release();
	memcpy(livePixels, stagedPixels, sizeof(livePixels));
	memcpy(liveBits, stagedBits, sizeof(liveBits));
	__mem_fence_release();
	liveSeq = liveSeq + 1;
	if (!liveActive) {
		liveActive = true;
		takeoverNumber++;
		keepaliveUs = clockUs;
		clockRunning = true;
		raiseEvent();
	}
}

// Page 1's Holder group; the CLAIM reply and a status 4 reply carry it
void HostLighting::putHolder(uint8_t* at) {
	put32(at, claimToken);
	memcpy(at + 4, claimName, CLAIM_NAME_SIZE);
	at[4 + CLAIM_NAME_SIZE] = claimFlags;
	at[5 + CLAIM_NAME_SIZE] = claimGeneration;
}

// Ends the takeover and an unsaved brightness step, clears the staged frame
// and returns the CONFIGURE settings to their defaults
void HostLighting::resetControl() {
	endTakeover();
	restoreBrightness();
	takeoverMode = TAKEOVER_WHOLE_FRAME;
	timeoutMs = TIMEOUT_DEFAULT_MS;
	applyBrightness = true;
}

// Every change of holder resets the control state and steps the generation
void HostLighting::setHolder(uint32_t token, const uint8_t* name, uint8_t flags) {
	resetControl();
	claimGeneration = (uint8_t)((claimGeneration % GENERATION_MAX) + 1);
	claimToken = token;
	if (name != nullptr)
		memcpy(claimName, name, CLAIM_NAME_SIZE);
	else
		memset(claimName, 0, CLAIM_NAME_SIZE);
	claimFlags = flags;
	leaseUs = clockUs;
	if (token != 0)
		clockRunning = true;
	raiseEvent();
}

// Advances the clock and ends an expired takeover, lease or unsaved step. A
// gap longer than STALL_US since the last check is a core-0 stall and moves
// every deadline by its length.
void __not_in_flash_func(HostLighting::checkExpiry)() {
	uint64_t now = getMicro();
	if (clockRunning) {
		uint64_t gap = now - clockUs;
		if (gap > STALL_US) {
			keepaliveUs += gap;
			leaseUs += gap;
			unsavedUs += gap;
		}
	}
	clockUs = now;
	const int32_t timeout = (int32_t)timeoutMs * 1000;
	if (liveActive && ((int64_t)(now - keepaliveUs) > timeout))
		endTakeover();
	if ((claimToken != 0) && ((int64_t)(now - leaseUs) > timeout))
		setHolder(0, nullptr, 0);
	// Only a SET_BRIGHTNESS leaves an unsaved step, so the steps are read
	// only after one
	bool unsaved = unsavedStep && brightnessUnsaved();
	if (unsaved && ((int64_t)(now - unsavedUs) > timeout)) {
		restoreBrightness();
		unsaved = false;
	}
	// A request core 1 has not applied yet can still leave an unsaved step
	unsavedStep = unsaved || (brightnessRequests != brightnessApplied);
	clockRunning = liveActive || (claimToken != 0) || unsavedStep;
}

// ---------------------------------------------------------------------------
// Host requests to core 1, which owns the animations

// Without the LED add-on (no LED output) nothing animates, with no
// animations, no brightness steps, animation off and step 0. SET_ANIMATION
// then accepts only off and SET_BRIGHTNESS only step 0 or 0xFF (the saved
// step); neither changes anything.
// AnimationStation and the web configurator run every profile, enabled or
// not.
uint16_t HostLighting::animationTotal() {
	return ledsActive ? MAX_ANIMATION_PROFILES : 0;
}

uint8_t HostLighting::brightnessSteps() {
	return ledsActive ? AnimationStation::getInstance().getBrightnessSteps() : 0;
}

uint8_t HostLighting::brightnessMaximum() {
	return ledsActive ? AnimationStation::getInstance().GetMaxBrightness() : 0;
}

// Animation speed steps. The speed hotkeys step 0 to CYCLE_STEPS - 1, the
// web configurator 1 to CYCLE_STEPS
uint8_t HostLighting::speedSteps() {
	return ledsActive ? CYCLE_STEPS : 0;
}

static uint8_t speedByte(int32_t speed) {
	return (uint8_t)std::clamp<int32_t>(speed, 0, 255);
}

// The animation the board shows and the brightness steps saved and shown,
// a pending request included
int8_t HostLighting::currentAnimation() {
	if (!ledsActive)
		return -1;
	return (animationRequests != animationApplied) ? animationRequest : (int8_t)Storage::getInstance().getAnimationOptions().baseProfileIndex;
}

// A step to save that core 1 has not applied yet, else BRIGHTNESS_SAVED
uint8_t __not_in_flash_func(HostLighting::pendingSave)() {
	return (brightnessRequests != brightnessApplied) ? (uint8_t)(brightnessRequest >> 8) : BRIGHTNESS_SAVED;
}

uint8_t __not_in_flash_func(HostLighting::savedBrightness)() {
	if (!ledsActive)
		return 0;
	uint8_t save = pendingSave();
	if (save != BRIGHTNESS_SAVED)
		return save;
	// Stored unclamped; the step shown is clamped to the steps
	uint32_t saved = Storage::getInstance().getAnimationOptions().brightness;
	return (uint8_t)std::min<uint32_t>(saved, brightnessSteps());
}

uint8_t __not_in_flash_func(HostLighting::shownBrightness)() {
	if (!ledsActive)
		return 0;
	if (brightnessRequests != brightnessApplied) {
		uint8_t show = (uint8_t)brightnessRequest;
		return (show == BRIGHTNESS_SAVED) ? savedBrightness() : show;
	}
	return AnimationStation::getInstance().GetBrightnessStepValue();
}

void HostLighting::postBrightness(uint8_t save, uint8_t show) {
	brightnessRequest = (uint16_t)((save << 8) | show);
	__mem_fence_release();
	brightnessRequests = brightnessRequests + 1;
}

// The step shown is unsaved while it differs from the saved step. Only an
// unsaved SET_BRIGHTNESS sets one without the other, and the hotkeys set both
bool __not_in_flash_func(HostLighting::brightnessUnsaved)() {
	return shownBrightness() != savedBrightness();
}

// Shows the saved step again, keeping a save core 1 has not applied yet
void HostLighting::restoreBrightness() {
	if (brightnessUnsaved())
		postBrightness(pendingSave(), BRIGHTNESS_SAVED);
}

// The animations' idle timeout has them off, until a button press. Read from
// core 0 as the brightness step is, one byte written by core 1
bool HostLighting::animationsIdle() {
	return ledsActive && AnimationStation::getInstance().IsInIdleTimeout();
}

uint16_t HostLighting::animationIndex() {
	int8_t index = currentAnimation();
	return (index < 0) ? ANIMATION_OFF : (uint16_t)index;
}

static uint32_t storedProfile() {
	return Storage::getInstance().getGamepadOptions().profileNumber;
}

// The profile the pin map was last built from (GP2040::getReinitGamepad).
// The pages report its mappings. A switch reaches it at the next
// main-loop pass, with the token
static uint32_t appliedProfile() {
	return Storage::getInstance().GetGamepad()->lastReinitProfileNumber;
}

// The profile with the pin map a stored number puts in effect, as
// Storage::setFunctionalPinMappings() selects. A number that is not an
// enabled alternative profile means the base mapping, profile 1
static uint8_t activeProfile(uint32_t number) {
	const ProfileOptions& profiles = Storage::getInstance().getProfileOptions();
	if ((number >= 2) && (number <= profiles.gpioMappingsSets_count + 1u) &&
			profiles.gpioMappingsSets[number - 2].enabled)
		return (uint8_t)number;
	return 1;
}

static uint8_t playerNumber() {
	uint32_t player = Storage::getInstance().GetProcessedGamepad()->auxState.playerID.value;
	return (uint8_t)std::min<uint32_t>(player, 255);
}

// ---------------------------------------------------------------------------
// Input modes (page 2), from the InputMode attribute table (inputmodes.h)

// Every InputMode but the config mode, which REBOOT target 1 enters
static bool listedInputMode(const InputModeAttributes& mode) {
	return mode.mode != INPUT_MODE_CONFIG;
}

// A listed mode with a driver, one that requires the USB host peripheral only
// while it is enabled, as the web configurator allows
static bool acceptedInputMode(const InputModeAttributes& mode) {
	return listedInputMode(mode) && (mode.capabilities & INPUT_MODE_DRIVER) &&
		((mode.usbHost != INPUT_MODE_USB_HOST_REQUIRED) || PeripheralManager::getInstance().isUSBEnabled(0));
}

// ---------------------------------------------------------------------------
// Names (page 3), the values of the enums the pages carry with their names in
// enums.proto. The generated names of LightType and the animation effects
// carry the type name, left out.

struct EnumName {
	int16_t value;
	const char* name;
};

#define HOST_LIGHTING_NAME(name, value) { (int16_t)(value), #name },
#define HOST_LIGHTING_KIND_NAME(name, value) { (int16_t)(value), #name + sizeof("LightType") },
#define HOST_LIGHTING_NON_PRESSED_NAME(name, value) \
	{ (int16_t)(value), #name + sizeof("AnimationNonPressedEffects") },
#define HOST_LIGHTING_PRESSED_NAME(name, value) { (int16_t)(value), #name + sizeof("AnimationPressedEffects") },
static constexpr EnumName actionNames[] = { GpioAction_VALUELIST(HOST_LIGHTING_NAME) };
static constexpr EnumName kindNames[] = { LightType_VALUELIST(HOST_LIGHTING_KIND_NAME) };
static constexpr EnumName authTypeNames[] = { InputModeAuthType_VALUELIST(HOST_LIGHTING_NAME) };
static constexpr EnumName deviceTypeNames[] = { InputModeDeviceType_VALUELIST(HOST_LIGHTING_NAME) };
static constexpr EnumName dpadModeNames[] = { DpadMode_VALUELIST(HOST_LIGHTING_NAME) };
static constexpr EnumName buttonLayoutNames[] = { ButtonLayout_VALUELIST(HOST_LIGHTING_NAME) };
static constexpr EnumName buttonLayoutRightNames[] = { ButtonLayoutRight_VALUELIST(HOST_LIGHTING_NAME) };
static constexpr EnumName orientationNames[] = { ButtonLayoutOrientation_VALUELIST(HOST_LIGHTING_NAME) };
static constexpr EnumName nonPressedEffectNames[] = { AnimationNonPressedEffects_VALUELIST(HOST_LIGHTING_NON_PRESSED_NAME) };
static constexpr EnumName pressedEffectNames[] = { AnimationPressedEffects_VALUELIST(HOST_LIGHTING_PRESSED_NAME) };
constexpr bool hasPrefix(const char* text, const char* prefix) {
	return (*prefix == 0) || ((*text == *prefix) && hasPrefix(text + 1, prefix + 1));
}
#define HOST_LIGHTING_KIND_PREFIX(name, value) \
	static_assert(hasPrefix(#name, "LightType_"), "page 3 strips the LightType_ prefix");
#define HOST_LIGHTING_NON_PRESSED_PREFIX(name, value) \
	static_assert(hasPrefix(#name, "AnimationNonPressedEffects_"), "page 3 strips the AnimationNonPressedEffects_ prefix");
#define HOST_LIGHTING_PRESSED_PREFIX(name, value) \
	static_assert(hasPrefix(#name, "AnimationPressedEffects_"), "page 3 strips the AnimationPressedEffects_ prefix");
LightType_VALUELIST(HOST_LIGHTING_KIND_PREFIX)
AnimationNonPressedEffects_VALUELIST(HOST_LIGHTING_NON_PRESSED_PREFIX)
AnimationPressedEffects_VALUELIST(HOST_LIGHTING_PRESSED_PREFIX)
#undef HOST_LIGHTING_KIND_PREFIX
#undef HOST_LIGHTING_NON_PRESSED_PREFIX
#undef HOST_LIGHTING_PRESSED_PREFIX
#undef HOST_LIGHTING_NAME
#undef HOST_LIGHTING_KIND_NAME
#undef HOST_LIGHTING_NON_PRESSED_NAME
#undef HOST_LIGHTING_PRESSED_NAME
static_assert((_GpioAction_MIN >= INT16_MIN) && (_GpioAction_MAX <= INT16_MAX),
	"page 3 carries GpioAction values as signed 16-bit");
static_assert((_InputModeAuthType_MAX < 8) && (_InputModeDeviceType_MAX < 16),
	"page 2 carries one bit per authentication method (8) and device type (16)");

// Page 3's enums in record order, each with its page 3 number
struct EnumNames {
	uint8_t id;
	const EnumName* names;
	uint16_t count;
};

#define HOST_LIGHTING_ENUM_NAMES(id, names) { id, names, sizeof(names) / sizeof(names[0]) }
static constexpr EnumNames nameEnums[] = {
	HOST_LIGHTING_ENUM_NAMES(NAMES_GPIO_ACTION, actionNames),
	HOST_LIGHTING_ENUM_NAMES(NAMES_LIGHT_TYPE, kindNames),
	HOST_LIGHTING_ENUM_NAMES(NAMES_AUTH_TYPE, authTypeNames),
	HOST_LIGHTING_ENUM_NAMES(NAMES_DEVICE_TYPE, deviceTypeNames),
	HOST_LIGHTING_ENUM_NAMES(NAMES_DPAD_MODE, dpadModeNames),
	HOST_LIGHTING_ENUM_NAMES(NAMES_BUTTON_LAYOUT, buttonLayoutNames),
	HOST_LIGHTING_ENUM_NAMES(NAMES_BUTTON_LAYOUT_RIGHT, buttonLayoutRightNames),
	HOST_LIGHTING_ENUM_NAMES(NAMES_BUTTON_LAYOUT_ORIENTATION, orientationNames),
	HOST_LIGHTING_ENUM_NAMES(NAMES_NON_PRESSED_EFFECT, nonPressedEffectNames),
	HOST_LIGHTING_ENUM_NAMES(NAMES_PRESSED_EFFECT, pressedEffectNames),
};
#undef HOST_LIGHTING_ENUM_NAMES

// ---------------------------------------------------------------------------
// Keyboard mapping (page 11), the fields of KeyboardMapping in their order,
// each with the action of its button, found by name: keyDpadUp is
// BUTTON_PRESS_UP and keyButtonB1 BUTTON_PRESS_B1, as KeyboardDriver presses
// them.

namespace {

constexpr char upperCase(char c) {
	return ((c >= 'a') && (c <= 'z')) ? (char)(c - 'a' + 'A') : c;
}

constexpr bool sameKey(const char* field, const char* action) {
	return (*field == 0) ? (*action == 0) : ((upperCase(*field) == *action) && sameKey(field + 1, action + 1));
}

// The button a field maps, the name after keyDpad or keyButton
constexpr const char* keyButton(const char* field) {
	return hasPrefix(field, "keyDpad") ? field + sizeof("keyDpad") - 1
		: (hasPrefix(field, "keyButton") ? field + sizeof("keyButton") - 1 : nullptr);
}

// The BUTTON_PRESS_ action of a field's button; NONE when there is none
constexpr int16_t keyboardAction(const char* field) {
	const char* button = keyButton(field);
	for (const EnumName& entry : actionNames) {
		if ((button != nullptr) && hasPrefix(entry.name, "BUTTON_PRESS_")
				&& sameKey(button, entry.name + sizeof("BUTTON_PRESS_") - 1))
			return entry.value;
	}
	return (int16_t)GpioAction::NONE;
}

struct KeyboardMappingField {
	int16_t action;
	uint32_t KeyboardMapping::* code;
};

#define HOST_LIGHTING_KEYBOARD_FIELD(parent, atype, htype, ltype, name, tag, other) \
	{ keyboardAction(#name), &KeyboardMapping::name },
constexpr KeyboardMappingField keyboardMappingFields[] = { KeyboardMapping_FIELDLIST(HOST_LIGHTING_KEYBOARD_FIELD, unused) };
#undef HOST_LIGHTING_KEYBOARD_FIELD
constexpr uint8_t KEYBOARD_MAPPING_FIELDS = sizeof(keyboardMappingFields) / sizeof(keyboardMappingFields[0]);

constexpr bool keyboardMappingNamed() {
	for (const KeyboardMappingField& field : keyboardMappingFields) {
		if (field.action == (int16_t)GpioAction::NONE)
			return false;
	}
	return true;
}
static_assert(keyboardMappingNamed(), "each KeyboardMapping field names the BUTTON_PRESS_ action of its button");

// Page 10 actions join page 11, so each action has at most one field
constexpr bool keyboardMappingUnique() {
	for (uint8_t i = 0; i < KEYBOARD_MAPPING_FIELDS; i++) {
		for (uint8_t j = i + 1; j < KEYBOARD_MAPPING_FIELDS; j++) {
			if (keyboardMappingFields[i].action == keyboardMappingFields[j].action)
				return false;
		}
	}
	return true;
}
static_assert(keyboardMappingUnique(), "each action has at most one KeyboardMapping field");

} // namespace

// ---------------------------------------------------------------------------
// Pages

struct HostLighting::Writer {
	uint8_t* p;
	void u8(uint8_t v) { *p++ = v; }
	void u16(uint16_t v) { put16(p, v); p += 2; }
	void u32(uint32_t v) { put32(p, v); p += 4; }
	void bytes(const void* src, uint8_t n) { memcpy(p, src, n); p += n; }
	void text(const char* s, uint8_t size) {
		size_t n = strnlen(s, size);
		memcpy(p, s, n);
		memset(p + n, 0, size - n);
		p += size;
	}
};

// Hall-effect triggers the add-on scans, one bit per trigger
// (HETriggerAddon::isTriggerScanned)
static uint32_t triggersScanned() {
	const HETriggerOptions& options = Storage::getInstance().getAddonOptions().heTriggerOptions;
	int32_t channels = options.muxChannels;
	if (!options.enabled || ((channels != 1) && (channels != 4) && (channels != 8) && (channels != 16)))
		return 0;
	uint32_t scanned = 0;
	for (uint32_t i = 0; i < HETRIGGER_COUNT; i++)
		scanned |= HETriggerAddon::isTriggerScanned(options, i) ? (1u << i) : 0;
	return scanned;
}

// Expander pins, while the PCF8575 add-on is loaded
static uint8_t expanderTotal(bool loaded) {
	const PCF8575Options& options = Storage::getInstance().getAddonOptions().pcf8575Options;
	const uint8_t size = sizeof(options.pins) / sizeof(options.pins[0]);
	return !loaded ? 0 : (uint8_t)std::min<uint32_t>(options.pins_count, size);
}

// Whether a key is a Hall-effect trigger the add-on scans
bool HostLighting::isScannedTriggerKey(uint16_t key) {
	return (key >= KEY_TRIGGER) && ((key - KEY_TRIGGER) < HETRIGGER_COUNT) &&
		(scannedTriggers & (1u << (key - KEY_TRIGGER)));
}

// The page 10 key of the input a light follows, a GPIO pin or a
// Hall-effect trigger the add-on reads. -1 when it follows neither.
int HostLighting::ownerKey(const Light& light) {
	if (light.InputKey < 0)
		return -1;
	if (light.InputSource == LightInputSource::LightInputSource_HallEffect)
		return ((light.InputIndex < HETRIGGER_COUNT) && isScannedTriggerKey(KEY_TRIGGER + light.InputIndex)) ?
			(int)KEY_TRIGGER + light.InputIndex : -1;
	return (light.InputIndex < NUM_BANK0_GPIOS) ? (int)light.InputIndex : -1;
}

// The key after key, in key order; KEY_END after the last
uint16_t HostLighting::nextKey(uint16_t key) {
	if (++key == NUM_BANK0_GPIOS)
		key = KEY_TRIGGER;
	while ((key >= KEY_TRIGGER) && (key < KEY_EXPANDER) && !isScannedTriggerKey(key))
		key++;
	if (key == KEY_EXPANDER + expanderKeys)
		key = KEY_END;
	return key;
}

// The action of a key from nextKey(). An expander output mirrors a button
// and is not an input.
static GpioAction keyAction(uint16_t key) {
	const AddonOptions& addons = Storage::getInstance().getAddonOptions();
	if (key < KEY_TRIGGER)
		return Storage::getInstance().getProfilePinMappings()[key].action;
	if (key < KEY_EXPANDER)
		return addons.heTriggerOptions.triggers[key - KEY_TRIGGER].action;
	const GpioMappingInfo& pin = addons.pcf8575Options.pins[key - KEY_EXPANDER];
	return (pin.direction == GPIO_DIRECTION_INPUT) ? pin.action : GpioAction::NONE;
}

// Which of the controls with an action this key is, from 1, in key order,
// and how many there are
void HostLighting::controlInstance(uint16_t key, GpioAction action, uint16_t& instance, uint16_t& instances) {
	instance = 0;
	instances = 0;
	for (uint16_t k = 0; k != KEY_END; k = nextKey(k)) {
		if (keyAction(k) != action)
			continue;
		instances++;
		if (k <= key)
			instance++;
	}
}

static bool isControl(GpioAction action) { return (int)action > 0; }

// The key of the index-th control; false past the last
bool HostLighting::controlKey(uint16_t index, uint16_t& key) {
	for (uint16_t k = 0; k != KEY_END; k = nextKey(k)) {
		if (!isControl(keyAction(k)))
			continue;
		if (index-- == 0) {
			key = k;
			return true;
		}
	}
	return false;
}

struct HostLighting::ControlCounts {
	uint16_t total = 0;
	uint16_t lit = 0;
};

HostLighting::ControlCounts HostLighting::controlCounts() {
	ControlCounts counts;
	for (uint16_t k = 0; k != KEY_END; k = nextKey(k)) {
		if (!isControl(keyAction(k)))
			continue;
		counts.total++;
		// Lights follow GPIO pins and Hall-effect triggers
		if ((k < NUM_BANK0_GPIOS) && (litPins & (1ull << k)))
			counts.lit++;
		else if ((k >= KEY_TRIGGER) && (k < KEY_EXPANDER) && (litTriggers & (1u << (k - KEY_TRIGGER))))
			counts.lit++;
	}
	return counts;
}

// Page 0 - Identity. The firmware information of /api/getFirmwareVersion
// plus the version ID, one string per group
static const char* const identityStrings[IDENTITY_GROUPS] = {
	BOARD_CONFIG_LABEL, GP2040_BOARDCONFIG, GP2040PLATFORM, GP2040VERSION,
	GP2040VERSIONID, GP2040BUILD, GP2040CONFIG, BOARD_CONFIG_FILE_NAME,
};

static uint8_t identityWidth(uint8_t group) {
	size_t n = strlen(identityStrings[group]);
	return (uint8_t)std::min<size_t>(n + 1, IDENTITY_STRING_MAX);
}

// Page 1 - State
uint16_t HostLighting::oneRecord() { return 1; }

// The D-pad mode Gamepad::process applies, from a held SUSTAIN_DP_MODE_* button
// or else GamepadOptions.dpadMode
static uint8_t dpadMode() {
	return (uint8_t)Storage::getInstance().GetGamepad()->getActiveDpadMode();
}

void HostLighting::stateRecord(uint16_t, uint16_t groups, Writer& w) {
	if (groups & (1 << 0)) {
		w.u8((uint8_t)((liveActive ? STATE_LIVE : 0) | (brightnessUnsaved() ? STATE_UNSAVED : 0) |
			(animationsIdle() ? STATE_IDLE : 0)));
		w.u8((uint8_t)DriverManager::getInstance().getInputMode());
		w.u8(activeProfile(appliedProfile()));
		w.u8(playerNumber());
		w.u16(animationIndex());
		w.u8(shownBrightness());
		w.u8(takeoverNumber);
		w.u8(savedBrightness());
		w.u8(dpadMode());
	}
	if (groups & (1 << 1)) {
		uint8_t holder[HOLDER_SIZE];
		putHolder(holder);
		w.bytes(holder, HOLDER_SIZE);
	}
	if (groups & (1 << 2)) {
		w.u8(takeoverMode);
		w.u16(timeoutMs);
		w.u8(applyBrightness ? 1 : 0);
	}
}

// Page 2 - Input modes
uint16_t HostLighting::inputModeTotal() {
	uint16_t n = 0;
	for (const InputModeAttributes& m : INPUT_MODE_ATTRIBUTES)
		n += listedInputMode(m) ? 1 : 0;
	return n;
}

// The authentication method the mode's driver uses: its GamepadOptions
// setting, or the one method it supports; VALUE_NONE for a mode with none
static uint8_t selectedAuthType(const InputModeAttributes& m) {
	if (m.authSetting != nullptr)
		return (uint8_t)(Storage::getInstance().getGamepadOptions().*m.authSetting);
	for (uint8_t type = 0; type < 8; type++) {
		if (m.authTypes == (1u << type))
			return type;
	}
	return VALUE_NONE;
}

void HostLighting::inputModeRecord(uint16_t index, uint16_t groups, Writer& w) {
	for (const InputModeAttributes& m : INPUT_MODE_ATTRIBUTES) {
		if (!listedInputMode(m) || (index-- != 0))
			continue;
		if (groups & (1 << 0)) {
			const HostLightingOptions& options = Storage::getInstance().getAddonOptions().hostLightingOptions;
			uint8_t flags = acceptedInputMode(m) ? INPUT_MODE_ACCEPTED : 0;
			if (m.lighting == INPUT_MODE_LIGHTING_ALWAYS)
				flags |= INPUT_MODE_LIT;
			else if ((m.lighting == INPUT_MODE_LIGHTING_SETTING) && (options.xinputMode != HOST_LIGHTING_XINPUT_MODE_OFF))
				flags |= INPUT_MODE_LIT | ((options.xinputMode == HOST_LIGHTING_XINPUT_MODE_AUTO) ? INPUT_MODE_DETECTED : 0);
			w.u8((uint8_t)m.mode);
			w.u8(flags);
		}
		if (groups & (1 << 1))
			w.text(m.name, INPUT_MODE_NAME_SIZE);
		if (groups & (1 << 2)) {
			// Page 1's player field is read only in a mode with the interface
			bool player = (m.capabilities & INPUT_MODE_PLAYER) && (m.lighting != INPUT_MODE_LIGHTING_NEVER);
			w.u8(m.lighting);
			w.u8(m.category);
			w.u8((player ? INPUT_MODE_CAP_PLAYER : 0) |
				((m.capabilities & INPUT_MODE_KEYBOARD_CODES) ? INPUT_MODE_CAP_KEYBOARD_CODES : 0));
		}
		if (groups & (1 << 3)) {
			w.u8(m.usbHost);
			w.u8(m.authTypes);
			w.u8(selectedAuthType(m));
		}
		if (groups & (1 << 4)) {
			w.u16(m.deviceTypes);
			w.u8((m.deviceTypes != 0)
				? (uint8_t)Storage::getInstance().getGamepadOptions().inputDeviceType : VALUE_NONE);
		}
		return;
	}
}

// Page 3 - Names, the values of each enum in nameEnums order
uint16_t HostLighting::nameTotal() {
	uint16_t n = 0;
	for (const EnumNames& e : nameEnums)
		n += e.count;
	return n;
}

void HostLighting::nameRecord(uint16_t index, uint16_t groups, Writer& w) {
	for (const EnumNames& e : nameEnums) {
		if (index >= e.count) {
			index -= e.count;
			continue;
		}
		const EnumName& entry = e.names[index];
		if (groups & (1 << 0)) {
			w.u8(e.id);
			w.u16((uint16_t)entry.value);
		}
		if (groups & (1 << 1))
			w.text(entry.name, NAME_SIZE);
		return;
	}
}

// Page 4 - Boot holds, in the order GP2040::getBootAction() and
// getGpioMappedBootAction() check them

namespace {

// The GamepadOptions fields that pick the input mode of a button held alone at
// boot, found by name as page 11's are: inputModeB1 is BUTTON_PRESS_B1. They
// are int32, a mode below zero disabled.
constexpr int16_t bootButtonAction(const char* field) {
	const char* button = hasPrefix(field, "inputMode") ? field + sizeof("inputMode") - 1 : nullptr;
	for (const EnumName& entry : actionNames) {
		if ((button != nullptr) && (*button != 0) && hasPrefix(entry.name, "BUTTON_PRESS_")
				&& sameKey(button, entry.name + sizeof("BUTTON_PRESS_") - 1))
			return entry.value;
	}
	return (int16_t)GpioAction::NONE;
}

constexpr int32_t GamepadOptions::* int32Field(int32_t GamepadOptions::* field) { return field; }
template <typename T>
constexpr int32_t GamepadOptions::* int32Field(T GamepadOptions::*) { return nullptr; }

struct GamepadOptionField {
	const char* name;
	int16_t action;
	int32_t GamepadOptions::* mode;
};

#define HOST_LIGHTING_GAMEPAD_FIELD(parent, atype, htype, ltype, name, tag, other) \
	{ #name, bootButtonAction(#name), int32Field(&GamepadOptions::name) },
constexpr GamepadOptionField gamepadOptionFields[] = { GamepadOptions_FIELDLIST(HOST_LIGHTING_GAMEPAD_FIELD, unused) };
#undef HOST_LIGHTING_GAMEPAD_FIELD

constexpr bool isBootButton(const GamepadOptionField& field) {
	return (field.action != (int16_t)GpioAction::NONE) && (field.mode != nullptr);
}

// Every inputMode<button> field is an int32 that names a BUTTON_PRESS_ action
constexpr bool bootButtonsNamed() {
	for (const GamepadOptionField& field : gamepadOptionFields) {
		if (hasPrefix(field.name, "inputMode") && (field.name[sizeof("inputMode") - 1] != 0) && !isBootButton(field))
			return false;
	}
	return true;
}
static_assert(bootButtonsNamed(), "each GamepadOptions inputMode<button> field is an int32 naming a BUTTON_PRESS_ action");

constexpr uint8_t bootButtonCount() {
	uint8_t n = 0;
	for (const GamepadOptionField& field : gamepadOptionFields)
		n += isBootButton(field) ? 1 : 0;
	return n;
}

constexpr uint8_t BOOT_MAPPINGS = sizeof(BootModeOptions::inputModeMappings) / sizeof(InputModeMapping);

// The UF2 bootloader and the web configurator, then the input modes
constexpr uint8_t BOOT_HOLDS_MAX = 2 + ((bootButtonCount() > BOOT_MAPPINGS) ? bootButtonCount() : BOOT_MAPPINGS);

struct BootHold {
	uint8_t kind;
	uint8_t mode;
	uint8_t profile;
	uint8_t flags;
	uint8_t source;
	int16_t actions[HOLD_ACTIONS];
	uint32_t pins;
};

uint8_t bootHolds(BootHold* holds) {
	Storage& storage = Storage::getInstance();
	const BootModeOptions& boot = storage.getBootModeOptions();
	const int16_t none = (int16_t)ACTION_NONE;
	uint8_t n = 0;
	auto add = [&](uint8_t kind, uint8_t mode, uint8_t profile, bool locked, uint32_t pins,
			int16_t first, int16_t second = (int16_t)ACTION_NONE, int16_t third = (int16_t)ACTION_NONE) {
		holds[n++] = { kind, mode, profile, (uint8_t)(locked ? HOLD_LOCKED : 0),
			(pins != 0) ? HOLD_PINS : HOLD_BUTTONS, { first, second, third }, pins };
	};
	// A hold into the config mode boots the web configurator
	auto addMode = [&](int32_t mode, uint8_t profile, bool locked, uint32_t pins, int16_t action) {
		if (mode == INPUT_MODE_CONFIG)
			add(HOLD_WEB_CONFIGURATOR, VALUE_NONE, profile, locked, pins, action);
		else
			add(HOLD_INPUT_MODE, (uint8_t)mode, profile, locked, pins, action);
	};
	if (boot.enabled) {
		// Pins held exactly. A mask of zero or below is unset, the web
		// configurator storing -1 for no pins in these uint32 fields.
		if ((int32_t)boot.usbModePinMask > 0)
			add(HOLD_UF2_BOOTLOADER, VALUE_NONE, VALUE_NONE, false, boot.usbModePinMask, none);
		if ((int32_t)boot.webConfigPinMask > 0)
			add(HOLD_WEB_CONFIGURATOR, VALUE_NONE, VALUE_NONE, false, boot.webConfigPinMask, none);
		for (size_t i = 0; (i < boot.inputModeMappings_count) && (n < BOOT_HOLDS_MAX); i++) {
			const InputModeMapping& mapping = boot.inputModeMappings[i];
			if (mapping.pinMask > 0)
				addMode(mapping.inputMode, (mapping.profileNumber > 0) ? (uint8_t)mapping.profileNumber : VALUE_NONE,
					false, (uint32_t)mapping.pinMask, none);
		}
		return n;
	}
	const ForcedSetupMode forced = storage.getForcedSetupOptions().mode;
	const bool modeSwitchLocked = (forced == FORCED_SETUP_MODE_LOCK_MODE_SWITCH) || (forced == FORCED_SETUP_MODE_LOCK_BOTH);
	const bool webConfigLocked = (forced == FORCED_SETUP_MODE_LOCK_WEB_CONFIG) || (forced == FORCED_SETUP_MODE_LOCK_BOTH);
	add(HOLD_UF2_BOOTLOADER, VALUE_NONE, VALUE_NONE, false, 0,
		GpioAction::BUTTON_PRESS_S1, GpioAction::BUTTON_PRESS_S2, GpioAction::BUTTON_PRESS_UP);
	add(HOLD_WEB_CONFIGURATOR, VALUE_NONE, VALUE_NONE, webConfigLocked, 0, GpioAction::BUTTON_PRESS_S2);
	const GamepadOptions& gamepad = storage.getGamepadOptions();
	for (const GamepadOptionField& field : gamepadOptionFields) {
		if (isBootButton(field) && ((gamepad.*field.mode) >= 0))
			addMode(gamepad.*field.mode, VALUE_NONE, modeSwitchLocked, 0, field.action);
	}
	return n;
}

} // namespace

uint16_t HostLighting::bootHoldTotal() {
	BootHold holds[BOOT_HOLDS_MAX];
	return bootHolds(holds);
}

void HostLighting::bootHoldRecord(uint16_t index, uint16_t groups, Writer& w) {
	BootHold holds[BOOT_HOLDS_MAX];
	if (index >= bootHolds(holds))
		return;
	const BootHold& hold = holds[index];
	if (groups & (1 << 0)) {
		w.u8(hold.kind);
		w.u8(hold.mode);
		w.u8(hold.profile);
		w.u8(hold.flags);
	}
	if (groups & (1 << 1)) {
		w.u8(hold.source);
		for (int16_t action : hold.actions)
			w.u16((uint16_t)action);
		w.u32(hold.pins);
	}
}

// Page 9 - Profiles. Profile 1 is the base mapping
uint16_t HostLighting::profileTotal() {
	return (uint16_t)(Storage::getInstance().getProfileOptions().gpioMappingsSets_count + 1);
}

// Page 10 - Controls
void HostLighting::controlRecord(uint16_t index, uint16_t groups, Writer& w) {
	uint16_t key = 0;
	if (!controlKey(index, key))
		return;
	GpioAction action = keyAction(key);
	uint16_t instance, instances;
	controlInstance(key, action, instance, instances);

	uint16_t lightTotal = lightCount(), firstLight = INDEX_NONE, count = 0, firstLed = INDEX_NONE;
	uint8_t minX = 255, minY = 255, maxX = 0, maxY = 0;
	for (uint16_t i = 0; i < lightTotal; i++) {
		const Light& light = lights->AllLights[i];
		if (ownerKey(light) != key)
			continue;
		if (count++ == 0) {
			firstLight = i;
			firstLed = (uint16_t)light.FirstLedIndex;
		}
		uint8_t x = gridCoord(light.Position.XPosition), y = gridCoord(light.Position.YPosition);
		minX = std::min(minX, x);
		minY = std::min(minY, y);
		maxX = std::max(maxX, x);
		maxY = std::max(maxY, y);
	}
	if (count == 0)
		minX = minY = 0;

	if (groups & (1 << 0)) {
		w.u8((uint8_t)key);
		w.u16((uint16_t)(int16_t)action);
		w.u8((uint8_t)(((instances > 1) ? 0x01 : 0) | ((count > 0) ? 0x02 : 0)));
	}
	if (groups & (1 << 1)) {
		w.u16(instance);
		w.u16(instances);
	}
	if (groups & (1 << 2)) {
		w.u16(firstLed);
		w.u8(minX);
		w.u8(minY);
		w.u8(maxX);
		w.u8(maxY);
	}
	if (groups & (1 << 3)) {
		w.u16(firstLight);
		w.u16(count);
	}
}

uint16_t HostLighting::controlTotal() { return controlCounts().total; }

// Page 11 - Keyboard mapping. The code is the low byte, as
// KeyboardDriver::pressKey takes it. The flags mark the buttons the keyboard
// interface sends as keys, none while it is not active
uint16_t HostLighting::keyboardMappingTotal() { return KEYBOARD_MAPPING_FIELDS; }

void HostLighting::keyboardMappingRecord(uint16_t index, uint16_t groups, Writer& w) {
	const KeyboardMappingField& field = keyboardMappingFields[index];
	if (groups & (1 << 0)) {
		w.u16((uint16_t)field.action);
		w.u8((uint8_t)(Storage::getInstance().getActiveKeyboardMapping().*field.code));
	}
	if (groups & (1 << 1))
		w.u8(KeyboardInterface::getInstance().sendsKey(field.code) ? KEYBOARD_MAPPING_SENDS_KEY : 0);
}

// Page 12 - Lights
void HostLighting::lightRecord(uint16_t index, uint16_t groups, Writer& w) {
	const Light& light = lights->AllLights[index];
	int pin = ownerKey(light);
	GpioAction action = (pin >= 0) ? keyAction((uint16_t)pin) : GpioAction::NONE;
	uint16_t instance = 0, instances = 0;
	if ((pin >= 0) && isControl(action))
		controlInstance((uint16_t)pin, action, instance, instances);

	if (groups & (1 << 0)) {
		w.u16((uint16_t)light.FirstLedIndex);
		w.u16(light.LedsPerLight);
	}
	if (groups & (1 << 1)) {
		w.u8((uint8_t)light.Type);
		w.u8((instances > 1) ? 0x01 : 0x00);
	}
	if (groups & (1 << 2)) {
		w.u8((pin >= 0) ? (uint8_t)pin : PIN_NONE);
		w.u16((pin >= 0) ? (uint16_t)(int16_t)action : ACTION_NONE);
	}
	if (groups & (1 << 3)) {
		w.u16(instance);
		w.u16(instances);
	}
	if (groups & (1 << 4)) {
		w.u8(gridCoord(light.Position.XPosition));
		w.u8(gridCoord(light.Position.YPosition));
	}
	if (groups & (1 << 5))
		w.u8(((light.NonButtonIndex >= 0) && (light.NonButtonIndex < SLOT_NONE)) ? (uint8_t)light.NonButtonIndex : SLOT_NONE);
}

// Page 8 - Summary. The change counters step in currentToken().
void HostLighting::summaryRecord(uint16_t, uint16_t groups, Writer& w) {
	if (groups & (1 << 0)) {
		ControlCounts counts = controlCounts();
		const DisplayOptions& display = Storage::getInstance().getDisplayOptions();
		w.u16(ledExtent);
		w.u16(LED_LIMIT);
		w.u16(renderRateHz);
		w.u8((uint8_t)Storage::getInstance().getLedOptions().ledFormat);
		w.u8((whiteChain() ? SUMMARY_WHITE_CHANNEL : 0) |
			(PeripheralManager::getInstance().isUSBEnabled(0) ? SUMMARY_USB_HOST : 0));
		w.u8(brightnessMaximum());
		w.u8(brightnessSteps());
		w.u8(speedSteps());
		w.u8((uint8_t)display.buttonLayout);
		w.u8((uint8_t)display.buttonLayoutRight);
		w.u8((uint8_t)display.buttonLayoutOrientation);
		w.u16(gridWidth);
		w.u16(gridHeight);
		w.u16(profileTotal());
		w.u16(counts.total);
		w.u16(counts.lit);
		w.u16((uint16_t)(counts.total - counts.lit));
		w.u16(lightCount());
		w.u16(animationTotal());
	}
	if (groups & (1 << 1))
		w.bytes(pageChanges, sizeof(pageChanges));
}

void HostLighting::profileRecord(uint16_t index, uint16_t groups, Writer& w) {
	if (!(groups & (1 << 0)))
		return;
	Storage& storage = Storage::getInstance();
	const GpioMappings& mapping = (index == 0)
		? storage.getGpioMappings() : storage.getProfileOptions().gpioMappingsSets[index - 1];
	w.u8((uint8_t)(index + 1));
	w.u8(((index == 0) || mapping.enabled) ? 0x01 : 0x00);
	w.text(mapping.profileLabel, PROFILE_LABEL_SIZE);
}

// Page 13 - Animations
void HostLighting::animationRecord(uint16_t index, uint16_t groups, Writer& w) {
	const AnimationProfile& profile = Storage::getInstance().getAnimationOptions().profiles[index];
	if (groups & (1 << 0)) {
		w.u8(profile.bEnabled ? 0x01 : 0x00);
		w.u8((uint8_t)profile.baseNonPressedEffect);
		w.u8((uint8_t)profile.basePressedEffect);
		w.u8((uint8_t)profile.baseCaseEffect);
	}
	if (groups & (1 << 1)) {
		w.u8(speedByte(profile.baseCycleTime));
		w.u8(speedByte(profile.basePressedCycleTime));
		w.u8(speedByte(profile.baseCaseCycleTime));
	}
}

const HostLighting::PageDef HostLighting::pages[] = {
	{ PAGE_STATE, STATE_GROUPS, sizeof(STATE_GROUPS), &HostLighting::oneRecord, &HostLighting::stateRecord },
	{ PAGE_INPUT_MODES, INPUT_MODE_GROUPS, sizeof(INPUT_MODE_GROUPS), &HostLighting::inputModeTotal, &HostLighting::inputModeRecord },
	{ PAGE_NAMES, NAME_GROUPS, sizeof(NAME_GROUPS), &HostLighting::nameTotal, &HostLighting::nameRecord },
	{ PAGE_BOOT_HOLDS, BOOT_HOLD_GROUPS, sizeof(BOOT_HOLD_GROUPS), &HostLighting::bootHoldTotal, &HostLighting::bootHoldRecord },
	{ PAGE_SUMMARY, SUMMARY_GROUPS, sizeof(SUMMARY_GROUPS), &HostLighting::oneRecord, &HostLighting::summaryRecord },
	{ PAGE_PROFILES, PROFILE_GROUPS, sizeof(PROFILE_GROUPS), &HostLighting::profileTotal, &HostLighting::profileRecord },
	{ PAGE_CONTROLS, CONTROL_GROUPS, sizeof(CONTROL_GROUPS), &HostLighting::controlTotal, &HostLighting::controlRecord },
	{ PAGE_KEYBOARD_MAPPING, KEYBOARD_MAPPING_GROUPS, sizeof(KEYBOARD_MAPPING_GROUPS), &HostLighting::keyboardMappingTotal, &HostLighting::keyboardMappingRecord },
	{ PAGE_LIGHTS, LIGHT_GROUPS, sizeof(LIGHT_GROUPS), &HostLighting::lightCount, &HostLighting::lightRecord },
	{ PAGE_ANIMATIONS, ANIMATION_GROUPS, sizeof(ANIMATION_GROUPS), &HostLighting::animationTotal, &HostLighting::animationRecord },
};

const HostLighting::PageDef* HostLighting::findPage(uint8_t number) {
	for (const PageDef& page : pages) {
		if (page.number == number)
			return &page;
	}
	return nullptr;
}

// The animation speeds are on page 13 and change at run time (hotkeys on
// core 1, SET_ANIMATION_SPEED); the token follows them
bool HostLighting::speedsChanged() {
	static_assert(ANIMATION_PROFILES == MAX_ANIMATION_PROFILES, "hashedSpeeds holds every animation profile");
	bool changed = false;
	const AnimationOptions& options = Storage::getInstance().getAnimationOptions();
	for (uint8_t i = 0; i < MAX_ANIMATION_PROFILES; i++) {
		const AnimationProfile& profile = options.profiles[i];
		const int32_t speeds[3] = { profile.baseCycleTime, profile.basePressedCycleTime, profile.baseCaseCycleTime };
		for (uint8_t k = 0; k < 3; k++) {
			if (hashedSpeeds[i][k] != speeds[k]) {
				hashedSpeeds[i][k] = speeds[k];
				changed = true;
			}
		}
	}
	return changed;
}

// State token and change counters. Each configuration page (8-15) is
// hashed, FNV-1a over every record with all groups as the page writers
// produce them (the Summary without its change counters), so a hash covers
// exactly what its page reports. A page with a moved hash steps its change
// counter. The token is the hash of the page hashes, so the same
// configuration gives the same token.
// Recomputed on the next use after a change of the inputs, which are the
// lights and add-on inputs (fixed per boot), the pin map and keyboard mapping
// (profile switch) and the keyboard mapping's codes (SET_KEYBOARD_MAPPING).
uint32_t HostLighting::currentToken() {
	if (speedsChanged())
		tokenStale = true;
	const uint32_t profile = appliedProfile();
	if (profile != hashedProfile) {
		hashedProfile = profile;
		tokenStale = true;
	}
	if (!tokenStale)
		return stateToken;
	tokenStale = false;
	for (const PageDef& page : pages) {
		if ((page.number < PAGE_SUMMARY) || (page.number >= PAGE_SUMMARY + CONFIGURATION_PAGES))
			continue;
		const uint16_t groups = (page.number == PAGE_SUMMARY) ? 1 : (uint16_t)((1u << page.groupCount) - 1);
		uint16_t total = (this->*page.total)();
		uint32_t hash = 2166136261u;
		hash = (hash ^ (total & 0xFF)) * 16777619u;
		hash = (hash ^ (total >> 8)) * 16777619u;
		for (uint16_t i = 0; i < total; i++) {
			uint8_t record[RECORD_AREA];
			Writer w = { record };
			(this->*page.record)(i, groups, w);
			for (const uint8_t* b = record; b < w.p; b++)
				hash = (hash ^ *b) * 16777619u;
		}
		const uint8_t slot = (uint8_t)(page.number - PAGE_SUMMARY);
		if (pagesHashed && (hash != pageHashes[slot]))
			pageChanges[slot]++;
		pageHashes[slot] = hash;
	}
	pagesHashed = true;
	uint32_t hash = 2166136261u;
	for (uint8_t slot = 0; slot < CONFIGURATION_PAGES; slot++) {
		for (uint8_t shift = 0; shift < 32; shift += 8)
			hash = (hash ^ ((pageHashes[slot] >> shift) & 0xFF)) * 16777619u;
	}
	stateToken = (hash != 0) ? hash : 1;
	return stateToken;
}

// A page reply from [3], with the header, records and token
void HostLighting::buildPage(uint8_t* reply, uint8_t number, uint16_t start, uint16_t mask) {
	reply[HEADER_PAGE] = number;
	put16(&reply[HEADER_START], start);
	put32(&reply[HEADER_TOKEN], currentToken());

	if (number == PAGE_IDENTITY) {
		uint16_t wanted = mask ? (mask & ((1u << IDENTITY_GROUPS) - 1)) : ((1u << IDENTITY_GROUPS) - 1);
		put16(&reply[HEADER_TOTAL], 1);
		// Past the one record the groups and stride are echoed as for it,
		// with no record, as on every other page
		uint8_t used = 0;
		uint16_t returned = 0;
		for (uint8_t g = 0; g < IDENTITY_GROUPS; g++) {
			uint8_t width = identityWidth(g);
			if (!(wanted & (1u << g)) || (used + width > RECORD_AREA))
				continue;
			// With its NUL when shorter than the group, cut at 48 otherwise
			if (start == 0)
				memcpy(&reply[HEADER_RECORDS + used], identityStrings[g], width);
			used += width;
			returned |= (uint16_t)(1u << g);
		}
		reply[HEADER_COUNT] = ((start == 0) && returned) ? 1 : 0;
		reply[HEADER_STRIDE] = used;
		put16(&reply[HEADER_GROUPS], returned);
		return;
	}

	const PageDef* page = findPage(number);
	uint16_t defined = (uint16_t)((1u << page->groupCount) - 1);
	uint16_t groups = mask ? (mask & defined) : defined;
	uint8_t stride = 0;
	for (uint8_t g = 0; g < page->groupCount; g++) {
		if (groups & (1u << g))
			stride += page->widths[g];
	}
	uint16_t total = (this->*page->total)();
	put16(&reply[HEADER_TOTAL], total);
	put16(&reply[HEADER_GROUPS], groups);
	reply[HEADER_STRIDE] = stride;
	uint8_t count = 0;
	if (stride != 0) {
		for (uint32_t i = start; (i < total) && (count < RECORD_AREA / stride); i++, count++) {
			Writer w = { &reply[HEADER_RECORDS + count * stride] };
			(this->*page->record)((uint16_t)i, groups, w);
		}
	}
	reply[HEADER_COUNT] = count;
}

// ---------------------------------------------------------------------------
// Replies and events

void HostLighting::clearQueue() {
	queueHead = 0;
	queueCount = 0;
}

// The end of a USB session. Unclaimed and unsubscribed, the default
// CONFIGURE settings and no reply waiting. A claim it ends is a change of
// holder.
void HostLighting::endSession() {
	if (claimToken != 0)
		setHolder(0, nullptr, 0);
	else
		resetControl();
	subscription = 0;
	eventPending = false;
	clearQueue();
	clockRunning = false;
}

// Records those fields as page 1 shows them now; true when any changed
bool HostLighting::noteState() {
	uint8_t profile = activeProfile(appliedProfile());
	uint8_t player = playerNumber();
	uint16_t animation = animationIndex();
	uint8_t brightness = shownBrightness();
	uint8_t saved = savedBrightness();
	bool unsaved = brightnessUnsaved();
	bool idle = animationsIdle();
	uint8_t dpad = dpadMode();
	uint32_t token = currentToken();
	bool changed = (profile != seenProfile) || (player != seenPlayer) || (animation != seenAnimation) ||
		(brightness != seenBrightness) || (saved != seenSaved) || (unsaved != seenUnsaved) || (idle != seenIdle) ||
		(dpad != seenDpadMode) || (token != seenToken);
	seenProfile = profile;
	seenPlayer = player;
	seenAnimation = animation;
	seenBrightness = brightness;
	seenSaved = saved;
	seenUnsaved = unsaved;
	seenIdle = idle;
	seenDpadMode = dpad;
	seenToken = token;
	return changed;
}

// Hands the next reply, else a pending event, to an idle IN endpoint. An
// event is built when it is sent and only while no reply is queued.
void HostLighting::drain() {
	int instance = getHIDInstance();
	if ((instance < 0) || !tud_hid_n_ready((uint8_t)instance))
		return;
	if (queueCount != 0) {
		if (tud_hid_n_report((uint8_t)instance, 0, replyQueue[queueHead], REPORT_SIZE)) {
			queueHead = (uint8_t)((queueHead + 1) % REPLY_QUEUE_SLOTS);
			queueCount--;
		}
		return;
	}
	if (eventPending) {
		uint8_t* event = replyQueue[queueHead];
		memset(event, 0, REPORT_SIZE);
		event[0] = EVENT_MARKER;
		event[1] = 0; // STATE_CHANGED
		buildPage(event, PAGE_STATE, 0, 0);
		if (tud_hid_n_report((uint8_t)instance, 0, event, REPORT_SIZE)) {
			eventPending = false;
			// The event shows page 1 as it is now, so no second event for the same values
			noteState();
		}
	}
}

// ---------------------------------------------------------------------------
// Commands

static uint8_t invalid(uint8_t* reply, uint8_t offset) {
	reply[3] = offset;
	return STATUS_INVALID;
}

// Offset of the first byte of magic that differs, 0 when all match
static uint8_t magicMismatch(const uint8_t* req, const char* magic) {
	for (uint8_t i = 0; i < 4; i++) {
		if (req[4 + i] != (uint8_t)magic[i])
			return (uint8_t)(4 + i);
	}
	return 0;
}

uint8_t HostLighting::cmdHello(const uint8_t*, uint8_t* reply) {
	memcpy(&reply[HELLO_MAGIC], "GPHL", 4);
	reply[HELLO_MAJOR] = PROTOCOL_MAJOR;
	reply[HELLO_MINOR] = PROTOCOL_MINOR;
	pico_unique_board_id_t id;
	pico_get_unique_board_id(&id);
	memcpy(&reply[HELLO_BOARD_ID], id.id, 8);
	reply[HELLO_QUEUE_LIMIT] = REPLY_QUEUE_SLOTS;
	reply[HELLO_FLAGS] = FLAGS_DEFINED;
	for (unsigned c = 1; c < 0x80; c++) {
		if (commandImplemented((uint8_t)c))
			setBit(&reply[HELLO_COMMANDS], c);
	}
	setBit(&reply[HELLO_PAGES], PAGE_IDENTITY);
	for (const PageDef& page : pages)
		setBit(&reply[HELLO_PAGES], page.number);
	put16(&reply[HELLO_ADDRESSING], ADDRESSING_DEFINED);
	put16(&reply[HELLO_PIXEL_FORMATS], (1 << PIXEL_RGB) | (1 << PIXEL_RGBW) | (1 << PIXEL_RGB565));
	put16(&reply[HELLO_EVENTS], EVENTS_DEFINED);
	put16(&reply[HELLO_EVENTS_ENABLED], subscription);
	return STATUS_OK;
}

uint8_t HostLighting::cmdGetPage(const uint8_t* req, uint8_t* reply) {
	uint8_t number = req[GET_PAGE_PAGE];
	if ((number != PAGE_IDENTITY) && (findPage(number) == nullptr))
		return invalid(reply, GET_PAGE_PAGE);
	uint16_t start = get16(&req[GET_PAGE_START]);
	uint32_t expected = get32(&req[GET_PAGE_TOKEN]);
	uint32_t token = currentToken();
	if ((expected != 0) && (expected != token)) {
		reply[HEADER_PAGE] = number;
		put16(&reply[HEADER_START], start);
		put32(&reply[HEADER_TOKEN], token);
		return STATUS_STALE;
	}
	buildPage(reply, number, start, get16(&req[GET_PAGE_MASK]));
	return STATUS_OK;
}

uint8_t HostLighting::cmdClaim(const uint8_t* req, uint8_t* reply) {
	uint8_t action = req[CLAIM_ACTION];
	uint32_t token = get32(&req[CLAIM_TOKEN]);
	const uint8_t* name = &req[CLAIM_NAME];
	uint8_t flags = (uint8_t)(req[CLAIM_FLAGS] & CLAIM_FLAGS_DEFINED);
	if (action > CLAIM_GIVE_UP)
		return invalid(reply, CLAIM_ACTION);
	if (token == 0)
		return invalid(reply, CLAIM_TOKEN);
	bool holder = (claimToken == token);
	if (action == CLAIM_GIVE_UP) {
		if (holder)
			setHolder(0, nullptr, 0);
	} else if (!holder && ((action == CLAIM_TAKE_OVER) || (claimToken == 0))) {
		setHolder(token, name, flags);
	} else if (holder) {
		leaseUs = clockUs;
		if ((memcmp(claimName, name, CLAIM_NAME_SIZE) != 0) || (claimFlags != flags)) {
			memcpy(claimName, name, CLAIM_NAME_SIZE);
			claimFlags = flags;
			raiseEvent();
		}
	}
	reply[CLAIM_ACTION] = action;
	put32(&reply[CLAIM_TOKEN], token);
	putHolder(&reply[CLAIM_HOLDER]);
	return STATUS_OK;
}

uint8_t HostLighting::cmdConfigure(const uint8_t* req, uint8_t* reply) {
	uint8_t mode = req[CONFIGURE_TAKEOVER];
	uint16_t timeout = get16(&req[CONFIGURE_TIMEOUT]);
	uint8_t brightness = req[CONFIGURE_BRIGHTNESS];
	if (mode > TAKEOVER_OVERLAY)
		return invalid(reply, CONFIGURE_TAKEOVER);
	if (brightness > 1)
		return invalid(reply, CONFIGURE_BRIGHTNESS);
	if (timeout == 0)
		timeout = TIMEOUT_DEFAULT_MS;
	timeout = std::clamp(timeout, TIMEOUT_MIN_MS, TIMEOUT_MAX_MS);
	if ((mode != takeoverMode) || (timeout != timeoutMs) || ((brightness != 0) != applyBrightness))
		raiseEvent();
	takeoverMode = mode;
	timeoutMs = timeout;
	applyBrightness = (brightness != 0);
	reply[CONFIGURE_TAKEOVER] = mode;
	put16(&reply[CONFIGURE_TIMEOUT], timeout);
	reply[CONFIGURE_BRIGHTNESS] = brightness;
	return STATUS_OK;
}

// Page 1 fields changed outside the request path and the token (a profile
// switch, a speed change), compared as page 1 shows them. The only detector
// for these, so one change raises one event.
void HostLighting::watchState() {
	// +2 and the token follow the applied profile, so a switch raises one
	// event carrying both
	if (noteState())
		raiseEvent();
}

uint8_t HostLighting::cmdSubscribe(const uint8_t* req, uint8_t* reply) {
	uint16_t added = (uint16_t)(get16(&req[3]) & EVENTS_DEFINED);
	if (subscription == 0 && added != 0)
		noteState();
	subscription |= added;
	put16(&reply[3], subscription);
	return STATUS_OK;
}

// STAGE and UNSTAGE (pixel == nullptr). Everything is validated before
// anything is staged, so an invalid request changes nothing.
uint8_t HostLighting::stageCommand(const uint8_t* req, uint8_t* reply, bool unstage) {
	uint8_t addressing = req[3] & 0x0F;
	uint8_t format = unstage ? 0 : (uint8_t)(req[3] >> 4);
	uint8_t pixelBytes = 0;
	if (!(ADDRESSING_DEFINED & (1u << addressing)))
		return invalid(reply, 3);
	if (!unstage) {
		if (format > PIXEL_RGB565)
			return invalid(reply, 3);
		pixelBytes = PIXEL_BYTES[format];
	}
	const bool white = whiteChain();
	uint32_t outcome = 0;
	uint8_t applied = 0, skipped = 0;

	if ((addressing == ADDRESSING_LED_RUN) || (addressing == ADDRESSING_LIGHT_RUN)) {
		uint16_t first = get16(&req[RUN_FIRST]);
		uint8_t count = req[RUN_COUNT];
		uint8_t capacity = pixelBytes ? (uint8_t)((REPORT_SIZE - RUN_DATA) / pixelBytes) : OUTCOME_BITS;
		capacity = std::min(capacity, OUTCOME_BITS);
		bool ledRun = (addressing == ADDRESSING_LED_RUN);
		if (ledRun && (first >= LED_LIMIT))
			return invalid(reply, RUN_FIRST);
		if ((count == 0) || (count > capacity) || (ledRun && ((uint32_t)first + count > LED_LIMIT)))
			return invalid(reply, RUN_COUNT);
		for (uint8_t n = 0; n < count; n++) {
			uint32_t pixel = 0;
			if (!unstage)
				pixel = decodePixel(&req[RUN_DATA + n * pixelBytes], format, white);
			const uint32_t* value = unstage ? nullptr : &pixel;
			bool ok = true;
			if (ledRun)
				stageLed((uint16_t)(first + n), value);
			else
				ok = ((uint32_t)first + n <= 0xFFFF) && stageLight((uint16_t)(first + n), value);
			if (ok) {
				outcome |= 1u << n;
				applied++;
			} else {
				skipped++;
			}
		}
	} else {
		uint8_t addressBytes = ((addressing == ADDRESSING_PIN) || (addressing == ADDRESSING_KIND)) ? 1 : 2;
		uint8_t n = req[ENTRY_COUNT];
		uint8_t capacity = (uint8_t)((REPORT_SIZE - ENTRY_DATA) / (addressBytes + pixelBytes));
		capacity = std::min(capacity, OUTCOME_BITS);
		if ((n == 0) || (n > capacity))
			return invalid(reply, ENTRY_COUNT);
		const uint16_t total = lightCount();
		for (uint8_t e = 0; e < n; e++) {
			const uint8_t* entry = &req[ENTRY_DATA + e * (addressBytes + pixelBytes)];
			uint16_t address = (addressBytes == 1) ? entry[0] : get16(entry);
			uint32_t pixel = 0;
			if (!unstage)
				pixel = decodePixel(entry + addressBytes, format, white);
			const uint32_t* value = unstage ? nullptr : &pixel;
			bool ok = false;
			if (addressing == ADDRESSING_LIGHT) {
				ok = stageLight(address, value);
			} else {
				for (uint16_t i = 0; i < total; i++) {
					const Light& light = lights->AllLights[i];
					int pin = ownerKey(light);
					bool match;
					if (addressing == ADDRESSING_PIN)
						match = (pin >= 0) && (pin == address);
					else if (addressing == ADDRESSING_ACTION)
						match = (pin >= 0) && ((uint16_t)(int16_t)keyAction((uint16_t)pin) == address);
					else
						match = ((uint8_t)light.Type == address);
					if (match && stageLight(i, value))
						ok = true;
				}
			}
			if (ok) {
				outcome |= 1u << e;
				applied++;
			} else {
				skipped++;
			}
		}
	}
	reply[STAGE_APPLIED] = applied;
	reply[STAGE_SKIPPED] = skipped;
	put32(&reply[STAGE_OUTCOME], outcome);
	return STATUS_OK;
}

uint8_t HostLighting::cmdStage(const uint8_t* req, uint8_t* reply) { return stageCommand(req, reply, false); }
uint8_t HostLighting::cmdUnstage(const uint8_t* req, uint8_t* reply) { return stageCommand(req, reply, true); }

uint8_t HostLighting::cmdFill(const uint8_t* req, uint8_t* reply) {
	uint8_t format = (uint8_t)(req[3] >> 4);
	if (format > PIXEL_RGB565)
		return invalid(reply, 3);
	uint32_t pixel = decodePixel(&req[4], format, whiteChain());
	for (uint16_t led = 0; led < LED_LIMIT; led++)
		stageLed(led, &pixel);
	return STATUS_OK;
}

uint8_t HostLighting::cmdClear(const uint8_t*, uint8_t*) {
	clearStaged();
	return STATUS_OK;
}

uint8_t HostLighting::cmdCommit(const uint8_t*, uint8_t*) {
	publish();
	return STATUS_OK;
}

uint8_t HostLighting::cmdRelease(const uint8_t*, uint8_t*) {
	endTakeover();
	return STATUS_OK;
}

uint8_t HostLighting::cmdSetProfile(const uint8_t* req, uint8_t* reply) {
	Storage& storage = Storage::getInstance();
	if (req[3] == activeProfile(storedProfile()))
		return STATUS_OK;
	if (!storage.setProfile(req[3]))
		return invalid(reply, 3);
	// Saved as the profile hotkeys save. State +2, the pin map, the token and
	// the event follow at the next main-loop pass, after the reinit
	// (GP2040::getReinitGamepad)
	EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
	return STATUS_OK;
}

uint8_t HostLighting::cmdSetAnimation(const uint8_t* req, uint8_t* reply) {
	uint16_t index = get16(&req[3]);
	int8_t value = -1;
	if (index != ANIMATION_OFF) {
		if ((index >= animationTotal()) || !Storage::getInstance().getAnimationOptions().profiles[index].bEnabled)
			return invalid(reply, 3);
		value = (int8_t)index;
	}
	if (value != currentAnimation()) {
		animationRequest = value;
		__mem_fence_release();
		animationRequests = animationRequests + 1;
	}
	return STATUS_OK;
}

uint8_t HostLighting::cmdSetAnimationSpeed(const uint8_t* req, uint8_t* reply) {
	uint16_t index = get16(&req[SET_ANIMATION_SPEED_INDEX]);
	if (index >= animationTotal())
		return invalid(reply, SET_ANIMATION_SPEED_INDEX);
	for (uint8_t at = SET_ANIMATION_SPEED_IDLE; at <= SET_ANIMATION_SPEED_CASE; at++) {
		if ((req[at] != SPEED_UNCHANGED) && (req[at] > speedSteps()))
			return invalid(reply, at);
	}
	// Each speed is an aligned 32-bit store, so core 1 never reads a mixed
	// value. AnimationStation saves them a second later, as for the speed
	// hotkeys.
	AnimationProfile& profile = Storage::getInstance().getAnimationOptions().profiles[index];
	const uint8_t idle = req[SET_ANIMATION_SPEED_IDLE];
	const uint8_t pressed = req[SET_ANIMATION_SPEED_PRESSED];
	const uint8_t caseSpeed = req[SET_ANIMATION_SPEED_CASE];
	bool changed = false;
	if ((idle != SPEED_UNCHANGED) && (profile.baseCycleTime != idle)) {
		profile.baseCycleTime = idle;
		changed = true;
	}
	if ((pressed != SPEED_UNCHANGED) && (profile.basePressedCycleTime != pressed)) {
		profile.basePressedCycleTime = pressed;
		changed = true;
	}
	if ((caseSpeed != SPEED_UNCHANGED) && (profile.baseCaseCycleTime != caseSpeed)) {
		profile.baseCaseCycleTime = caseSpeed;
		changed = true;
	}
	if (changed) {
		__mem_fence_release();
		speedRequests = speedRequests + 1;
	}
	return STATUS_OK;
}

// A saved step is shown and saved (AnimationStation saves it a second
// later); an unsaved step is only shown; 0xFF shows the saved step
uint8_t HostLighting::cmdSetBrightness(const uint8_t* req, uint8_t* reply) {
	uint8_t step = req[SET_BRIGHTNESS_STEP];
	uint8_t save = req[SET_BRIGHTNESS_SAVE];
	if ((step != BRIGHTNESS_SAVED) && (step > brightnessSteps()))
		return invalid(reply, SET_BRIGHTNESS_STEP);
	if (save > 1)
		return invalid(reply, SET_BRIGHTNESS_SAVE);
	bool saving = (save != 0) && (step != BRIGHTNESS_SAVED);
	uint8_t saved = saving ? step : savedBrightness();
	uint8_t shown = (step == BRIGHTNESS_SAVED) ? saved : step;
	if ((saved != savedBrightness()) || (shown != shownBrightness()))
		postBrightness(saving ? step : pendingSave(), step);
	// Each SET_BRIGHTNESS that shows a step without saving it starts its
	// keepalive, also when the step equals the saved one, since a hotkey can
	// change the saved step before core 1 shows it
	if (!saving && (step != BRIGHTNESS_SAVED)) {
		unsavedUs = clockUs;
		unsavedStep = true;
		clockRunning = true;
	}
	return STATUS_OK;
}

// Codes of the active profile's keyboard mapping, live (KeyboardDriver reads
// it for every report) and saved as the profile hotkeys save. A code equal
// to the one page 11 shows is left as stored. The command names the profile
// it expects, as page 1 shows it, and is refused as stale under another, and
// until a profile switch applies (GP2040::getReinitGamepad), when the active
// mapping is still the old profile's.
uint8_t HostLighting::cmdSetKeyboardMapping(const uint8_t* req, uint8_t* reply) {
	static_assert(SET_KEYBOARD_MAPPING_CODES + KEYBOARD_MAPPING_FIELDS <= REPORT_SIZE,
		"SET_KEYBOARD_MAPPING carries the whole mapping in one report");
	const uint16_t first = get16(&req[SET_KEYBOARD_MAPPING_FIRST]);
	const uint8_t count = req[SET_KEYBOARD_MAPPING_COUNT];
	if (first >= KEYBOARD_MAPPING_FIELDS)
		return invalid(reply, SET_KEYBOARD_MAPPING_FIRST);
	if ((count == 0) || (count > (uint16_t)(KEYBOARD_MAPPING_FIELDS - first)))
		return invalid(reply, SET_KEYBOARD_MAPPING_COUNT);
	const uint32_t applied = appliedProfile();
	if ((storedProfile() != applied) || (req[SET_KEYBOARD_MAPPING_PROFILE] != activeProfile(applied)))
		return STATUS_STALE;
	KeyboardMapping& mapping = Storage::getInstance().getActiveKeyboardMapping();
	bool changed = false;
	for (uint8_t i = 0; i < count; i++) {
		uint32_t& code = mapping.*keyboardMappingFields[first + i].code;
		const uint8_t value = req[SET_KEYBOARD_MAPPING_CODES + i];
		if ((uint8_t)code != value) {
			code = value;
			changed = true;
		}
	}
	if (changed) {
		tokenStale = true;
		EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
	}
	return STATUS_OK;
}

uint8_t HostLighting::cmdSetInputMode(const uint8_t* req, uint8_t* reply) {
	InputMode mode = (InputMode)req[3];
	const InputModeAttributes* attributes = inputModeAttributes(mode);
	if ((attributes == nullptr) || !acceptedInputMode(*attributes))
		return invalid(reply, 3);
	uint8_t bad = magicMismatch(req, "MODE");
	if (bad)
		return invalid(reply, bad);
	// Saved and rebooted from the main loop, after the reply has been queued
	GamepadOptions& gamepadOptions = Storage::getInstance().getGamepadOptions();
	if (gamepadOptions.inputMode != mode) {
		gamepadOptions.inputMode = mode;
		EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true, true));
	} else {
		EventManager::getInstance().triggerEvent(new GPRestartEvent(System::BootMode::GAMEPAD));
	}
	return STATUS_OK;
}

uint8_t HostLighting::cmdReboot(const uint8_t* req, uint8_t* reply) {
	System::BootMode bootMode;
	if (!rebootModeToBootMode(req[3], bootMode))
		return invalid(reply, 3);
	uint8_t bad = magicMismatch(req, "BOOT");
	if (bad)
		return invalid(reply, bad);
	EventManager::getInstance().triggerEvent(new GPRestartEvent(bootMode));
	return STATUS_OK;
}

const HostLighting::Command HostLighting::commands[] = {
	{ CMD_HELLO, &HostLighting::cmdHello },
	{ CMD_GET_PAGE, &HostLighting::cmdGetPage },
	{ CMD_CLAIM, &HostLighting::cmdClaim },
	{ CMD_CONFIGURE, &HostLighting::cmdConfigure },
	{ CMD_SUBSCRIBE, &HostLighting::cmdSubscribe },
	{ CMD_STAGE, &HostLighting::cmdStage },
	{ CMD_UNSTAGE, &HostLighting::cmdUnstage },
	{ CMD_FILL, &HostLighting::cmdFill },
	{ CMD_CLEAR, &HostLighting::cmdClear },
	{ CMD_COMMIT, &HostLighting::cmdCommit },
	{ CMD_RELEASE, &HostLighting::cmdRelease },
	{ CMD_SET_PROFILE, &HostLighting::cmdSetProfile },
	{ CMD_SET_ANIMATION, &HostLighting::cmdSetAnimation },
	{ CMD_SET_ANIMATION_SPEED, &HostLighting::cmdSetAnimationSpeed },
	{ CMD_SET_BRIGHTNESS, &HostLighting::cmdSetBrightness },
	{ CMD_SET_KEYBOARD_MAPPING, &HostLighting::cmdSetKeyboardMapping },
	{ CMD_SET_INPUT_MODE, &HostLighting::cmdSetInputMode },
	{ CMD_REBOOT, &HostLighting::cmdReboot },
};

// HELLO lists, and the dispatch runs, only the commands found here.
// SET_KEYBOARD_MAPPING needs HostLightingOptions.allowKeyboardMappingChanges.
const HostLighting::Command* HostLighting::findCommand(uint8_t id) {
	if ((id == CMD_SET_KEYBOARD_MAPPING) &&
		!Storage::getInstance().getAddonOptions().hostLightingOptions.allowKeyboardMappingChanges)
		return nullptr;
	for (const Command& command : commands) {
		if (command.id == id)
			return &command;
	}
	return nullptr;
}

bool HostLighting::commandImplemented(uint8_t command) { return findCommand(command) != nullptr; }

// A bus reset recorded by usbEvent() since the last call
bool HostLighting::busResetSeen() {
	if (!busResetPending)
		return false;
	busResetPending = false;
	return true;
}

// Session upkeep before each request and in each main-loop pass. A bus reset
// is the one sign of a new host the board gets while it keeps power, so it
// drops the replies still queued and ends a claim, as a change of holder.
// The subscription stays. A host with a handle that outlives the reset (a Linux
// reset-resume) would lose it unseen, while it sees a lost claim at once.
// Not inlined, so process() and setReport() share one copy in RAM.
void __no_inline_not_in_flash_func(HostLighting::upkeep)() {
	if (busResetSeen()) {
		clearQueue();
		if (claimToken != 0)
			setHolder(0, nullptr, 0);
	}
	checkExpiry();
}

void HostLighting::setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t* buffer, uint16_t bufsize) {
	(void)report_id;
	// OUT endpoint data and SET_REPORT control requests
	if ((report_type != HID_REPORT_TYPE_INVALID) && (report_type != HID_REPORT_TYPE_OUTPUT))
		return;
	if (bufsize == 0)
		return;

	// Bytes past a short report read as zero
	uint8_t req[REPORT_SIZE] = {};
	memcpy(req, buffer, std::min<uint16_t>(bufsize, REPORT_SIZE));

	// An expired takeover or lease ends before this request is acted on
	upkeep();

	// No command 0x00, and bit 7 marks replies and events
	const uint8_t id = req[0];
	if ((id == 0) || (id & REPLY_FLAG))
		return;
	const uint8_t flags = req[2];
	const bool answer = !(flags & FLAG_NO_REPLY);
	// A request that needs a reply with the queue full is not executed
	if (answer && (queueCount == REPLY_QUEUE_SLOTS))
		return;

	uint8_t discard[REPORT_SIZE];
	uint8_t* reply = answer ? replyQueue[(queueHead + queueCount) % REPLY_QUEUE_SLOTS] : discard;
	memset(reply, 0, REPORT_SIZE);
	reply[0] = (uint8_t)(id | REPLY_FLAG);
	reply[1] = req[1];

	// A request other than CLAIM under a generation that is no longer current
	// is refused, as the holder has changed since its sender looked. An exclusive
	// claim refuses a request without HOLDER and the current generation, but
	// CLAIM, and HELLO, GET_PAGE and SUBSCRIBE without COMMIT_AFTER or
	// KEEPALIVE. A CLAIM has its flags act only when it leaves the sender
	// holding the claim, under either claim.
	const uint8_t generation = (uint8_t)((flags & FLAG_GENERATION) >> FLAG_GENERATION_SHIFT);
	const bool outdated = (id != CMD_CLAIM) && (generation != 0) && (generation != claimGeneration);
	const bool guarded = (claimToken != 0) && (claimFlags & CLAIM_EXCLUSIVE) &&
		!((flags & FLAG_HOLDER) && (generation == claimGeneration));
	bool open = false;
	for (uint8_t allowed : EXCLUSIVE_OPEN_COMMANDS)
		open = open || (id == allowed);
	bool openUnflagged = false;
	for (uint8_t allowed : EXCLUSIVE_OPEN_UNFLAGGED)
		openUnflagged = openUnflagged || (id == allowed);
	const Command* command = findCommand(id);
	uint8_t status;
	if (command == nullptr) {
		status = STATUS_UNSUPPORTED;
	} else if (outdated || (guarded && !open && (!openUnflagged || (flags & EXCLUSIVE_REFUSED_FLAGS)))) {
		putHolder(&reply[CLAIMED_HOLDER]);
		status = STATUS_CLAIMED;
	} else {
		status = (this->*command->run)(req, reply);
	}
	reply[2] = status;

	const bool flagsAct = (id == CMD_CLAIM) ? (claimToken == get32(&req[CLAIM_TOKEN])) : !guarded;
	if ((status == STATUS_OK) && flagsAct) {
		if (flags & FLAG_COMMIT_AFTER)
			publish();
		// Refreshes a live takeover or unsaved step only. KEEPALIVE never
		// starts one
		bool refresh = ((id >= CMD_KEEPALIVE_FIRST) && (id <= CMD_KEEPALIVE_LAST)) ||
			(flags & (FLAG_COMMIT_AFTER | FLAG_KEEPALIVE));
		if (refresh && liveActive)
			keepaliveUs = clockUs;
		if (refresh && brightnessUnsaved())
			unsavedUs = clockUs;
	}

	if (answer) {
		queueCount++;
		drain();
	}
}

uint16_t HostLighting::getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
	// Replies travel on the IN endpoint only; GET_REPORT is not served
	(void)report_id;
	(void)report_type;
	(void)buffer;
	(void)reqlen;
	return 0;
}

const uint8_t* HostLighting::getReportDescriptor() {
	return hostlighting_report_descriptor;
}

uint16_t HostLighting::getReportDescriptorLength() {
	return sizeof(hostlighting_report_descriptor);
}

// ---------------------------------------------------------------------------
// Core 1

void HostLighting::registerLights(const Lights& layout, uint16_t rateHz) {
	uint16_t extent = 0, width = 0, height = 0;
	uint64_t pins = 0;
	uint32_t triggers = 0;
	for (const Light& light : layout.AllLights) {
		uint16_t first, end;
		if (lightLeds(light, first, end))
			extent = std::max(extent, end);
		uint16_t x = (uint16_t)(gridCoord(light.Position.XPosition) + 1);
		uint16_t y = (uint16_t)(gridCoord(light.Position.YPosition) + 1);
		width = std::max(width, x);
		height = std::max(height, y);
		// From the light itself, as scannedTriggers is found on core 0
		if (light.InputKey < 0)
			continue;
		if (light.InputSource == LightInputSource::LightInputSource_HallEffect)
			triggers |= (light.InputIndex < 32) ? (1u << light.InputIndex) : 0;
		else if (light.InputIndex < NUM_BANK0_GPIOS)
			pins |= 1ull << light.InputIndex;
	}
	ledExtent = extent;
	gridWidth = width;
	gridHeight = height;
	litPins = pins;
	litTriggers = triggers;
	renderRateHz = rateHz;
	tokenStale = true;
	__mem_fence_release();
	lights = &layout;
	ledsActive = true;
}

void HostLighting::render(AnimationStation& station, uint32_t* frame, int format) {
	static_assert(LED_LIMIT == FRAME_MAX, "the frame buffers hold the render pipeline's frame");
	// Host requests. Core 1 owns the animations. Applied as the hotkeys
	// apply them; OptionsChanged() has AnimationStation save a change a
	// second later. An animation picked wakes them from the idle timeout
	// first, so SetMode() builds them awake; off leaves them idle.
	uint32_t requested = animationRequests;
	if (requested != animationApplied) {
		__mem_fence_acquire();
		int8_t value = animationRequest;
		animationApplied = requested;
		if (value >= 0)
			station.WakeFromIdle();
		station.SetMode(value);
		station.OptionsChanged();
	}
	// Only a step to save reaches options.brightness, the value
	// AnimationStation saves. Marked applied after it applies, so core 0
	// never reads the old steps as current.
	requested = brightnessRequests;
	if (requested != brightnessApplied) {
		__mem_fence_acquire();
		uint16_t request = brightnessRequest;
		uint8_t save = (uint8_t)(request >> 8);
		uint8_t show = (uint8_t)request;
		AnimationOptions& options = Storage::getInstance().getAnimationOptions();
		if (save != BRIGHTNESS_SAVED) {
			options.brightness = save;
			station.OptionsChanged();
		}
		station.SetBrightnessStepValue((show == BRIGHTNESS_SAVED) ? (uint8_t)options.brightness : show);
		__mem_fence_release();
		brightnessApplied = requested;
	}
	requested = speedRequests;
	if (requested != speedApplied) {
		__mem_fence_acquire();
		speedApplied = requested;
		station.CycleParameterChange();
		station.OptionsChanged();
	}

	if (!liveActive)
		return;
	// CONFIGURE can change these mid-pass; one read each keeps the frame whole
	const uint8_t mode = takeoverMode;
	const bool scaled = applyBrightness;

	// Scale table built with the animations' own expression (RGB::value)
	float factor = scaled ? station.GetNormalisedBrightness() : 1.0f;
	uint32_t factorBits;
	memcpy(&factorBits, &factor, sizeof(factorBits));
	bool rescaled = (factorBits != scaleBits);
	if (rescaled) {
		for (unsigned c = 0; c < 256; c++)
			scale[c] = (uint8_t)(uint32_t)(c * factor);
		scaleBits = factorBits;
	}

	// Converted frame, rebuilt when the host publishes or the scale or
	// format changes; the steady state is a copy. The channel packing must
	// stay in step with RGB::value() (animation.h); a stored grey on a white
	// chain is already W alone (decodePixel).
	const uint16_t extent = ledExtent;
	if (rescaled || (liveSeq != convertedSeq) || (format != convertedFormat)) {
		uint32_t before, after;
		do {
			before = liveSeq;
			__mem_fence_acquire();
			for (uint16_t i = 0; i < extent; i++) {
				uint32_t p = livePixels[i];
				uint32_t r = scale[(p >> 16) & 0xFF], g = scale[(p >> 8) & 0xFF];
				uint32_t b = scale[p & 0xFF], w = scale[p >> 24];
				switch (format) {
					case LED_FORMAT_GRB:  converted[i] = (g << 16) | (r << 8) | b; break;
					case LED_FORMAT_RGB:  converted[i] = (r << 16) | (g << 8) | b; break;
					case LED_FORMAT_GRBW: converted[i] = (g << 24) | (r << 16) | (b << 8) | w; break;
					default:              converted[i] = (r << 24) | (g << 16) | (b << 8) | w; break;
				}
			}
			memcpy(convertedBits, liveBits, sizeof(convertedBits));
			__mem_fence_acquire();
			after = liveSeq;
		} while ((before != after) || (before & 1));
		convertedSeq = after;
		convertedFormat = format;
	}

	if (mode == TAKEOVER_OVERLAY) {
		for (uint16_t i = 0; i < extent; i++) {
			if (convertedBits[i / 32] & (1u << (i % 32)))
				frame[i] = converted[i];
		}
	} else {
		memcpy(frame, converted, extent * sizeof(uint32_t));
	}
}

// ---------------------------------------------------------------------------
// USB function

USBFunctionSetting HostLighting::setting(InputMode mode) {
	const HostLightingOptions& options = Storage::getInstance().getAddonOptions().hostLightingOptions;
	const InputModeAttributes* attributes = inputModeAttributes(mode);
	if (!options.enabled || (attributes == nullptr))
		return USBFunctionSetting::OFF;
	if (attributes->lighting == INPUT_MODE_LIGHTING_ALWAYS)
		return USBFunctionSetting::ON;
	if (attributes->lighting != INPUT_MODE_LIGHTING_SETTING)
		return USBFunctionSetting::OFF;
	switch (options.xinputMode) {
		case HOST_LIGHTING_XINPUT_MODE_ON:
			return USBFunctionSetting::ON;
		case HOST_LIGHTING_XINPUT_MODE_AUTO:
			return USBFunctionSetting::AUTO;
		default:
			return USBFunctionSetting::OFF;
	}
}

// ---------------------------------------------------------------------------
// USB session (core 0, except usbEvent)

void HostLighting::reportComplete() {
	drain();
}

// From tud_event_hook_cb, in interrupt context. Sets flags only
void HostLighting::usbEvent(uint32_t eventId) {
	if (eventId == DCD_EVENT_BUS_RESET)
		busResetPending = true;
}

// TinyUSB reports an unmount only for SET_CONFIGURATION 0 on the RP2040,
// which forces VBUS detection. An unplugged bus-powered board loses power,
// and behind a powered hub or switch the board is suspended, then reset at
// the next plug-in. The takeover ends at the suspend, and the reset ends
// the claim and drops the replies still queued (upkeep()).
void HostLighting::usbUnmounted() {
	endSession();
}

void HostLighting::usbSuspended() {
	// The event, if subscribed, waits for resume
	endTakeover();
	restoreBrightness();
}

void HostLighting::setup(bool expanderInputs) {
	scannedTriggers = triggersScanned();
	expanderKeys = expanderTotal(expanderInputs);
}

void __not_in_flash_func(HostLighting::process)() {
	if (busResetPending || clockRunning)
		upkeep();
	if (subscription != 0) {
		uint64_t now = getMicro();
		if ((now - watchedUs) >= WATCH_INTERVAL_US) {
			watchedUs = now;
			watchState();
		}
	}
	if ((queueCount != 0) || eventPending)
		drain();
}
