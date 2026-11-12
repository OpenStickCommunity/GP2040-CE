/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2026 OpenStickCommunity (gp2040-ce.info)
 */

#include "keyboardinterface.h"

#include <string.h>

#include "pico/platform.h"
#include "device/dcd.h"

#include "eventmanager.h"
#include "gamepad.h"
#include "inputmodes.h"
#include "storagemanager.h"
#include "events/GPEncoderEvent.h"

// keyboardButtonMask uses the GamepadState button bits, with the directions
// at GAMEPAD_MASK_DU to GAMEPAD_MASK_DR as hotkeys and macros do. Its 32
// bits map to the 32 KeyboardMapping fields.
#define DIRECTION_SHIFT    16
#define DIRECTION_BUTTONS  (GAMEPAD_MASK_DU | GAMEPAD_MASK_DD | GAMEPAD_MASK_DL | GAMEPAD_MASK_DR)

static_assert((GAMEPAD_MASK_DPAD << DIRECTION_SHIFT) == DIRECTION_BUTTONS, "directions follow the sixteen buttons");

// KeyboardMapping field for each mask bit, in bit order
struct MaskKey {
	uint32_t mask;
	uint32_t KeyboardMapping::* code;
};

#define BUTTON_KEY(button) { GAMEPAD_MASK_##button, &KeyboardMapping::keyButton##button }
static constexpr MaskKey maskKeys[] = {
	BUTTON_KEY(B1), BUTTON_KEY(B2), BUTTON_KEY(B3), BUTTON_KEY(B4),
	BUTTON_KEY(L1), BUTTON_KEY(R1), BUTTON_KEY(L2), BUTTON_KEY(R2),
	BUTTON_KEY(S1), BUTTON_KEY(S2), BUTTON_KEY(L3), BUTTON_KEY(R3),
	BUTTON_KEY(A1), BUTTON_KEY(A2), BUTTON_KEY(A3), BUTTON_KEY(A4),
	{ GAMEPAD_MASK_DU, &KeyboardMapping::keyDpadUp }, { GAMEPAD_MASK_DD, &KeyboardMapping::keyDpadDown },
	{ GAMEPAD_MASK_DL, &KeyboardMapping::keyDpadLeft }, { GAMEPAD_MASK_DR, &KeyboardMapping::keyDpadRight },
	BUTTON_KEY(E1), BUTTON_KEY(E2), BUTTON_KEY(E3), BUTTON_KEY(E4),
	BUTTON_KEY(E5), BUTTON_KEY(E6), BUTTON_KEY(E7), BUTTON_KEY(E8),
	BUTTON_KEY(E9), BUTTON_KEY(E10), BUTTON_KEY(E11), BUTTON_KEY(E12),
};
#undef BUTTON_KEY

// Each KeyboardMapping field must appear once, at its mask bit
#define KEYBOARD_MAPPING_FIELD(...) + 1
constexpr uint8_t KEYBOARD_MAPPING_FIELDS = 0 KeyboardMapping_FIELDLIST(KEYBOARD_MAPPING_FIELD, unused);
#undef KEYBOARD_MAPPING_FIELD

constexpr bool maskKeysInBitOrder() {
	for (uint8_t i = 0; i < (sizeof(maskKeys) / sizeof(maskKeys[0])); i++) {
		if (maskKeys[i].mask != (1UL << i))
			return false;
		for (uint8_t j = i + 1; j < (sizeof(maskKeys) / sizeof(maskKeys[0])); j++) {
			if (maskKeys[i].code == maskKeys[j].code)
				return false;
		}
	}
	return true;
}
static_assert((sizeof(maskKeys) / sizeof(maskKeys[0])) == KEYBOARD_MAPPING_FIELDS, "a mask bit for each KeyboardMapping field");
static_assert(maskKeysInBitOrder(), "each KeyboardMapping field once, at its mask's bit");

// Maximum queued volume steps
#define VOLUME_STEPS_MAX  100

