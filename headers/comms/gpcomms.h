#ifndef GPCOMMS_H_
#define GPCOMMS_H_

#include "enums.pb.h"
#include "gamepad/GamepadState.h"

typedef enum {
	GPCMD_UNKNOWN = 0x00,
	GPCMD_STATUS  = 0x01,
	GPCMD_STATE   = 0x02,
	GPCMD_MESSAGE = 0x03,
	GPCMD_ACK     = 0xFF,
} GPComms_Command;

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

#endif
