#ifndef GPCOMMS_H_
#define GPCOMMS_H_

#include "enums.pb.h"
#include "gamepad/GamepadState.h"

#ifndef GPCOMMS_BUFFER_SIZE
#define GPCOMMS_BUFFER_SIZE 100
#endif

typedef enum {
	GPCMD_UNKNOWN = 0x00,
	GPCMD_STATUS  = 0x01,
	GPCMD_STATE   = 0x02,
	GPCMD_MESSAGE = 0x03,
	GPCMD_ACK     = 0xFF,
} GPComms_Command;

template <typename T>
struct GPComms_Payload {
	uint8_t command;
	T data;
};

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
	Mask_t gpioState;
};

struct GPComms_Message {
	uint16_t length = 0;
	char *message;
};

class GPComms {
public:
	static GamepadState *getGamepadState() { return &gamepadState; }
	static Mask_t getGpioState() { return gpioState; }
	static void handleBuffer(uint8_t *buf, int size);
	static void updateGamepad();
private:
	static void handleStatus(GPComms_Status *gpStatus);
	static void handleState(GPComms_State *gpState);
	static void handleMessage(GPComms_Message *gpMessage);
	static GamepadState gamepadState;
	static Mask_t gpioState;
};

#endif