void KeyboardInterface::setup() {
	if (getHIDInstance() < 0)
		return;

	const RotaryOptions& rotary = Storage::getInstance().getAddonOptions().rotaryOptions;
	volumeEncoders = 0;
	if (rotary.enabled) {
		if (rotary.encoderOne.enabled && (rotary.encoderOne.mode == ENCODER_MODE_VOLUME))
			volumeEncoders |= 1 << 0;
		if (rotary.encoderTwo.enabled && (rotary.encoderTwo.mode == ENCODER_MODE_VOLUME))
			volumeEncoders |= 1 << 1;
	}

	// The mapping and mask are read again on a profile change and on a save,
	// which a Host Lighting keyboard mapping change triggers
	loadProfile();
	EventManager::getInstance().registerEventHandler(GP_EVENT_PROFILE_CHANGE, GPEVENT_CALLBACK(this->handleMappingChange(event)), this);
	EventManager::getInstance().registerEventHandler(GP_EVENT_STORAGE_SAVE, GPEVENT_CALLBACK(this->handleMappingChange(event)), this);
	if (volumeEncoders != 0)
		EventManager::getInstance().registerEventHandler(GP_EVENT_ENCODER_CHANGE, GPEVENT_CALLBACK(this->handleEncoder(event)), this);
}

USBFunctionSetting KeyboardInterface::setting(InputMode mode) {
	// Keyboard mode sends keys through its own driver
	const InputModeAttributes* attributes = inputModeAttributes(mode);
	if ((attributes == nullptr) || !(attributes->capabilities & INPUT_MODE_USB_FUNCTIONS) ||
			(attributes->capabilities & INPUT_MODE_KEYBOARD_CODES))
		return USBFunctionSetting::OFF;
	const GamepadOptions& options = Storage::getInstance().getGamepadOptions();
	if (!options.keyboardInterfaceEnabled)
		return USBFunctionSetting::OFF;
	// XInput takes the XInput setting, as its composite identity is PC only
	if (mode != INPUT_MODE_XINPUT)
		return USBFunctionSetting::ON;
	switch (options.keyboardInterfaceXInputMode) {
		case KEYBOARD_INTERFACE_XINPUT_MODE_ON:
			return USBFunctionSetting::ON;
		case KEYBOARD_INTERFACE_XINPUT_MODE_AUTO:
			return USBFunctionSetting::AUTO;
		default:
			return USBFunctionSetting::OFF;
	}
}

const uint8_t* KeyboardInterface::getReportDescriptor() {
	return keyboard_report_descriptor;
}

uint16_t KeyboardInterface::getReportDescriptorLength() {
	return sizeof(keyboard_report_descriptor);
}

uint16_t KeyboardInterface::getReport(uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
	(void)report_type;
	if (report_id == KEYBOARD_KEY_REPORT_ID) {
		const uint16_t length = (reqlen < sizeof(report.keycode)) ? reqlen : sizeof(report.keycode);
		memcpy(buffer, report.keycode, length);
		return length;
	}
	if ((report_id == KEYBOARD_MULTIMEDIA_REPORT_ID) && (reqlen >= 1)) {
		buffer[0] = report.multimedia;
		return 1;
	}
	return 0;
}

// Interrupt context. A bus reset clears the host's key state, as an unmount
// does.
void KeyboardInterface::usbEvent(uint32_t eventId) {
	if (eventId == DCD_EVENT_BUS_RESET)
		busReset = true;
}

// After an unmount the next report is sent in full. Queued volume steps are
// dropped, except one already pressed, so its release still goes out.
void KeyboardInterface::usbUnmounted() {
	memset(sentKeycode, 0, sizeof(sentKeycode));
	sentMultimedia = 0;
	keysChanged = true;
	volumeSteps = volume;
}

void KeyboardInterface::usbSuspended() {
	volumeSteps = volume;
}

// Reads the active profile's keyboard mapping and button mask. The base
// profile's mapping is Config.keyboardMapping and its mask is in
// Config.gpioMappings. Masked buttons with no key stay on the gamepad. It can
// run on core 1, from a display profile change or a save, so the mask is
// stored in one write.
void KeyboardInterface::loadProfile() {
	Storage& storage = Storage::getInstance();
	mapping = &storage.getActiveKeyboardMapping();
	const GpioMappings* profile = &storage.getGpioMappings();
	for (const GpioMappings& set : storage.getProfileOptions().gpioMappingsSets) {
		if (&set.keyboardMapping == mapping)
			profile = &set;
	}
	uint32_t mask = profile->keyboardButtonMask;
	for (uint32_t bits = mask; bits != 0; bits &= bits - 1) {
		const uint8_t bit = __builtin_ctz(bits);
		if ((mapping->*maskKeys[bit].code) == 0)
			mask &= ~(1UL << bit);
	}
	buttonMask = mask;
	keysChanged = true;
}

