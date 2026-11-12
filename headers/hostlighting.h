/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#ifndef _HOSTLIGHTING_H_
#define _HOSTLIGHTING_H_

#include <stdint.h>

#include "tusb.h"
#include "enums.pb.h"
#include "gpusbfunction.h"

// Host Lighting Protocol (HLP) 2.0, a vendor HID interface through which a
// host drives the board's RGB LEDs and reads its layout and state. Exposed
// beside the input interface in XInput, Keyboard, Generic and SInput modes
// when HostLightingOptions enables it, as a USB function (GPUSBFunction). See
// docs/host-lighting.md.

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

// Default for HostLightingOptions.allowKeyboardMappingChanges. While off,
// HELLO omits SET_KEYBOARD_MAPPING and the board answers it as unsupported.
#ifndef HOST_LIGHTING_ALLOW_KEYBOARD_MAPPING_CHANGES
#define HOST_LIGHTING_ALLOW_KEYBOARD_MAPPING_CHANGES 0
#endif

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
struct Light;
struct Lights;

class HostLighting : public GPUSBFunction {
public:
	HostLighting(HostLighting const&) = delete;
	void operator=(HostLighting const&) = delete;
	static HostLighting& getInstance() // Thread-safe storage ensures cross-thread talk
	{
		static HostLighting instance;
		return instance;
	}

	// Core 0, from GP2040::setup() after the add-ons load and before USB
	// starts. Counts the add-on inputs.
	// expanderInputs is true when the PCF8575 add-on is loaded, so page 10
	// lists its input pins.
	void setup(bool expanderInputs);

	// ON in the modes that always carry the lighting interface. In XInput it
	// follows the XInput lighting setting. OFF while HostLightingOptions is
	// off.
	USBFunctionSetting setting(InputMode mode) override;

	// LED add-on (core 1). Publishes the light layout and the render rate.
	// The layout must not change while the lighting interface exists.
	void registerLights(const Lights& layout, uint16_t rateHz);

	// LED add-on (core 1), each render tick before the frame is shown.
	// Applies host animation, speed and brightness requests, then a live host
	// frame.
	void render(AnimationStation& station, uint32_t* frame, int format);

	const uint8_t* getReportDescriptor() override;
	uint16_t getReportDescriptorLength() override;
	bool hasOutEndpoint() override { return true; }
	uint16_t getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) override;
	void setReport(uint8_t report_id, hid_report_type_t report_type, const uint8_t* buffer, uint16_t bufsize) override;

	// USB session hooks (core 0, except usbEvent)
	void reportComplete() override;
	void usbEvent(uint32_t eventId) override; // Sets flags only, in interrupt context
	void usbUnmounted() override;
	void usbSuspended() override;

	// Core 0 main loop, after tud_task(), for expiry, events and the reply drain
	void process() override;

