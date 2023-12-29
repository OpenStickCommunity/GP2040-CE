#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "BoardConfig.h"
#include "types.h"
#include <string.h>

#include "enums.pb.h"
#include "gamepad/GamepadDebouncer.h"
#include "gamepad/GamepadState.h"
#include "gamepad/descriptors/HIDDescriptors.h"
#include "gamepad/descriptors/SwitchDescriptors.h"
#include "gamepad/descriptors/XInputDescriptors.h"
#include "gamepad/descriptors/KeyboardDescriptors.h"
#include "gamepad/descriptors/PS4Descriptors.h"
#include "gamepad/descriptors/NeogeoDescriptors.h"
#include "gamepad/descriptors/MDMiniDescriptors.h"
#include "gamepad/descriptors/PCEngineDescriptors.h"
#include "gamepad/descriptors/EgretDescriptors.h"
#include "gamepad/descriptors/AstroDescriptors.h"
#include "gamepad/descriptors/PSClassicDescriptors.h"
#include "gamepad/descriptors/XboxOriginalDescriptors.h"
#include "gamepad/descriptors/XBOneDescriptors.h"

#include "pico/stdlib.h"

#include "config.pb.h"

// MUST BE DEFINED FOR MPG
extern uint32_t getMillis();
extern uint64_t getMicro();

#define GAMEPAD_FEATURE_REPORT_SIZE 32

struct GamepadButtonMapping
{
	GamepadButtonMapping(Mask_t bm) :
		pinMask(0),
		buttonMask(bm)
	{}

	uint32_t pinMask;
	const uint16_t buttonMask;
};

#define GAMEPAD_DIGITAL_INPUT_COUNT 18 // Total number of buttons, including D-pad

class Gamepad {
public:
	Gamepad();

	void setup();
	void reinit();
	void process();
	void read();
	void save();
	void debounce();
	
	void hotkey();

	/**
	 * @brief Flag to indicate analog trigger support.
	 */
	bool hasAnalogTriggers {false};

	/**
	 * @brief Flag to indicate Left analog stick support.
	 */
	bool hasLeftAnalogStick {false};

	/**
	 * @brief Flag to indicate Right analog stick support.
	 */
	bool hasRightAnalogStick {false};

	void sendReportSuccess();
	void *getReport();
	uint16_t getReportSize();
	HIDReport *getHIDReport();
	SwitchReport *getSwitchReport();
	XInputReport *getXInputReport();
	XboxOneGamepad_Data_t *getXBOneReport();
	KeyboardReport *getKeyboardReport();
	PS4Report *getPS4Report();
	NeogeoReport *getNeogeoReport();
	MDMiniReport *getMDMiniReport();
	PCEngineReport *getPCEngineReport();
    EgretReport *getEgretReport();
    AstroReport *getAstroReport();
    PSClassicReport *getPSClassicReport();
    XboxOriginalReport *getXboxOriginalReport();

	uint8_t last_report[CFG_TUD_ENDPOINT0_SIZE] = { };
	uint8_t last_report_counter = 0;
	uint16_t last_axis_counter = 0;

	/**
	 * @brief Check for a button press. Used by `pressed[Button]` helper methods.
	 */
	inline bool __attribute__((always_inline)) pressedButton(const uint16_t mask) {
		return (state.buttons & mask) == mask;
	}

	/**
	 * @brief Check for a dpad press. Used by `pressed[Dpad]` helper methods.
	 */
	inline bool __attribute__((always_inline)) pressedDpad(const uint8_t mask) { return (state.dpad & mask) == mask; }

	/**
	 * @brief Check for an aux button press. Same idea as `pressedButton`.
	 */
	inline bool __attribute__((always_inline)) pressedAux(const uint16_t mask) {
		return (state.aux & mask) == mask;
	}

	/**
	 * @brief Check for a hotkey combination press. Checks aux, buttons, and dpad.
	 */
	inline bool __attribute__((always_inline)) pressedHotkey(const HotkeyEntry hotkey) {
		return (hotkey.action != 0 && pressedButton(hotkey.buttonsMask) &&
				pressedDpad(hotkey.dpadMask) && pressedAux(hotkey.auxMask));
	}

	/**
	 * @brief Remote hotkey bits from the state bitmask and provide pressed action.
	 */
	inline GamepadHotkey __attribute__((always_inline)) selectHotkey(const HotkeyEntry hotkey) {
		state.buttons &= ~(hotkey.buttonsMask);
		state.dpad &= ~(hotkey.dpadMask);
		return static_cast<GamepadHotkey>(hotkey.action);
	}

