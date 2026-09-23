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
#include "device/dcd.h"

#include "BoardConfig.h"
#include "drivermanager.h"
#include "eventmanager.h"
#include "helper.h"
#include "rebootmodes.h"
#include "storagemanager.h"
#include "system.h"
#include "usbdriver.h"
#include "version.h"
#include "animationstation.h"
#include "pixel.h"
#include "NeoPico.h"
#include "inputmodenames.h"

// Watchdog scratch register holding the XInput AUTO host verdict (scratch[5]
// belongs to System::reboot's BootMode)
#define HOST_LIGHTING_VERDICT_SCRATCH   6
#define HOST_LIGHTING_VERDICT_PC_HOST   0x484C5043 // "HLPC"
#define HOST_LIGHTING_XINPUT_DETECT_MS  4000

namespace {

// Wire constants, in the order of the tables in docs/host-lighting.md.

constexpr uint8_t PROTOCOL_MAJOR = 2;
constexpr uint8_t PROTOCOL_MINOR = 0;
constexpr uint8_t REPORT_SIZE = 64;
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
constexpr uint8_t STATUS_STALE_TOKEN = 3;
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

// Replies are queued until the IN endpoint takes them. The depth is on the
// wire (HELLO [17]), and hosts rely on it.
constexpr uint8_t REPLY_QUEUE_SLOTS = 16;

// CLAIM request and reply; the reply echoes the action and token, then
// shows the holder as page 1's Controller group, as a status 4 reply does
constexpr uint8_t CLAIM_ACTION = 3;
constexpr uint8_t CLAIM_TOKEN = 4;
constexpr uint8_t CLAIM_NAME = 8;
constexpr uint8_t CLAIM_FLAGS = 24;
constexpr uint8_t CLAIM_HOLDER = 8;
constexpr uint8_t CLAIMED_HOLDER = 3;
constexpr uint8_t CLAIM_NAME_SIZE = 16;
constexpr uint8_t CONTROLLER_SIZE = 22;
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
constexpr uint8_t TAKEOVER_WHOLE_FRAME = 0;
constexpr uint8_t TAKEOVER_OVERLAY = 1;
constexpr uint16_t TIMEOUT_DEFAULT_MS = 2000;
constexpr uint16_t TIMEOUT_MIN_MS = 100;
constexpr uint16_t TIMEOUT_MAX_MS = 10000;

// Pages
constexpr uint8_t PAGE_IDENTITY = 0;
constexpr uint8_t PAGE_STATE = 1;
constexpr uint8_t PAGE_INPUT_MODES = 2;
constexpr uint8_t PAGE_NAMES = 3;
constexpr uint8_t PAGE_SUMMARY = 8;
constexpr uint8_t PAGE_PROFILES = 9;
constexpr uint8_t PAGE_CONTROLS = 10;
constexpr uint8_t PAGE_LIGHTS = 11;
constexpr uint8_t PAGE_ANIMATIONS = 12;

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
constexpr uint8_t STATE_GROUPS[] = { 9, CONTROLLER_SIZE, 4 };
constexpr uint8_t INPUT_MODE_GROUPS[] = { 2, 22 };
constexpr uint8_t NAME_GROUPS[] = { 3, 40 };
constexpr uint8_t SUMMARY_GROUPS[] = { 30, 8 };
// Configuration pages 8 to 15, with one change counter each in the Summary
constexpr uint8_t CONFIGURATION_PAGES = 8;
constexpr uint8_t PROFILE_GROUPS[] = { 18 };
constexpr uint8_t CONTROL_GROUPS[] = { 4, 4, 6, 4 };
constexpr uint8_t LIGHT_GROUPS[] = { 4, 2, 3, 4, 2, 1 };
constexpr uint8_t ANIMATION_GROUPS[] = { 4, 3 };
constexpr uint8_t INPUT_MODE_NAME_SIZE = 22;
constexpr uint8_t NAME_SIZE = 40;
// Page 3 enums
constexpr uint8_t NAMES_GPIO_ACTION = 0;
constexpr uint8_t NAMES_LIGHT_TYPE = 1;
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

// Page 1 State flags
constexpr uint8_t STATE_LIVE = 1 << 0;
constexpr uint8_t STATE_UNSAVED = 1 << 1;
constexpr uint8_t STATE_IDLE = 1 << 2;

// Page 8 Summary flags
constexpr uint8_t SUMMARY_WHITE_CHANNEL = 1 << 0;

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
constexpr uint16_t ANIMATION_OFF = 0xFFFF;

// Addressable LED limit (Summary +2..3), the render pipeline's frame.
// Every frame buffer here is sized from it.
constexpr uint16_t LED_LIMIT = FRAME_MAX;

// A gap this long between two clock checks is a core-0 stall (a settings
// flash write stops core 0 and core 1 for hundreds of milliseconds); it is
// not counted toward the keepalive or the lease.
constexpr uint32_t STALL_US = 20000;

// While subscribed, page 1 fields changed outside the request path are
// compared at this interval, so their event follows within it
constexpr uint32_t WATCH_INTERVAL_US = 10000;

inline void put16(uint8_t * p, uint16_t v) { p[0] = (uint8_t)v; p[1] = (uint8_t)(v >> 8); }
inline void put32(uint8_t * p, uint32_t v) { put16(p, (uint16_t)v); put16(p + 2, (uint16_t)(v >> 16)); }
inline uint16_t get16(const uint8_t * p) { return (uint16_t)(p[0] | (p[1] << 8)); }
inline uint32_t get32(const uint8_t * p) { return get16(p) | ((uint32_t)get16(p + 2) << 16); }
inline void setBit(uint8_t * bitmap, unsigned bit) { bitmap[bit / 8] |= (uint8_t)(1u << (bit % 8)); }

uint32_t nowMs() { return to_ms_since_boot(get_absolute_time()); }

// One timer register read; differences stay valid across the 71-minute wrap
// for any gap shorter than half of it
uint32_t nowUs() { return time_us_32(); }

} // namespace

// ---------------------------------------------------------------------------
// Interface this boot

// Input mode, settings and the XInput verdict are fixed per boot
static int lightingInstance() {
	static int instance = -2;
	if (instance == -2) {
		DriverManager & drivers = DriverManager::getInstance();
		InputMode mode = drivers.getInputMode();
		instance = (!drivers.isConfigMode() && HostLighting::enabledForMode(mode))
			? ((mode == INPUT_MODE_XINPUT) ? 0 : HOST_LIGHTING_HID_INSTANCE) : -1;
	}
	return instance;
}

// ---------------------------------------------------------------------------
// Lights

// The LED add-on's layout. Written on core 1 in LED setup, which finishes
// before core 0 calls tud_init(), and again only on an LED restart, which
// only the web configurator requests (config mode, no lighting interface).
// It is therefore fixed while the interface exists and read without a lock.
static const Lights * lights = nullptr;
static uint16_t ledExtent = 0;
static uint16_t gridWidth = 0;
static uint16_t gridHeight = 0;
static uint64_t litPins = 0;
static uint32_t litTriggers = 0;
static uint16_t renderRateHz = 0;