private:
	// Every member has a constant initialiser, so the object is built at
	// compile time and getInstance() needs no first-use guard. Core 1 calls
	// it.
	HostLighting() = default;

	// Wire values that size or start the members below (docs/host-lighting.md)
	static constexpr uint8_t REPORT_SIZE = 64;
	// Replies are queued until the IN endpoint takes them. The depth is on the
	// wire (HELLO [17]), and hosts rely on it.
	static constexpr uint8_t REPLY_QUEUE_SLOTS = 16;
	static constexpr uint8_t CLAIM_NAME_SIZE = 16;
	static constexpr uint8_t TAKEOVER_WHOLE_FRAME = 0;
	static constexpr uint16_t TIMEOUT_DEFAULT_MS = 2000;
	// Configuration pages 8 to 15, with one change counter each in the Summary
	static constexpr uint8_t CONFIGURATION_PAGES = 8;
	// Addressable LED limit (Summary +2..3), the render pipeline's frame
	// (FRAME_MAX). Every frame buffer here is sized from it.
	static constexpr uint16_t LED_LIMIT = 100;
	// MAX_ANIMATION_PROFILES, the profiles with speeds the token follows
	static constexpr uint8_t ANIMATION_PROFILES = 4;

	// Lights and frames
	uint16_t lightCount();
	bool lightLeds(const Light& light, uint16_t& first, uint16_t& end);
	void stageLed(uint16_t led, const uint32_t* pixel);
	bool stageLight(uint16_t ordinal, const uint32_t* pixel);
	void clearStaged();

	// Session takeover, keepalive, claim and subscription
	void raiseEvent();
	void endTakeover();
	void publish();
	void putHolder(uint8_t* at);
	void resetControl();
	void setHolder(uint32_t token, const uint8_t* name, uint8_t flags);
	void checkExpiry();

	// Host requests to core 1
	uint16_t animationTotal();
	uint8_t brightnessSteps();
	uint8_t brightnessMaximum();
	uint8_t speedSteps();
	int8_t currentAnimation();
	uint8_t pendingSave();
	uint8_t savedBrightness();
	uint8_t shownBrightness();
	void postBrightness(uint8_t save, uint8_t show);
	bool brightnessUnsaved();
	void restoreBrightness();
	bool animationsIdle();
	uint16_t animationIndex();

	// Pages
	struct Writer;
	struct ControlCounts;
	typedef uint16_t (HostLighting::*TotalFn)();
	typedef void (HostLighting::*RecordFn)(uint16_t index, uint16_t groups, Writer& w);
	struct PageDef {
		uint8_t number;
		const uint8_t* widths;
		uint8_t groupCount;
		TotalFn total;
		RecordFn record;
	};
	static const PageDef pages[];
	const PageDef* findPage(uint8_t number);
	bool isScannedTriggerKey(uint16_t key);
	int ownerKey(const Light& light);
	uint16_t nextKey(uint16_t key);
	void controlInstance(uint16_t key, GpioAction action, uint16_t& instance, uint16_t& instances);
	bool controlKey(uint16_t index, uint16_t& key);
	ControlCounts controlCounts();
	uint16_t oneRecord();
	void stateRecord(uint16_t index, uint16_t groups, Writer& w);
	uint16_t inputModeTotal();
	void inputModeRecord(uint16_t index, uint16_t groups, Writer& w);
	uint16_t nameTotal();
	void nameRecord(uint16_t index, uint16_t groups, Writer& w);
	uint16_t bootHoldTotal();
	void bootHoldRecord(uint16_t index, uint16_t groups, Writer& w);
	uint16_t profileTotal();
	void controlRecord(uint16_t index, uint16_t groups, Writer& w);
	uint16_t controlTotal();
	uint16_t keyboardMappingTotal();
	void keyboardMappingRecord(uint16_t index, uint16_t groups, Writer& w);
	void lightRecord(uint16_t index, uint16_t groups, Writer& w);
	void summaryRecord(uint16_t index, uint16_t groups, Writer& w);
	void profileRecord(uint16_t index, uint16_t groups, Writer& w);
	void animationRecord(uint16_t index, uint16_t groups, Writer& w);
	bool speedsChanged();
	uint32_t currentToken();
	void buildPage(uint8_t* reply, uint8_t number, uint16_t start, uint16_t mask);

	// Replies and events
	void clearQueue();
	void endSession();
	bool noteState();
	void drain();

	// Commands
	typedef uint8_t (HostLighting::*CommandFn)(const uint8_t* req, uint8_t* reply);
	struct Command {
		uint8_t id;
		CommandFn run;
	};
	static const Command commands[];
	const Command* findCommand(uint8_t id);
	bool commandImplemented(uint8_t command);
	uint8_t cmdHello(const uint8_t* req, uint8_t* reply);
	uint8_t cmdGetPage(const uint8_t* req, uint8_t* reply);
	uint8_t cmdClaim(const uint8_t* req, uint8_t* reply);
	uint8_t cmdConfigure(const uint8_t* req, uint8_t* reply);
	void watchState();
	uint8_t cmdSubscribe(const uint8_t* req, uint8_t* reply);
	uint8_t stageCommand(const uint8_t* req, uint8_t* reply, bool unstage);
	uint8_t cmdStage(const uint8_t* req, uint8_t* reply);
	uint8_t cmdUnstage(const uint8_t* req, uint8_t* reply);
	uint8_t cmdFill(const uint8_t* req, uint8_t* reply);
	uint8_t cmdClear(const uint8_t* req, uint8_t* reply);
	uint8_t cmdCommit(const uint8_t* req, uint8_t* reply);
	uint8_t cmdRelease(const uint8_t* req, uint8_t* reply);
	uint8_t cmdSetProfile(const uint8_t* req, uint8_t* reply);
	uint8_t cmdSetAnimation(const uint8_t* req, uint8_t* reply);
	uint8_t cmdSetAnimationSpeed(const uint8_t* req, uint8_t* reply);
	uint8_t cmdSetBrightness(const uint8_t* req, uint8_t* reply);
	uint8_t cmdSetKeyboardMapping(const uint8_t* req, uint8_t* reply);
	uint8_t cmdSetInputMode(const uint8_t* req, uint8_t* reply);
	uint8_t cmdReboot(const uint8_t* req, uint8_t* reply);
	bool busResetSeen();
	void upkeep();

	// The LED add-on's layout. Written on core 1 in LED setup, which finishes
	// before core 0 calls tud_init(), and again only on an LED restart, which
	// only the web configurator requests (config mode, no lighting interface).
	// It is therefore fixed while the interface exists and read without a lock.
	const Lights* lights = nullptr;
	uint16_t ledExtent = 0;
	uint16_t gridWidth = 0;
	uint16_t gridHeight = 0;
	uint64_t litPins = 0;
	uint32_t litTriggers = 0;
	uint16_t renderRateHz = 0;

	// Add-on inputs this boot, found in setup(). Their options change only in
	// the web configurator, which reboots
	uint32_t scannedTriggers = 0;
	uint8_t expanderKeys = 0;

	// Stored pixels are 0xWWRRGGBB. The staged frame is core 0 only.
	uint32_t stagedPixels[LED_LIMIT] = {};
	uint32_t stagedBits[(LED_LIMIT + 31) / 32] = {};

	// Live frame for core 1, under a sequence lock. Core 0 makes liveSeq odd,
	// writes, makes it even; core 1 retries its copy until it reads the same
	// even value before and after.
	volatile uint32_t liveSeq = 0;
	uint32_t livePixels[LED_LIMIT] = {};
	uint32_t liveBits[(LED_LIMIT + 31) / 32] = {};
	volatile bool liveActive = false;
	volatile uint8_t takeoverMode = TAKEOVER_WHOLE_FRAME;
	volatile bool applyBrightness = true;

	// Session takeover, keepalive, claim and subscription (core 0)
	uint16_t timeoutMs = TIMEOUT_DEFAULT_MS;
	uint8_t takeoverNumber = 0;
	uint64_t keepaliveUs = 0;   // last keepalive refresh
	uint64_t leaseUs = 0;       // last claim renewal
	uint64_t unsavedUs = 0;     // last unsaved brightness keepalive refresh
	uint64_t clockUs = 0;       // last clock check
	bool clockRunning = false;  // a takeover, claim or unsaved step was live then
	bool unsavedStep = false;   // an unsaved SET_BRIGHTNESS ran, until no unsaved step is shown or pending
	uint32_t claimToken = 0;
	uint8_t claimName[CLAIM_NAME_SIZE] = {};
	uint8_t claimFlags = 0;
	uint8_t claimGeneration = 1;
	uint16_t subscription = 0;
	bool eventPending = false;
	uint64_t watchedUs = 0;     // last comparison of page 1, while subscribed

	// Host requests to core 1, which owns the animations. A request is pending
	// while its count differs from the applied count; the value is the last
	// one written.
	volatile int8_t animationRequest = 0;
	volatile uint32_t animationRequests = 0;
	volatile uint32_t animationApplied = 0;
	// A brightness request packs a step to save (BRIGHTNESS_SAVED for none) in
	// the high byte and a step to show (BRIGHTNESS_SAVED for the saved one) in the
	// low byte, so one store carries both. Core 1 resolves the saved step against
	// its own, so a hotkey between request and render is not undone.
	volatile uint16_t brightnessRequest = 0;
	volatile uint32_t brightnessRequests = 0;
	volatile uint32_t brightnessApplied = 0;
	volatile bool ledsActive = false;
	// Speeds changed on core 0 by SET_ANIMATION_SPEED; core 1 re-reads the
	// running animations' speeds when the count moves
	volatile uint32_t speedRequests = 0;
	uint32_t speedApplied = 0;

	// Core 1's render state, the brightness scale table and the converted frame
	uint8_t scale[256] = {};
	uint32_t scaleBits = 0xFFFFFFFF;
	uint32_t converted[LED_LIMIT] = {};
	uint32_t convertedBits[(LED_LIMIT + 31) / 32] = {};
	uint32_t convertedSeq = 0xFFFFFFFF;
	int convertedFormat = -1;

	// State token and change counters (currentToken())
	uint32_t stateToken = 1;
	bool tokenStale = true;
	// The profile with the pin map and keyboard mapping last hashed, as the
	// reinit applied it (GP2040::getReinitGamepad, core 0)
	uint32_t hashedProfile = 0;
	uint32_t pageHashes[CONFIGURATION_PAGES] = {};
	bool pagesHashed = false;
	uint8_t pageChanges[CONFIGURATION_PAGES] = {};
	// The animation speeds last hashed (speedsChanged())
	int32_t hashedSpeeds[ANIMATION_PROFILES][3] = {};

	// Replies waiting for the IN endpoint
	uint8_t replyQueue[REPLY_QUEUE_SLOTS][REPORT_SIZE] = {};
	uint8_t queueHead = 0;
	uint8_t queueCount = 0;

	// Page 1 fields that change outside the request path (profile, player,
	// animation, brightness steps, the unsaved and idle flags, the D-pad mode)
	// and the token, as page 1 last showed them
	uint8_t seenProfile = 0;
	uint8_t seenPlayer = 0;
	uint16_t seenAnimation = 0;
	uint8_t seenBrightness = 0;
	uint8_t seenSaved = 0;
	bool seenUnsaved = false;
	bool seenIdle = false;
	uint8_t seenDpadMode = 0;
	uint32_t seenToken = 0;

	// USB bus events, recorded in interrupt context
	volatile bool busResetPending = false;
};

#endif