	inline bool __attribute__((always_inline)) pressedUp()    { return pressedDpad(GAMEPAD_MASK_UP); }
	inline bool __attribute__((always_inline)) pressedDown()  { return pressedDpad(GAMEPAD_MASK_DOWN); }
	inline bool __attribute__((always_inline)) pressedLeft()  { return pressedDpad(GAMEPAD_MASK_LEFT); }
	inline bool __attribute__((always_inline)) pressedRight() { return pressedDpad(GAMEPAD_MASK_RIGHT); }
	inline bool __attribute__((always_inline)) pressedB1()    { return pressedButton(GAMEPAD_MASK_B1); }
	inline bool __attribute__((always_inline)) pressedB2()    { return pressedButton(GAMEPAD_MASK_B2); }
	inline bool __attribute__((always_inline)) pressedB3()    { return pressedButton(GAMEPAD_MASK_B3); }
	inline bool __attribute__((always_inline)) pressedB4()    { return pressedButton(GAMEPAD_MASK_B4); }
	inline bool __attribute__((always_inline)) pressedL1()    { return pressedButton(GAMEPAD_MASK_L1); }
	inline bool __attribute__((always_inline)) pressedR1()    { return pressedButton(GAMEPAD_MASK_R1); }
	inline bool __attribute__((always_inline)) pressedL2()    { return pressedButton(GAMEPAD_MASK_L2); }
	inline bool __attribute__((always_inline)) pressedR2()    { return pressedButton(GAMEPAD_MASK_R2); }
	inline bool __attribute__((always_inline)) pressedS1()    { return pressedButton(GAMEPAD_MASK_S1); }
	inline bool __attribute__((always_inline)) pressedS2()    { return pressedButton(GAMEPAD_MASK_S2); }
	inline bool __attribute__((always_inline)) pressedL3()    { return pressedButton(GAMEPAD_MASK_L3); }
	inline bool __attribute__((always_inline)) pressedR3()    { return pressedButton(GAMEPAD_MASK_R3); }
	inline bool __attribute__((always_inline)) pressedA1()    { return pressedButton(GAMEPAD_MASK_A1); }
	inline bool __attribute__((always_inline)) pressedA2()    { return pressedButton(GAMEPAD_MASK_A2); }

	const GamepadOptions& getOptions() const { return options; }

	void setInputMode(InputMode inputMode) { options.inputMode = inputMode; }
	void setSOCDMode(SOCDMode socdMode) { options.socdMode = socdMode; }
	void setDpadMode(DpadMode dpadMode) { options.dpadMode = dpadMode; }

	GamepadDebouncer debouncer;
	GamepadState rawState;
	GamepadState state;
	GamepadState turboState;
	GamepadButtonMapping *mapDpadUp;
	GamepadButtonMapping *mapDpadDown;
	GamepadButtonMapping *mapDpadLeft;
	GamepadButtonMapping *mapDpadRight;
	GamepadButtonMapping *mapButtonB1;
	GamepadButtonMapping *mapButtonB2;
	GamepadButtonMapping *mapButtonB3;
	GamepadButtonMapping *mapButtonB4;
	GamepadButtonMapping *mapButtonL1;
	GamepadButtonMapping *mapButtonR1;
	GamepadButtonMapping *mapButtonL2;
	GamepadButtonMapping *mapButtonR2;
	GamepadButtonMapping *mapButtonS1;
	GamepadButtonMapping *mapButtonS2;
	GamepadButtonMapping *mapButtonL3;
	GamepadButtonMapping *mapButtonR3;
	GamepadButtonMapping *mapButtonA1;
	GamepadButtonMapping *mapButtonA2;
	GamepadButtonMapping *mapButtonFn;

	bool userRequestedReinit = false;

	inline static const SOCDMode resolveSOCDMode(const GamepadOptions& options) {
		 return (options.socdMode == SOCD_MODE_BYPASS &&
				 (options.inputMode == INPUT_MODE_HID ||
				  options.inputMode == INPUT_MODE_SWITCH ||
				  options.inputMode == INPUT_MODE_NEOGEO ||
				  options.inputMode == INPUT_MODE_PS4)) ?
				SOCD_MODE_NEUTRAL : options.socdMode;
	};

private:
	void releaseAllKeys(void);
	void pressKey(uint8_t code);
	uint8_t getModifier(uint8_t code);
	uint8_t getMultimedia(uint8_t code);
	void processHotkeyIfNewAction(GamepadHotkey action);

	GamepadOptions& options;
	const HotkeyOptions& hotkeyOptions;

	GamepadHotkey lastAction = HOTKEY_NONE;

	uint32_t keep_alive_timer;
	uint8_t keep_alive_sequence;
	uint8_t virtual_keycode_sequence;
	bool xb1_guide_pressed;
};

#endif
