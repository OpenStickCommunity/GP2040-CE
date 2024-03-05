#ifndef GP_STATE_H_
#define GP_STATE_H_

#include "enums.pb.h"
#include "gamepad/GamepadState.h"

typedef enum {
	GPCMD_UNKNOWN = 0x00,
	GPCMD_STATUS  = 0x01,
	GPCMD_STATE   = 0x02,
	GPCMD_MESSAGE = 0x03,
	GPCMD_ACK     = 0xFF,
} GPCommand;

struct GPStatus {
	InputMode inputMode;
	int8_t turboRate; // Negative value indicate off

	bool macroEnabled: 1;
	DpadMode dpadMode: 2;
	SOCDMode socdMode: 3;
	// 2 bits left
};
static size_t GP_STATUS_SIZE = sizeof(GPStatus);

struct GPState {
// struct __attribute__ ((packed)) GPState {
	GamepadState gamepadState;
	Mask_t gpioState;
};
static size_t GP_STATE_SIZE = sizeof(GPState);

struct GPMessage {
	size_t length = 0;
	char *message;
};
static size_t GP_MESSAGE_SIZE = sizeof(GPMessage);

#endif
