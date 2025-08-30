#ifndef _GAMEPAD_H_
#define _GAMEPAD_H_

#include "BoardConfig.h"
#include "types.h"
#include <string.h>
#include <string>

#include "enums.pb.h"
#include "gamepad/GamepadState.h"
#include "gamepad/GamepadAuxState.h"

#include "pico/stdlib.h"

#include "config.pb.h"

// MUST BE DEFINED FOR MPG
extern uint32_t getMillis();
extern uint64_t getMicro();

struct GamepadButtonMapping
{
	GamepadButtonMapping(Mask_t bm) :
		pinMask(0),
		buttonMask(bm)
	{}

	uint32_t pinMask;
	const uint32_t buttonMask;
};

class Gamepad {
public:
	Gamepad();

	void setup();
	void reinit();
	void process();
	void read();
	void save();

	void hotkey();
	void clearState();
	void clearRumbleState();

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

	/**
	 * @brief Check for a button press. Used by `pressed[Button]` helper methods.
	 */
	inline bool __attribute__((always_inline)) pressedButton(const uint32_t mask) {
		return (state.buttons & mask) == mask;
	}

	/**
	 * @brief Check for a dpad press. Used by `pressed[Dpad]` helper methods.
	 */
	inline bool __attribute__((always_inline)) pressedDpad(const uint8_t mask) {
		return (state.dpad & mask) == mask;
	}

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
	 * @brief Remove hotkey bits from the state bitmask and provide pressed action.
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
	inline bool __attribute__((always_inline)) pressedA3()    { return pressedButton(GAMEPAD_MASK_A3); }
	inline bool __attribute__((always_inline)) pressedA4()    { return pressedButton(GAMEPAD_MASK_A4); }
	inline bool __attribute__((always_inline)) pressedE1()    { return pressedButton(GAMEPAD_MASK_E1); }
	inline bool __attribute__((always_inline)) pressedE2()    { return pressedButton(GAMEPAD_MASK_E2); }
	inline bool __attribute__((always_inline)) pressedE3()    { return pressedButton(GAMEPAD_MASK_E3); }
	inline bool __attribute__((always_inline)) pressedE4()    { return pressedButton(GAMEPAD_MASK_E4); }
	inline bool __attribute__((always_inline)) pressedE5()    { return pressedButton(GAMEPAD_MASK_E5); }
	inline bool __attribute__((always_inline)) pressedE6()    { return pressedButton(GAMEPAD_MASK_E6); }
	inline bool __attribute__((always_inline)) pressedE7()    { return pressedButton(GAMEPAD_MASK_E7); }
	inline bool __attribute__((always_inline)) pressedE8()    { return pressedButton(GAMEPAD_MASK_E8); }
	inline bool __attribute__((always_inline)) pressedE9()    { return pressedButton(GAMEPAD_MASK_E9); }
	inline bool __attribute__((always_inline)) pressedE10()   { return pressedButton(GAMEPAD_MASK_E10); }
	inline bool __attribute__((always_inline)) pressedE11()   { return pressedButton(GAMEPAD_MASK_E11); }
	inline bool __attribute__((always_inline)) pressedE12()   { return pressedButton(GAMEPAD_MASK_E12); }

	const GamepadOptions& getOptions() const { return options; }
	const DpadMode getActiveDpadMode() { return activeDpadMode; }

	void setInputMode(InputMode inputMode) { options.inputMode = inputMode; }
	void setSOCDMode(SOCDMode socdMode) { options.socdMode = socdMode; }
	void setDpadMode(DpadMode dpadMode) { options.dpadMode = dpadMode; }

	GamepadState state;
	GamepadState turboState;
	GamepadAuxState auxState;
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
	GamepadButtonMapping *mapButtonA3;
	GamepadButtonMapping *mapButtonA4;
	GamepadButtonMapping *mapButtonE1;
	GamepadButtonMapping *mapButtonE2;
	GamepadButtonMapping *mapButtonE3;
	GamepadButtonMapping *mapButtonE4;
	GamepadButtonMapping *mapButtonE5;
	GamepadButtonMapping *mapButtonE6;
	GamepadButtonMapping *mapButtonE7;
	GamepadButtonMapping *mapButtonE8;
	GamepadButtonMapping *mapButtonE9;
	GamepadButtonMapping *mapButtonE10;
	GamepadButtonMapping *mapButtonE11;
	GamepadButtonMapping *mapButtonE12;
	GamepadButtonMapping *mapButtonFn;
	GamepadButtonMapping *mapButtonDP;
	GamepadButtonMapping *mapButtonLS;
	GamepadButtonMapping *mapButtonRS;
	GamepadButtonMapping *mapDigitalUp;
	GamepadButtonMapping *mapDigitalDown;
	GamepadButtonMapping *mapDigitalLeft;
	GamepadButtonMapping *mapDigitalRight;
	GamepadButtonMapping *mapAnalogLSXNeg;
	GamepadButtonMapping *mapAnalogLSXPos;
	GamepadButtonMapping *mapAnalogLSYNeg;
	GamepadButtonMapping *mapAnalogLSYPos;
	GamepadButtonMapping *mapAnalogRSXNeg;
	GamepadButtonMapping *mapAnalogRSXPos;
	GamepadButtonMapping *mapAnalogRSYNeg;
	GamepadButtonMapping *mapAnalogRSYPos;
	GamepadButtonMapping *map48WayMode;

	// gamepad specific proxy of debounced buttons --- 1 = active (inverse of the raw GPIO)
	// see GP2040::debounceGpioGetAll for details
	Mask_t debouncedGpio;

	bool userRequestedReinit = false;

	// These are special to SOCD
	inline static const SOCDMode resolveSOCDMode(const GamepadOptions& options) {
		return (options.socdMode == SOCD_MODE_BYPASS &&
				(options.inputMode == INPUT_MODE_PS3 ||
				options.inputMode == INPUT_MODE_SWITCH ||
				options.inputMode == INPUT_MODE_NEOGEO ||
				options.inputMode == INPUT_MODE_PS4)) ?
			SOCD_MODE_NEUTRAL : options.socdMode;
	};

private:

	uint8_t getModifier(uint8_t code);
	uint8_t getMultimedia(uint8_t code);
	void processHotkeyAction(GamepadHotkey action);

	GamepadOptions & options;
	DpadMode activeDpadMode;
	bool map48WayModeToggle;
	const HotkeyOptions & hotkeyOptions;

	HotkeyEntry hotkeys[16];
	GamepadHotkey lastAction = HOTKEY_NONE;
};

#endif
