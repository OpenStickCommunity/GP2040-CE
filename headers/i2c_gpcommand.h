#ifndef I2C_GPCOMMAND_H_
#define I2C_GPCOMMAND_H_

#include "enums.pb.h"
#include "gamepad/GamepadState.h"

typedef enum {
	GPCMD_UNKNOWN = 0x00,
	GPCMD_STATUS  = 0x01,
	GPCMD_STATE   = 0x02,
	GPCMD_MESSAGE = 0x03,
	GPCMD_ACK     = 0xFF,
} I2C_GPCommand;

struct I2C_GPStatus {
	InputMode inputMode;
	int8_t turboRate; // Negative value indicate off

	bool macroEnabled: 1;
	DpadMode dpadMode: 2;
	SOCDMode socdMode: 3;
	// 2 bits left
};

struct I2C_GPState {
	GamepadState gamepadState;
	Mask_t gpioState;
};

struct I2C_GPMessage {
	uint16_t length = 0;
	char *message;
};

#endif