static uint16_t lightCount() {
	if (lights == nullptr)
		return 0;
	size_t n = lights->AllLights.size();
	return (n > 0xFFFF) ? 0xFFFF : (uint16_t)n;
}

static uint8_t gridCoord(int v) {
	return (uint8_t)((v < 0) ? 0 : ((v > 255) ? 255 : v));
}

// LEDs of a light below the limit as [first, end), false when none
static bool lightLeds(const Light & light, uint16_t & first, uint16_t & end) {
	if (light.FirstLedIndex >= LED_LIMIT)
		return false;
	uint32_t last = light.FirstLedIndex + light.LedsPerLight;
	first = (uint16_t)light.FirstLedIndex;
	end = (uint16_t)((last > LED_LIMIT) ? LED_LIMIT : last);
	return end > first;
}

// ---------------------------------------------------------------------------
// Frames

// Stored pixels are 0xWWRRGGBB. The staged frame is core 0 only.
static uint32_t stagedPixels[LED_LIMIT] = {};
static uint32_t stagedBits[(LED_LIMIT + 31) / 32] = {};

// Live frame for core 1, under a sequence lock. Core 0 makes liveSeq odd,
// writes, makes it even; core 1 retries its copy until it reads the same
// even value before and after.
static volatile uint32_t liveSeq = 0;
static uint32_t livePixels[LED_LIMIT] = {};
static uint32_t liveBits[(LED_LIMIT + 31) / 32] = {};
static volatile bool liveActive = false;
static volatile uint8_t takeoverMode = TAKEOVER_WHOLE_FRAME;
static volatile bool applyBrightness = true;

static bool whiteChain() {
	LEDFormat_Proto format = Storage::getInstance().getLedOptions().ledFormat;
	return (format == LEDFormat_Proto_LED_FORMAT_GRBW) || (format == LEDFormat_Proto_LED_FORMAT_RGBW);
}