bool KeyboardInterface::sendsKey(uint32_t KeyboardMapping::* code) const {
	for (const MaskKey& key : maskKeys) {
		if (key.code == code)
			return (buttonMask & key.mask) != 0;
	}
	return false;
}

void KeyboardInterface::handleMappingChange(GPEvent* e) {
	(void)e;
	loadProfile();
}

// Steps are dropped while the host is away, so they do not play on its return
void KeyboardInterface::handleEncoder(GPEvent* e) {
	const GPEncoderChangeEvent* event = (GPEncoderChangeEvent*)e;
	if (!(volumeEncoders & (1 << event->encoder)) || !tud_ready())
		return;
	if ((event->direction > 0) && (volumeSteps < VOLUME_STEPS_MAX))
		volumeSteps++;
	else if ((event->direction < 0) && (volumeSteps > -VOLUME_STEPS_MAX))
		volumeSteps--;
}

// Called every pass before the input driver
void __not_in_flash_func(KeyboardInterface::processInputs)(Gamepad* gamepad) {
	GamepadState& state = gamepad->state;
	const uint32_t pressed = (state.buttons & ~DIRECTION_BUTTONS) |
		((uint32_t)(state.dpad & GAMEPAD_MASK_DPAD) << DIRECTION_SHIFT);
	const uint32_t masked = pressed & buttonMask;
	if (masked != 0) {
		state.buttons &= ~(masked & ~DIRECTION_BUTTONS);
		state.dpad &= ~(uint8_t)(masked >> DIRECTION_SHIFT);
	}
	if (masked != keys) {
		keys = masked;
		keysChanged = true;
	}
}

// Each volume step is pressed in one report and released in the next
void KeyboardInterface::buildReport() {
	if (volume != 0) {
		volumeSteps -= volume;
		volume = 0;
	} else if (volumeSteps != 0) {
		volume = (volumeSteps > 0) ? 1 : -1;
	}

	// Directions first, then B1-A4 and E1-E12: KeyboardMapping field order, as
	// KeyboardDriver presses them, so the same media key wins
	report.releaseAllKeys();
	for (uint32_t part : { keys & DIRECTION_BUTTONS, keys & ~DIRECTION_BUTTONS })
		for (uint32_t bits = part; bits != 0; bits &= bits - 1)
			report.pressKey((uint8_t)(mapping->*maskKeys[__builtin_ctz(bits)].code));
	if (volume != 0)
		report.pressKey((volume > 0) ? KEYBOARD_MULTIMEDIA_VOLUME_UP : KEYBOARD_MULTIMEDIA_VOLUME_DOWN);
	keysChanged = false;
	reportPending = true;
}

// Called every pass after tud_task(). Returns at once while nothing has
// changed.
void __not_in_flash_func(KeyboardInterface::process)() {
	if (!reportPending && !keysChanged && !busReset && (volumeSteps == 0) && (volume == 0))
		return;
	if (busReset) {
		busReset = false;
		usbUnmounted();
	}

	// A key press wakes a suspended host, as the input drivers do. TinyUSB
	// ignores this unless the host enabled remote wakeup.
	if (tud_suspended()) {
		if (keysChanged && (keys != 0))
			tud_remote_wakeup();
		return;
	}

	if (!reportPending)
		buildReport();

	const uint8_t instance = (uint8_t)getHIDInstance();
	if (!tud_hid_n_ready(instance))
		return;
	if (memcmp(sentKeycode, report.keycode, sizeof(sentKeycode)) != 0) {
		if (tud_hid_n_report(instance, KEYBOARD_KEY_REPORT_ID, report.keycode, sizeof(report.keycode)))
			memcpy(sentKeycode, report.keycode, sizeof(sentKeycode));
		return;
	}
	if (sentMultimedia != report.multimedia) {
		if (tud_hid_n_report(instance, KEYBOARD_MULTIMEDIA_REPORT_ID, &report.multimedia, sizeof(report.multimedia)))
			sentMultimedia = report.multimedia;
		return;
	}
	reportPending = false;
}
