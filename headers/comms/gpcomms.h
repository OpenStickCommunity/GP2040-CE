#ifndef GPCOMMS_H_
#define GPCOMMS_H_

#include "enums.pb.h"
#include "types.h"
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

typedef enum {
	GPCMD_UNKNOWN = 0x00,
	GPCMD_STATE   = 0x01,
	GPCMD_STATUS  = 0x02,
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
	static void readGamepad();
	static void processGamepad() { }
	static GamepadState *getGamepadState() { return &gamepadState; }
	static Mask_t getGpioState() { return gpioState; }
	static void handleBuffer(uint8_t *buf, int size);
private:
	static void handleStatus(uint8_t *payload);
	static void handleState(uint8_t *payload);
	static void handleMessage(uint8_t *payload);
	static GamepadState gamepadState;
	static Mask_t gpioState;
};

#endif
