#ifndef GPCOMMS_H_
#define GPCOMMS_H_

#include "enums.pb.h"
#include "types.h"
#include "gamepad.h"
#include "gamepad/GamepadState.h"

#ifndef GPCOMMS_DEFAULT_MODE
#define GPCOMMS_DEFAULT_MODE GP_COMMS_MODE_OFF
#endif

#ifndef GPCOMMS_DEFAULT_HWBLOCK
#define GPCOMMS_DEFAULT_HWBLOCK -1
#endif

#ifndef GPCOMMS_BUFFER_SIZE
#define GPCOMMS_BUFFER_SIZE 100
#endif

struct GPComms_Status {
	InputMode inputMode;
	int8_t turboRate; // Negative value indicate off

	bool macroEnabled: 1;
	DpadMode dpadMode: 2;
	SOCDMode socdMode: 3;
	// 2 bits left
};

struct GPComms_State {
	GamepadState gamepadState;
	Mask_t gpioState = 0;
};

struct GPComms_Message {
	uint16_t length = 0;
	char *message;
};

class GPComms {
public:
	static void readGamepad();
	static void processGamepad() { }
	static GamepadState *getGamepadState() { return &gamepadState; }
	static Mask_t getGpioState() { return gpioState; }
	static void handleBuffer(uint8_t *buf, int size);
	static uint8_t fillBufferStatus(uint8_t *buf, Gamepad *gamepad, const AddonOptions &addonOptions);
	static uint8_t fillBufferState(uint8_t *buf, Gamepad *gamepad);
	static uint8_t fillBufferMessage(uint8_t *buf, char *text, uint16_t textLength);
private:
	static void handleStatus(uint8_t *payload);
	static void handleState(uint8_t *payload);
	static void handleMessage(uint8_t *payload);
	static GamepadState gamepadState;
	static Mask_t gpioState;
};

#endif