// A wire pixel as stored. On a chain with a white emitter an RGB or RGB565
// grey is stored as W alone, as RGB::value() maps it for the animations.
static uint32_t decodePixel(const uint8_t * p, uint8_t format, bool white) {
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

static void stageLed(uint16_t led, const uint32_t * pixel) {
	if (pixel != nullptr) {
		stagedPixels[led] = *pixel;
		stagedBits[led / 32] |= 1u << (led % 32);
	} else {
		stagedPixels[led] = 0;
		stagedBits[led / 32] &= ~(1u << (led % 32));
	}
}

// Every LED of the light below the limit; false when it has none
static bool stageLight(uint16_t ordinal, const uint32_t * pixel) {
	uint16_t first, end;
	if ((ordinal >= lightCount()) || !lightLeds(lights->AllLights[ordinal], first, end))
		return false;
	for (uint16_t led = first; led < end; led++)
		stageLed(led, pixel);
	return true;
}

static void clearStaged() {
	memset(stagedPixels, 0, sizeof(stagedPixels));
	memset(stagedBits, 0, sizeof(stagedBits));
}

// ---------------------------------------------------------------------------
// Session takeover, keepalive, claim and subscription (core 0)

static uint16_t timeoutMs = TIMEOUT_DEFAULT_MS;
static uint8_t takeoverNumber = 0;
static uint32_t keepaliveUs = 0;   // last keepalive refresh
static uint32_t leaseUs = 0;       // last claim renewal
static uint32_t unsavedUs = 0;     // last unsaved brightness keepalive refresh
static uint32_t clockUs = 0;       // last clock check
static bool clockRunning = false;  // a takeover, claim or unsaved step was live then
static uint32_t claimToken = 0;
static uint8_t claimName[CLAIM_NAME_SIZE] = {};
static uint8_t claimFlags = 0;
static uint8_t claimGeneration = 1;
static uint16_t subscription = 0;
static bool eventPending = false;

// Unsaved brightness step (defined with the core 1 requests below)
static bool brightnessUnsaved();
static void restoreBrightness();

static void raiseEvent() {
	if (subscription & EVENT_STATE_CHANGED)
		eventPending = true;
}

// Every end of a takeover clears the staged frame, as CLEAR does
static void endTakeover() {
	clearStaged();
	if (liveActive) {
		liveActive = false;
		raiseEvent();
	}
}

static void publish() {
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

// Page 1's Controller group; the CLAIM reply and a status 4 reply carry it
static void putController(uint8_t * at) {
	put32(at, claimToken);
	memcpy(at + 4, claimName, CLAIM_NAME_SIZE);
	at[4 + CLAIM_NAME_SIZE] = claimFlags;
	at[5 + CLAIM_NAME_SIZE] = claimGeneration;
}

// Ends the takeover and an unsaved brightness step, clears the staged frame
// and returns the CONFIGURE settings to their defaults
static void resetControl() {
	endTakeover();
	restoreBrightness();
	takeoverMode = TAKEOVER_WHOLE_FRAME;
	timeoutMs = TIMEOUT_DEFAULT_MS;
	applyBrightness = true;
}

// Every change of holder resets the control state and steps the generation
static void setHolder(uint32_t token, const uint8_t * name, uint8_t flags) {
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
static void checkExpiry() {
	uint32_t now = nowUs();
	if (clockRunning) {
		uint32_t gap = now - clockUs;
		if (gap > STALL_US) {
			keepaliveUs += gap;
			leaseUs += gap;
			unsavedUs += gap;
		}
	}
	clockUs = now;
	const int32_t timeout = (int32_t)timeoutMs * 1000;
	if (liveActive && ((int32_t)(now - keepaliveUs) > timeout))
		endTakeover();
	if ((claimToken != 0) && ((int32_t)(now - leaseUs) > timeout))
		setHolder(0, nullptr, 0);
	bool unsaved = brightnessUnsaved();
	if (unsaved && ((int32_t)(now - unsavedUs) > timeout)) {
		restoreBrightness();
		unsaved = false;
	}
	clockRunning = liveActive || (claimToken != 0) || unsaved;
}

// ---------------------------------------------------------------------------
// Host requests to core 1, which owns the animations. A request is pending
// while its count differs from the applied count; the value is the last
// one written.

static volatile int8_t animationRequest = 0;
static volatile uint32_t animationRequests = 0;
static volatile uint32_t animationApplied = 0;
// A brightness request packs a step to save (BRIGHTNESS_SAVED for none) in
// the high byte and a step to show (BRIGHTNESS_SAVED for the saved one) in the
// low byte, so one store carries both. Core 1 resolves the saved step against
// its own, so a hotkey between request and render is not undone.
static volatile uint16_t brightnessRequest = 0;
static volatile uint32_t brightnessRequests = 0;
static volatile uint32_t brightnessApplied = 0;
static volatile bool ledsActive = false;

// Without the LED add-on (no LED output) nothing animates, with no
// animations, no brightness steps, animation off and step 0. SET_ANIMATION
// then accepts only off and SET_BRIGHTNESS only step 0 or 0xFF (the saved
// step); neither changes anything.
// AnimationStation and the web configurator run every profile, enabled or
// not.
static uint16_t animationTotal() {
	return ledsActive ? MAX_ANIMATION_PROFILES : 0;
}

static uint8_t brightnessSteps() {
	return ledsActive ? AnimationStation::getInstance().getBrightnessSteps() : 0;
}

static uint8_t brightnessMaximum() {
	return ledsActive ? AnimationStation::getInstance().GetMaxBrightness() : 0;
}

// Animation speed steps. The speed hotkeys step 0 to CYCLE_STEPS - 1, the
// web configurator 1 to CYCLE_STEPS
static uint8_t speedSteps() {
	return ledsActive ? CYCLE_STEPS : 0;
}

static uint8_t speedByte(int32_t speed) {
	return (uint8_t)((speed < 0) ? 0 : ((speed > 255) ? 255 : speed));
}

// Speeds changed on core 0 by SET_ANIMATION_SPEED; core 1 re-reads the
// running animations' speeds when the count moves
static volatile uint32_t speedRequests = 0;
static uint32_t speedApplied = 0;

// The animation the board shows and the brightness steps saved and shown,
// a pending request included
static int8_t currentAnimation() {
	if (!ledsActive)
		return -1;
	return (animationRequests != animationApplied) ? animationRequest : (int8_t)Storage::getInstance().getAnimationOptions().baseProfileIndex;
}

// A step to save that core 1 has not applied yet, else BRIGHTNESS_SAVED
static uint8_t pendingSave() {
	return (brightnessRequests != brightnessApplied) ? (uint8_t)(brightnessRequest >> 8) : BRIGHTNESS_SAVED;
}

static uint8_t savedBrightness() {
	if (!ledsActive)
		return 0;
	uint8_t save = pendingSave();
	if (save != BRIGHTNESS_SAVED)
		return save;
	// Stored unclamped; the step shown is clamped to the steps
	uint32_t saved = Storage::getInstance().getAnimationOptions().brightness;
	return (uint8_t)((saved > brightnessSteps()) ? brightnessSteps() : saved);
}

static uint8_t shownBrightness() {
	if (!ledsActive)
		return 0;
	if (brightnessRequests != brightnessApplied) {
		uint8_t show = (uint8_t)brightnessRequest;
		return (show == BRIGHTNESS_SAVED) ? savedBrightness() : show;
	}
	return AnimationStation::getInstance().GetBrightnessStepValue();
}

static void postBrightness(uint8_t save, uint8_t show) {
	brightnessRequest = (uint16_t)((save << 8) | show);
	__mem_fence_release();
	brightnessRequests = brightnessRequests + 1;
}

// The step shown is unsaved while it differs from the saved step. Only an
// unsaved SET_BRIGHTNESS sets one without the other, and the hotkeys set both
static bool brightnessUnsaved() {
	return shownBrightness() != savedBrightness();
}

// Shows the saved step again, keeping a save core 1 has not applied yet
static void restoreBrightness() {
	if (brightnessUnsaved())
		postBrightness(pendingSave(), BRIGHTNESS_SAVED);
}

// The animations' idle timeout has them off, until a button press. Read from
// core 0 as the brightness step is, one byte written by core 1
static bool animationsIdle() {
	return ledsActive && AnimationStation::getInstance().IsInIdleTimeout();
}

static uint16_t animationIndex() {
	int8_t index = currentAnimation();
	return (index < 0) ? ANIMATION_OFF : (uint16_t)index;
}

static uint32_t storedProfile() {
	return Storage::getInstance().getGamepadOptions().profileNumber;
}

// The profile whose pin map a stored number puts in effect, as
// Storage::setFunctionalPinMappings() selects. A number that is not an
// enabled alternative profile means the base mapping, profile 1
static uint8_t activeProfile(uint32_t number) {
	const ProfileOptions & profiles = Storage::getInstance().getProfileOptions();
	if ((number >= 2) && (number <= profiles.gpioMappingsSets_count + 1u) &&
			profiles.gpioMappingsSets[number - 2].enabled)
		return (uint8_t)number;
	return 1;
}

static uint8_t playerNumber() {
	uint32_t player = Storage::getInstance().GetProcessedGamepad()->auxState.playerID.value;
	return (uint8_t)((player > 255) ? 255 : player);
}

// ---------------------------------------------------------------------------
// Input modes (page 2)

#define HOST_LIGHTING_INPUT_MODE(name, value) { name, name##_NAME },
static const struct {
	InputMode mode;
	const char * name;
} inputModes[] = { InputMode_VALUELIST(HOST_LIGHTING_INPUT_MODE) };
#undef HOST_LIGHTING_INPUT_MODE

static bool listedInputMode(InputMode mode) {
	return (mode != INPUT_MODE_CONFIG) && DriverManager::hasDriver(mode);
}

// ---------------------------------------------------------------------------
// Names (page 3), the GpioAction and LightType values with their names in
// enums.proto. LightType's generated names carry the type name, left out.

struct EnumName {
	int16_t value;
	const char * name;
};

#define HOST_LIGHTING_ACTION_NAME(name, value) { (int16_t)(value), #name },
#define HOST_LIGHTING_KIND_NAME(name, value) { (int16_t)(value), #name + sizeof("LightType") },
static const EnumName actionNames[] = { GpioAction_VALUELIST(HOST_LIGHTING_ACTION_NAME) };
static const EnumName kindNames[] = { LightType_VALUELIST(HOST_LIGHTING_KIND_NAME) };
constexpr bool hasPrefix(const char * text, const char * prefix) {
	return (*prefix == 0) || ((*text == *prefix) && hasPrefix(text + 1, prefix + 1));
}
#define HOST_LIGHTING_KIND_PREFIX(name, value) \
	static_assert(hasPrefix(#name, "LightType_"), "page 3 strips the LightType_ prefix");
LightType_VALUELIST(HOST_LIGHTING_KIND_PREFIX)
#undef HOST_LIGHTING_KIND_PREFIX
#undef HOST_LIGHTING_ACTION_NAME
#undef HOST_LIGHTING_KIND_NAME
static_assert((_GpioAction_MIN >= INT16_MIN) && (_GpioAction_MAX <= INT16_MAX),
	"page 3 carries GpioAction values as signed 16-bit");
constexpr uint16_t ACTION_NAMES = sizeof(actionNames) / sizeof(actionNames[0]);
constexpr uint16_t KIND_NAMES = sizeof(kindNames) / sizeof(kindNames[0]);

// ---------------------------------------------------------------------------
// Pages

namespace {

struct Writer {
	uint8_t * p;
	void u8(uint8_t v) { *p++ = v; }
	void u16(uint16_t v) { put16(p, v); p += 2; }
	void u32(uint32_t v) { put32(p, v); p += 4; }
	void bytes(const void * src, uint8_t n) { memcpy(p, src, n); p += n; }
	void text(const char * s, uint8_t size) {
		size_t n = strnlen(s, size);
		memcpy(p, s, n);
		memset(p + n, 0, size - n);
		p += size;
	}
};

typedef uint16_t (*TotalFn)();
typedef void (*RecordFn)(uint16_t index, uint16_t groups, Writer & w);

struct PageDef {
	uint8_t number;
	const uint8_t * widths;
	uint8_t groupCount;
	TotalFn total;
	RecordFn record;
};

// Hall-effect triggers the add-on reads (HETriggerAddon::setup), muxChannels
// on each of up to four muxes, 32 at most
uint8_t triggerTotal() {
	const HETriggerOptions & options = Storage::getInstance().getAddonOptions().heTriggerOptions;
	int32_t channels = options.muxChannels;
	if (!options.enabled || ((channels != 1) && (channels != 4) && (channels != 8) && (channels != 16)))
		return 0;
	int32_t muxes = 32 / channels;
	return (uint8_t)(((muxes > 4) ? 4 : muxes) * channels);
}

// Expander pins, while the PCF8575 add-on is loaded
uint8_t expanderTotal(bool loaded) {
	const PCF8575Options & options = Storage::getInstance().getAddonOptions().pcf8575Options;
	const uint8_t size = sizeof(options.pins) / sizeof(options.pins[0]);
	return !loaded ? 0 : ((options.pins_count > size) ? size : (uint8_t)options.pins_count);
}

// Add-on inputs this boot, counted in HostLighting::setup(). Their options
// change only in the web configurator, which reboots
uint8_t triggerKeys = 0;
uint8_t expanderKeys = 0;

// The page 10 key of the input a light follows, a GPIO pin or a
// Hall-effect trigger the add-on reads. -1 when it follows neither.
int ownerKey(const Light & light) {
	if (light.InputKey < 0)
		return -1;
	if (light.InputSource == LightInputSource::LightInputSource_HallEffect)
		return (light.InputIndex < triggerKeys) ? (int)KEY_TRIGGER + light.InputIndex : -1;
	return (light.InputIndex < NUM_BANK0_GPIOS) ? (int)light.InputIndex : -1;
}

// The key after key, in key order; KEY_END after the last
uint16_t nextKey(uint16_t key) {
	if (++key == NUM_BANK0_GPIOS)
		key = KEY_TRIGGER;
	if (key == KEY_TRIGGER + triggerKeys)
		key = KEY_EXPANDER;
	if (key == KEY_EXPANDER + expanderKeys)
		key = KEY_END;
	return key;
}

// The action of a key from nextKey(). An expander output mirrors a button
// and is not an input.
GpioAction keyAction(uint16_t key) {
	const AddonOptions & addons = Storage::getInstance().getAddonOptions();
	if (key < KEY_TRIGGER)
		return Storage::getInstance().getProfilePinMappings()[key].action;
	if (key < KEY_EXPANDER)
		return addons.heTriggerOptions.triggers[key - KEY_TRIGGER].action;
	const GpioMappingInfo & pin = addons.pcf8575Options.pins[key - KEY_EXPANDER];
	return (pin.direction == GPIO_DIRECTION_INPUT) ? pin.action : GpioAction::NONE;
}

// Which of the controls with an action this key is, from 1, in key order,
// and how many there are
void controlInstance(uint16_t key, GpioAction action, uint16_t & instance, uint16_t & instances) {
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

bool isControl(GpioAction action) { return (int)action > 0; }

// The key of the index-th control; false past the last
bool controlKey(uint16_t index, uint16_t & key) {
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

struct ControlCounts {
	uint16_t total = 0;
	uint16_t lit = 0;
};

ControlCounts controlCounts() {
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
const char * const identityStrings[IDENTITY_GROUPS] = {
	BOARD_CONFIG_LABEL, GP2040_BOARDCONFIG, GP2040PLATFORM, GP2040VERSION,
	GP2040VERSIONID, GP2040BUILD, GP2040CONFIG, BOARD_CONFIG_FILE_NAME,
};

uint8_t identityWidth(uint8_t group) {
	size_t n = strlen(identityStrings[group]);
	return (n >= IDENTITY_STRING_MAX) ? IDENTITY_STRING_MAX : (uint8_t)(n + 1);
}

// Page 1 - State
uint16_t oneRecord() { return 1; }

void stateRecord(uint16_t, uint16_t groups, Writer & w) {
	if (groups & (1 << 0)) {
		w.u8((uint8_t)((liveActive ? STATE_LIVE : 0) | (brightnessUnsaved() ? STATE_UNSAVED : 0) |
			(animationsIdle() ? STATE_IDLE : 0)));
		w.u8((uint8_t)DriverManager::getInstance().getInputMode());
		w.u8(activeProfile(storedProfile()));
		w.u8(playerNumber());
		w.u16(animationIndex());
		w.u8(shownBrightness());
		w.u8(takeoverNumber);
		w.u8(savedBrightness());
	}
	if (groups & (1 << 1)) {
		uint8_t holder[CONTROLLER_SIZE];
		putController(holder);
		w.bytes(holder, CONTROLLER_SIZE);
	}
	if (groups & (1 << 2)) {
		w.u8(takeoverMode);
		w.u16(timeoutMs);
		w.u8(applyBrightness ? 1 : 0);
	}
}

// Page 2 - Input modes
uint16_t inputModeTotal() {
	uint16_t n = 0;
	for (const auto & m : inputModes)
		n += listedInputMode(m.mode) ? 1 : 0;
	return n;
}

void inputModeRecord(uint16_t index, uint16_t groups, Writer & w) {
	for (const auto & m : inputModes) {
		if (!listedInputMode(m.mode) || (index-- != 0))
			continue;
		if (groups & (1 << 0)) {
			const HostLightingOptions & options = Storage::getInstance().getAddonOptions().hostLightingOptions;
			uint8_t flags = 0;
			if ((m.mode == INPUT_MODE_GENERIC) || (m.mode == INPUT_MODE_KEYBOARD) || (m.mode == INPUT_MODE_SINPUT))
				flags = 0x01;
			else if ((m.mode == INPUT_MODE_XINPUT) && (options.xinputMode != HOST_LIGHTING_XINPUT_MODE_OFF))
				flags = (options.xinputMode == HOST_LIGHTING_XINPUT_MODE_AUTO) ? 0x03 : 0x01;
			w.u8((uint8_t)m.mode);
			w.u8(flags);
		}
		if (groups & (1 << 1))
			w.text(m.name, INPUT_MODE_NAME_SIZE);
		return;
	}
}

// Page 3 - Names. The GpioAction values, then the LightType values
uint16_t nameTotal() {
	return ACTION_NAMES + KIND_NAMES;
}

void nameRecord(uint16_t index, uint16_t groups, Writer & w) {
	const bool action = index < ACTION_NAMES;
	const EnumName & entry = action ? actionNames[index] : kindNames[index - ACTION_NAMES];
	if (groups & (1 << 0)) {
		w.u8(action ? NAMES_GPIO_ACTION : NAMES_LIGHT_TYPE);
		w.u16((uint16_t)entry.value);
	}
	if (groups & (1 << 1))
		w.text(entry.name, NAME_SIZE);
}

// Page 9 - Profiles. Profile 1 is the base mapping
uint16_t profileTotal() {
	return (uint16_t)(Storage::getInstance().getProfileOptions().gpioMappingsSets_count + 1);
}

// Page 10 - Controls
void controlRecord(uint16_t index, uint16_t groups, Writer & w) {
	uint16_t key = 0;
	if (!controlKey(index, key))
		return;
	GpioAction action = keyAction(key);
	uint16_t instance, instances;
	controlInstance(key, action, instance, instances);

	uint16_t lightTotal = lightCount(), firstLight = INDEX_NONE, count = 0, firstLed = INDEX_NONE;
	uint8_t minX = 255, minY = 255, maxX = 0, maxY = 0;
	for (uint16_t i = 0; i < lightTotal; i++) {
		const Light & light = lights->AllLights[i];
		if (ownerKey(light) != key)
			continue;
		if (count++ == 0) {
			firstLight = i;
			firstLed = (uint16_t)light.FirstLedIndex;
		}
		uint8_t x = gridCoord(light.Position.XPosition), y = gridCoord(light.Position.YPosition);
		minX = (x < minX) ? x : minX;
		minY = (y < minY) ? y : minY;
		maxX = (x > maxX) ? x : maxX;
		maxY = (y > maxY) ? y : maxY;
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

uint16_t controlTotal() { return controlCounts().total; }

// Page 11 - Lights
void lightRecord(uint16_t index, uint16_t groups, Writer & w) {
	const Light & light = lights->AllLights[index];
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
uint8_t pageChanges[CONFIGURATION_PAGES] = {};

void summaryRecord(uint16_t, uint16_t groups, Writer & w) {
	if (groups & (1 << 0)) {
		ControlCounts counts = controlCounts();
		const DisplayOptions & display = Storage::getInstance().getDisplayOptions();
		w.u16(ledExtent);
		w.u16(LED_LIMIT);
		w.u16(renderRateHz);
		w.u8((uint8_t)Storage::getInstance().getLedOptions().ledFormat);
		w.u8(whiteChain() ? SUMMARY_WHITE_CHANNEL : 0);
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

void profileRecord(uint16_t index, uint16_t groups, Writer & w) {
	if (!(groups & (1 << 0)))
		return;
	Storage & storage = Storage::getInstance();
	const GpioMappings & mapping = (index == 0)
		? storage.getGpioMappings() : storage.getProfileOptions().gpioMappingsSets[index - 1];
	w.u8((uint8_t)(index + 1));
	w.u8(((index == 0) || mapping.enabled) ? 0x01 : 0x00);
	w.text(mapping.profileLabel, PROFILE_LABEL_SIZE);
}

// Page 12 - Animations
void animationRecord(uint16_t index, uint16_t groups, Writer & w) {
	const AnimationProfile & profile = Storage::getInstance().getAnimationOptions().profiles[index];
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

const PageDef pages[] = {
	{ PAGE_STATE, STATE_GROUPS, sizeof(STATE_GROUPS), oneRecord, stateRecord },
	{ PAGE_INPUT_MODES, INPUT_MODE_GROUPS, sizeof(INPUT_MODE_GROUPS), inputModeTotal, inputModeRecord },
	{ PAGE_NAMES, NAME_GROUPS, sizeof(NAME_GROUPS), nameTotal, nameRecord },
	{ PAGE_SUMMARY, SUMMARY_GROUPS, sizeof(SUMMARY_GROUPS), oneRecord, summaryRecord },
	{ PAGE_PROFILES, PROFILE_GROUPS, sizeof(PROFILE_GROUPS), profileTotal, profileRecord },
	{ PAGE_CONTROLS, CONTROL_GROUPS, sizeof(CONTROL_GROUPS), controlTotal, controlRecord },
	{ PAGE_LIGHTS, LIGHT_GROUPS, sizeof(LIGHT_GROUPS), lightCount, lightRecord },
	{ PAGE_ANIMATIONS, ANIMATION_GROUPS, sizeof(ANIMATION_GROUPS), animationTotal, animationRecord },
};

const PageDef * findPage(uint8_t number) {
	for (const PageDef & page : pages) {
		if (page.number == number)
			return &page;
	}
	return nullptr;
}

} // namespace

// State token and change counters. Each configuration page (8-15) is
// hashed, FNV-1a over every record with all groups as the page writers
// produce them (the Summary without its change counters), so a hash covers
// exactly what its page reports. A page whose hash moved steps its change
// counter. The token is the hash of the page hashes, so the same
// configuration gives the same token.
// Recomputed on the next use after a change of the inputs, which are the
// lights and add-on inputs (fixed per boot) and the pin map (profile switch).
static uint32_t stateToken = 1;
static bool tokenStale = true;
// The profile whose pin map was last hashed, as the reinit applied it
// (GP2040::getReinitGamepad, core 0)
static uint32_t hashedProfile = 0;
static uint32_t pageHashes[CONFIGURATION_PAGES];
static bool pagesHashed = false;

// The animation speeds are on page 12 and change at run time (hotkeys on
// core 1, SET_ANIMATION_SPEED); the token follows them
static int32_t hashedSpeeds[MAX_ANIMATION_PROFILES][3];

static bool speedsChanged() {
	bool changed = false;
	const AnimationOptions & options = Storage::getInstance().getAnimationOptions();
	for (uint8_t i = 0; i < MAX_ANIMATION_PROFILES; i++) {
		const AnimationProfile & profile = options.profiles[i];
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

static uint32_t currentToken() {
	if (speedsChanged())
		tokenStale = true;
	const uint32_t profile = Storage::getInstance().GetGamepad()->lastReinitProfileNumber;
	if (profile != hashedProfile) {
		hashedProfile = profile;
		tokenStale = true;
	}
	if (!tokenStale)
		return stateToken;
	tokenStale = false;
	for (const PageDef & page : pages) {
		if ((page.number < PAGE_SUMMARY) || (page.number >= PAGE_SUMMARY + CONFIGURATION_PAGES))
			continue;
		const uint16_t groups = (page.number == PAGE_SUMMARY) ? 1 : (uint16_t)((1u << page.groupCount) - 1);
		uint16_t total = page.total();
		uint32_t hash = 2166136261u;
		hash = (hash ^ (total & 0xFF)) * 16777619u;
		hash = (hash ^ (total >> 8)) * 16777619u;
		for (uint16_t i = 0; i < total; i++) {
			uint8_t record[RECORD_AREA];
			Writer w = { record };
			page.record(i, groups, w);
			for (const uint8_t * b = record; b < w.p; b++)
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
static void buildPage(uint8_t * reply, uint8_t number, uint16_t start, uint16_t mask) {
	reply[HEADER_PAGE] = number;
	put16(&reply[HEADER_START], start);
	put32(&reply[HEADER_TOKEN], currentToken());

	if (number == PAGE_IDENTITY) {
		uint16_t wanted = mask ? (mask & ((1u << IDENTITY_GROUPS) - 1)) : ((1u << IDENTITY_GROUPS) - 1);
		put16(&reply[HEADER_TOTAL], 1);
		uint8_t used = 0;
		uint16_t returned = 0;
		if (start == 0) {
			for (uint8_t g = 0; g < IDENTITY_GROUPS; g++) {
				uint8_t width = identityWidth(g);
				if (!(wanted & (1u << g)) || (used + width > RECORD_AREA))
					continue;
				// With its NUL when shorter than the group, cut at 48 otherwise
				memcpy(&reply[HEADER_RECORDS + used], identityStrings[g], width);
				used += width;
				returned |= (uint16_t)(1u << g);
			}
		}
		reply[HEADER_COUNT] = returned ? 1 : 0;
		reply[HEADER_STRIDE] = used;
		put16(&reply[HEADER_GROUPS], returned);
		return;
	}

	const PageDef * page = findPage(number);
	uint16_t defined = (uint16_t)((1u << page->groupCount) - 1);
	uint16_t groups = mask ? (mask & defined) : defined;
	uint8_t stride = 0;
	for (uint8_t g = 0; g < page->groupCount; g++) {
		if (groups & (1u << g))
			stride += page->widths[g];
	}
	uint16_t total = page->total();
	put16(&reply[HEADER_TOTAL], total);
	put16(&reply[HEADER_GROUPS], groups);
	reply[HEADER_STRIDE] = stride;
	uint8_t count = 0;
	if (stride != 0) {
		for (uint32_t i = start; (i < total) && (count < RECORD_AREA / stride); i++, count++) {
			Writer w = { &reply[HEADER_RECORDS + count * stride] };
			page->record((uint16_t)i, groups, w);
		}
	}
	reply[HEADER_COUNT] = count;
}

// ---------------------------------------------------------------------------
// Replies and events

static uint8_t replyQueue[REPLY_QUEUE_SLOTS][REPORT_SIZE];
static uint8_t queueHead = 0;
static uint8_t queueCount = 0;

static void clearQueue() {
	queueHead = 0;
	queueCount = 0;
}

// The end of a USB session. Unclaimed and unsubscribed, the default
// CONFIGURE settings and no reply waiting. A claim it ends is a change of
// holder.
static void endSession() {
	if (claimToken != 0)
		setHolder(0, nullptr, 0);
	else
		resetControl();
	subscription = 0;
	eventPending = false;
	clearQueue();
	clockRunning = false;
}

// Page 1 fields that change outside the request path (profile, player,
// animation, brightness steps, the unsaved and idle flags) and the token, as
// page 1 last showed them
static uint8_t seenProfile = 0;
static uint8_t seenPlayer = 0;
static uint16_t seenAnimation = 0;
static uint8_t seenBrightness = 0;
static uint8_t seenSaved = 0;
static bool seenUnsaved = false;
static bool seenIdle = false;
static uint32_t seenToken = 0;

// Records those fields as page 1 shows them now; true when any changed
static bool noteState(uint32_t number) {
	uint8_t profile = activeProfile(number);
	uint8_t player = playerNumber();
	uint16_t animation = animationIndex();
	uint8_t brightness = shownBrightness();
	uint8_t saved = savedBrightness();
	bool unsaved = brightnessUnsaved();
	bool idle = animationsIdle();
	uint32_t token = currentToken();
	bool changed = (profile != seenProfile) || (player != seenPlayer) || (animation != seenAnimation) ||
		(brightness != seenBrightness) || (saved != seenSaved) || (unsaved != seenUnsaved) || (idle != seenIdle) ||
		(token != seenToken);
	seenProfile = profile;
	seenPlayer = player;
	seenAnimation = animation;
	seenBrightness = brightness;
	seenSaved = saved;
	seenUnsaved = unsaved;
	seenIdle = idle;
	seenToken = token;
	return changed;
}

// Hands the next reply, else a pending event, to an idle IN endpoint. An
// event is built when it is sent and only while no reply is queued.
static void drain() {
	int instance = lightingInstance();
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
		uint8_t * event = replyQueue[queueHead];
		memset(event, 0, REPORT_SIZE);
		event[0] = EVENT_MARKER;
		event[1] = 0; // STATE_CHANGED
		buildPage(event, PAGE_STATE, 0, 0);
		if (tud_hid_n_report((uint8_t)instance, 0, event, REPORT_SIZE)) {
			eventPending = false;
			// The event shows page 1 as it is now, so no second event for the same values
			noteState(storedProfile());
		}
	}
}

// ---------------------------------------------------------------------------
// Commands

namespace {

uint8_t invalid(uint8_t * reply, uint8_t offset) {
	reply[3] = offset;
	return STATUS_INVALID;
}

// Offset of the first byte of magic that differs, 0 when all match
uint8_t magicMismatch(const uint8_t * req, const char * magic) {
	for (uint8_t i = 0; i < 4; i++) {
		if (req[4 + i] != (uint8_t)magic[i])
			return (uint8_t)(4 + i);
	}
	return 0;
}

bool commandImplemented(uint8_t command);

uint8_t cmdHello(const uint8_t *, uint8_t * reply) {
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
	for (const PageDef & page : pages)
		setBit(&reply[HELLO_PAGES], page.number);
	put16(&reply[HELLO_ADDRESSING], ADDRESSING_DEFINED);
	put16(&reply[HELLO_PIXEL_FORMATS], (1 << PIXEL_RGB) | (1 << PIXEL_RGBW) | (1 << PIXEL_RGB565));
	put16(&reply[HELLO_EVENTS], EVENTS_DEFINED);
	put16(&reply[HELLO_EVENTS_ENABLED], subscription);
	return STATUS_OK;
}

uint8_t cmdGetPage(const uint8_t * req, uint8_t * reply) {
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
		return STATUS_STALE_TOKEN;
	}
	buildPage(reply, number, start, get16(&req[GET_PAGE_MASK]));
	return STATUS_OK;
}

uint8_t cmdClaim(const uint8_t * req, uint8_t * reply) {
	uint8_t action = req[CLAIM_ACTION];
	uint32_t token = get32(&req[CLAIM_TOKEN]);
	const uint8_t * name = &req[CLAIM_NAME];
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
	putController(&reply[CLAIM_HOLDER]);
	return STATUS_OK;
}

uint8_t cmdConfigure(const uint8_t * req, uint8_t * reply) {
	uint8_t mode = req[CONFIGURE_TAKEOVER];
	uint16_t timeout = get16(&req[CONFIGURE_TIMEOUT]);
	uint8_t brightness = req[CONFIGURE_BRIGHTNESS];
	if (mode > TAKEOVER_OVERLAY)
		return invalid(reply, CONFIGURE_TAKEOVER);
	if (brightness > 1)
		return invalid(reply, CONFIGURE_BRIGHTNESS);
	if (timeout == 0)
		timeout = TIMEOUT_DEFAULT_MS;
	timeout = (timeout < TIMEOUT_MIN_MS) ? TIMEOUT_MIN_MS : ((timeout > TIMEOUT_MAX_MS) ? TIMEOUT_MAX_MS : timeout);
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
void watchState() {
	// A profile switch reaches +2 before its reinit (GP2040::getReinitGamepad,
	// next main-loop pass) changes the token. Wait for the reinit, so one
	// event carries both. One read of the stored number for both uses.
	const uint32_t number = storedProfile();
	if (number != Storage::getInstance().GetGamepad()->lastReinitProfileNumber)
		return;
	if (noteState(number))
		raiseEvent();
}

uint8_t cmdSubscribe(const uint8_t * req, uint8_t * reply) {
	uint16_t added = (uint16_t)(get16(&req[3]) & EVENTS_DEFINED);
	if (subscription == 0 && added != 0)
		noteState(storedProfile());
	subscription |= added;
	put16(&reply[3], subscription);
	return STATUS_OK;
}

// STAGE and UNSTAGE (pixel == nullptr). Everything is validated before
// anything is staged, so an invalid request changes nothing.
uint8_t stageCommand(const uint8_t * req, uint8_t * reply, bool unstage) {
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
		if (capacity > OUTCOME_BITS)
			capacity = OUTCOME_BITS;
		bool ledRun = (addressing == ADDRESSING_LED_RUN);
		if (ledRun && (first >= LED_LIMIT))
			return invalid(reply, RUN_FIRST);
		if ((count == 0) || (count > capacity) || (ledRun && ((uint32_t)first + count > LED_LIMIT)))
			return invalid(reply, RUN_COUNT);
		for (uint8_t n = 0; n < count; n++) {
			uint32_t pixel = 0;
			if (!unstage)
				pixel = decodePixel(&req[RUN_DATA + n * pixelBytes], format, white);
			const uint32_t * value = unstage ? nullptr : &pixel;
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
		if (capacity > OUTCOME_BITS)
			capacity = OUTCOME_BITS;
		if ((n == 0) || (n > capacity))
			return invalid(reply, ENTRY_COUNT);
		const uint16_t total = lightCount();
		for (uint8_t e = 0; e < n; e++) {
			const uint8_t * entry = &req[ENTRY_DATA + e * (addressBytes + pixelBytes)];
			uint16_t address = (addressBytes == 1) ? entry[0] : get16(entry);
			uint32_t pixel = 0;
			if (!unstage)
				pixel = decodePixel(entry + addressBytes, format, white);
			const uint32_t * value = unstage ? nullptr : &pixel;
			bool ok = false;
			if (addressing == ADDRESSING_LIGHT) {
				ok = stageLight(address, value);
			} else {
				for (uint16_t i = 0; i < total; i++) {
					const Light & light = lights->AllLights[i];
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

uint8_t cmdStage(const uint8_t * req, uint8_t * reply) { return stageCommand(req, reply, false); }
uint8_t cmdUnstage(const uint8_t * req, uint8_t * reply) { return stageCommand(req, reply, true); }

uint8_t cmdFill(const uint8_t * req, uint8_t * reply) {
	uint8_t format = (uint8_t)(req[3] >> 4);
	if (format > PIXEL_RGB565)
		return invalid(reply, 3);
	uint32_t pixel = decodePixel(&req[4], format, whiteChain());
	for (uint16_t led = 0; led < LED_LIMIT; led++)
		stageLed(led, &pixel);
	return STATUS_OK;
}

uint8_t cmdClear(const uint8_t *, uint8_t *) {
	clearStaged();
	return STATUS_OK;
}

uint8_t cmdCommit(const uint8_t *, uint8_t *) {
	publish();
	return STATUS_OK;
}

uint8_t cmdRelease(const uint8_t *, uint8_t *) {
	endTakeover();
	return STATUS_OK;
}

uint8_t cmdSetProfile(const uint8_t * req, uint8_t * reply) {
	Storage & storage = Storage::getInstance();
	if (req[3] == activeProfile(storedProfile()))
		return STATUS_OK;
	if (!storage.setProfile(req[3]))
		return invalid(reply, 3);
	// Saved as the profile hotkeys save; the pin map, the token and the
	// event follow at the next main-loop pass, after the reinit
	// (GP2040::getReinitGamepad)
	EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true));
	return STATUS_OK;
}

uint8_t cmdSetAnimation(const uint8_t * req, uint8_t * reply) {
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

uint8_t cmdSetAnimationSpeed(const uint8_t * req, uint8_t * reply) {
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
	AnimationProfile & profile = Storage::getInstance().getAnimationOptions().profiles[index];
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
uint8_t cmdSetBrightness(const uint8_t * req, uint8_t * reply) {
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
	// Each SET_BRIGHTNESS that leaves an unsaved step starts its keepalive
	if (shown != saved) {
		unsavedUs = clockUs;
		clockRunning = true;
	}
	return STATUS_OK;
}

uint8_t cmdSetInputMode(const uint8_t * req, uint8_t * reply) {
	InputMode mode = (InputMode)req[3];
	if (!listedInputMode(mode))
		return invalid(reply, 3);
	uint8_t bad = magicMismatch(req, "MODE");
	if (bad)
		return invalid(reply, bad);
	// Saved and rebooted from the main loop, after the reply has been queued
	GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
	if (gamepadOptions.inputMode != mode) {
		gamepadOptions.inputMode = mode;
		EventManager::getInstance().triggerEvent(new GPStorageSaveEvent(true, true));
	} else {
		EventManager::getInstance().triggerEvent(new GPRestartEvent(System::BootMode::GAMEPAD));
	}
	return STATUS_OK;
}

uint8_t cmdReboot(const uint8_t * req, uint8_t * reply) {
	System::BootMode bootMode;
	if (!rebootModeToBootMode(req[3], bootMode))
		return invalid(reply, 3);
	uint8_t bad = magicMismatch(req, "BOOT");
	if (bad)
		return invalid(reply, bad);
	EventManager::getInstance().triggerEvent(new GPRestartEvent(bootMode));
	return STATUS_OK;
}

typedef uint8_t (*CommandFn)(const uint8_t * req, uint8_t * reply);

struct Command {
	uint8_t id;
	CommandFn run;
};

const Command commands[] = {
	{ CMD_HELLO, cmdHello },
	{ CMD_GET_PAGE, cmdGetPage },
	{ CMD_CLAIM, cmdClaim },
	{ CMD_CONFIGURE, cmdConfigure },
	{ CMD_SUBSCRIBE, cmdSubscribe },
	{ CMD_STAGE, cmdStage },
	{ CMD_UNSTAGE, cmdUnstage },
	{ CMD_FILL, cmdFill },
	{ CMD_CLEAR, cmdClear },
	{ CMD_COMMIT, cmdCommit },
	{ CMD_RELEASE, cmdRelease },
	{ CMD_SET_PROFILE, cmdSetProfile },
	{ CMD_SET_ANIMATION, cmdSetAnimation },
	{ CMD_SET_ANIMATION_SPEED, cmdSetAnimationSpeed },
	{ CMD_SET_BRIGHTNESS, cmdSetBrightness },
	{ CMD_SET_INPUT_MODE, cmdSetInputMode },
	{ CMD_REBOOT, cmdReboot },
};

const Command * findCommand(uint8_t id) {
	for (const Command & command : commands) {
		if (command.id == id)
			return &command;
	}
	return nullptr;
}

bool commandImplemented(uint8_t command) { return findCommand(command) != nullptr; }

// Session upkeep before each request and in each main-loop pass. A bus reset
// is the one sign of a new host the board gets while it keeps power, so it
// drops the replies still queued and ends a claim, as a change of holder.
// The subscription stays. A host whose handle outlives the reset (a Linux
// reset-resume) would lose it unseen, while it sees a lost claim at once.
bool busResetSeen();

void upkeep() {
	if (busResetSeen()) {
		clearQueue();
		if (claimToken != 0)
			setHolder(0, nullptr, 0);
	}
	checkExpiry();
}

} // namespace

void HostLighting::setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t * buffer, uint16_t bufsize) {
	(void)report_id;
	// OUT endpoint data and SET_REPORT control requests
	if ((report_type != HID_REPORT_TYPE_INVALID) && (report_type != HID_REPORT_TYPE_OUTPUT))
		return;
	if ((lightingInstance() < 0) || (bufsize == 0))
		return;

	// Bytes past a short report read as zero
	uint8_t req[REPORT_SIZE] = {};
	memcpy(req, buffer, (bufsize < REPORT_SIZE) ? bufsize : REPORT_SIZE);

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
	uint8_t * reply = answer ? replyQueue[(queueHead + queueCount) % REPLY_QUEUE_SLOTS] : discard;
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
	const Command * command = findCommand(id);
	uint8_t status;
	if (command == nullptr) {
		status = STATUS_UNSUPPORTED;
	} else if (outdated || (guarded && !open && (!openUnflagged || (flags & EXCLUSIVE_REFUSED_FLAGS)))) {
		putController(&reply[CLAIMED_HOLDER]);
		status = STATUS_CLAIMED;
	} else {
		status = command->run(req, reply);
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

uint16_t HostLighting::getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t * buffer, uint16_t reqlen) {
	// Replies travel on the IN endpoint only; GET_REPORT is not served
	(void)report_id;
	(void)report_type;
	(void)buffer;
	(void)reqlen;
	return 0;
}

const uint8_t * HostLighting::getReportDescriptor() {
	return hostlighting_report_descriptor;
}

// ---------------------------------------------------------------------------
// Core 1

void HostLighting::registerLights(const Lights & layout, uint16_t rateHz) {
	uint16_t extent = 0, width = 0, height = 0;
	uint64_t pins = 0;
	uint32_t triggers = 0;
	for (const Light & light : layout.AllLights) {
		uint16_t first, end;
		if (lightLeds(light, first, end) && (end > extent))
			extent = end;
		uint16_t x = (uint16_t)(gridCoord(light.Position.XPosition) + 1);
		uint16_t y = (uint16_t)(gridCoord(light.Position.YPosition) + 1);
		width = (x > width) ? x : width;
		height = (y > height) ? y : height;
		// From the light itself, as triggerKeys is counted on core 0
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

void HostLighting::render(AnimationStation & station, uint32_t * frame, int format) {
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
		AnimationOptions & options = Storage::getInstance().getAnimationOptions();
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
	static uint8_t scale[256];
	static uint32_t scaleBits = 0xFFFFFFFF;
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
	static uint32_t converted[LED_LIMIT];
	static uint32_t convertedBits[(LED_LIMIT + 31) / 32];
	static uint32_t convertedSeq = 0xFFFFFFFF;
	static int convertedFormat = -1;
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
// Modes and XInput

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

bool HostLighting::isLightingInterface(uint8_t itf) {
	return (int)itf == lightingInstance();
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

// USB bus events, recorded in interrupt context
static volatile bool busResetPending = false;
static volatile bool suspendSinceResume = false;
static volatile bool resetAfterSuspend = false;

namespace {
bool busResetSeen() {
	if (!busResetPending)
		return false;
	busResetPending = false;
	return true;
}
} // namespace

void HostLighting::xinputAutoDetectTask(bool consoleAuthSeen) {
	const HostLightingOptions & options = Storage::getInstance().getAddonOptions().hostLightingOptions;
	if (!options.enabled || (options.xinputMode != HOST_LIGHTING_XINPUT_MODE_AUTO))
		return;

	if (xinputCompositeActive()) {
		// Once the composite has mounted, a bus reset that follows a suspend
		// means a switch moved the board to another host. Back to stock, so
		// a console never enumerates the PC-only composite. A PC-side port
		// reset has no suspend before it and keeps the composite.
		static bool mounted = false;
		if (!mounted) {
			if (get_usb_mounted() && !get_usb_suspended()) {
				mounted = true;
				resetAfterSuspend = false;
			}
			return;
		}
		if (resetAfterSuspend) {
			watchdog_hw->scratch[HOST_LIGHTING_VERDICT_SCRATCH] = 0;
			System::reboot(System::BootMode::GAMEPAD);
		}
		return;
	}

	// Stock identity, decided once per boot
	static bool settled = false;
	if (settled)
		return;
	if (consoleAuthSeen) {
		settled = true;
		return;
	}
	static uint32_t enumeratedSinceMs = 0;
	if (!get_usb_mounted() || get_usb_suspended()) {
		enumeratedSinceMs = 0;
		return;
	}
	uint32_t now = nowMs();
	if (enumeratedSinceMs == 0) {
		enumeratedSinceMs = now;
		return;
	}
	if ((now - enumeratedSinceMs) >= HOST_LIGHTING_XINPUT_DETECT_MS) {
		watchdog_hw->scratch[HOST_LIGHTING_VERDICT_SCRATCH] = HOST_LIGHTING_VERDICT_PC_HOST;
		System::reboot(System::BootMode::GAMEPAD);
	}
}

// ---------------------------------------------------------------------------
// USB session (core 0, except usbEvent)

void HostLighting::reportComplete(uint8_t itf) {
	if ((int)itf == lightingInstance())
		drain();
}

// From tud_event_hook_cb, in interrupt context. Sets flags only
void HostLighting::usbEvent(uint32_t eventId) {
	switch (eventId) {
		case DCD_EVENT_BUS_RESET:
			busResetPending = true;
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

// TinyUSB reports an unmount only for SET_CONFIGURATION 0 on the RP2040,
// which forces VBUS detection. An unplugged bus-powered board loses power,
// and behind a powered hub or switch the board is suspended, then reset at
// the next plug-in. The takeover ends at the suspend, and the reset ends
// the claim and drops the replies still queued (upkeep()).
void HostLighting::usbUnmounted() {
	if (lightingInstance() < 0)
		return;
	endSession();
}

void HostLighting::usbSuspended() {
	if (lightingInstance() < 0)
		return;
	// The event, if subscribed, waits for resume
	endTakeover();
	restoreBrightness();
}

bool HostLighting::interfaceActive = false;
bool HostLighting::xinputAutoDetect = false;

void HostLighting::setup(bool expanderInputs) {
	triggerKeys = triggerTotal();
	expanderKeys = expanderTotal(expanderInputs);

	// A boot into another mode forgets the XInput AUTO verdict, since the
	// board may then be moved to a console without losing power. The web
	// configurator, reached only from a PC, keeps it.
	DriverManager & drivers = DriverManager::getInstance();
	if (!drivers.isConfigMode() && (drivers.getInputMode() != INPUT_MODE_XINPUT))
		watchdog_hw->scratch[HOST_LIGHTING_VERDICT_SCRATCH] = 0;

	const HostLightingOptions & options = Storage::getInstance().getAddonOptions().hostLightingOptions;
	interfaceActive = lightingInstance() >= 0;
	xinputAutoDetect = !drivers.isConfigMode() && (drivers.getInputMode() == INPUT_MODE_XINPUT) &&
		options.enabled && (options.xinputMode == HOST_LIGHTING_XINPUT_MODE_AUTO);
}

void HostLighting::task() {
	if (lightingInstance() < 0)
		return;
	if (busResetPending || clockRunning)
		upkeep();
	if (subscription != 0) {
		static uint32_t watchedUs = 0;
		uint32_t now = nowUs();
		if ((now - watchedUs) >= WATCH_INTERVAL_US) {
			watchedUs = now;
			watchState();
		}
	}
	if ((queueCount != 0) || eventPending)
		drain();
}
