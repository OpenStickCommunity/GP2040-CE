#include <pico/stdlib.h>
#include <hardware/gpio.h>

#include "addons/i2clkp.h"
#include "storagemanager.h"
#include "helper.h"
#include "config.pb.h"

#define LKP_PROTO_VER_MAJOR 1
#define LKP_PROTO_VER_MINOR 0
#define LKP_PROTO_VER_REV 0

#define LKP_I2C_BREAK_US 100

// RW registers
#define LKP_REG_CTL 0x0000
#define LKP_REG_CTL_SET 0x0001
#define LKP_REG_CTL_CLEAR 0x0002
#define LKP_REG_CTL_TOGGLE 0x0003

#define LKP_REG_CTL_INTR_EN 0b00000010
#define LKP_REG_CTL_INTR_TRIG 0b10000000

// RO registers
#define LKP_REG_VER 0x0100
#define LKP_REG_VER_MAJOR LKP_REG_VER
#define LKP_REG_VER_MINOR (LKP_REG_VER + 1)
#define LKP_REG_VER_REV (LKP_REG_VER + 2)

#define LKP_REG_KEYS 0x0104
#define LKP_REG_KEY0 LKP_REG_KEYS
#define LKP_REG_KEY1 (LKP_REG_KEYS + 1)
#define LKP_REG_KEY2 (LKP_REG_KEYS + 2)
#define LKP_REG_KEY3 (LKP_REG_KEYS + 3)

#define LKP_REG_KEYS_ANALOG_BASE 0x0108
#define LKP_REG_KEYS_ANALOG_SIZE 32

static uint8_t reverse8(uint8_t bits) {
	uint8_t rev = 0;
	for (int _=0; _<8; _++) {
		rev <<= 1;
		rev |= bits & 1;
		bits >>= 1;
	}
	return rev;
}

static inline void fillReg16(uint8_t* dest, uint16_t reg) {
	dest[0] = reg >> 8;
	dest[1] = reg & 0xff;
}

int I2CLKPInput::readReg(uint16_t u16Register) {
	uint8_t u16RegisterBE[2];
	uint8_t reg;

	fillReg16(u16RegisterBE, u16Register);
	auto rc = i2c_write_timeout_per_char_us(
		this->i2c, this->addr, u16RegisterBE, sizeof(u16RegisterBE), true, LKP_I2C_BREAK_US
	);
	if (rc == sizeof(u16RegisterBE)) {
		rc = i2c_read_timeout_per_char_us(
			this->i2c, this->addr, &reg, sizeof(reg), false, LKP_I2C_BREAK_US
		);
	}
	return rc == sizeof(reg) ? reg : -1;
}

bool I2CLKPInput::readReg(uint16_t u16Register, uint8_t* dest, size_t len) {
	uint8_t u16RegisterBE[2];

	fillReg16(u16RegisterBE, u16Register);
	auto rc = i2c_write_timeout_per_char_us(
		this->i2c, this->addr, u16RegisterBE, sizeof(u16RegisterBE), true, LKP_I2C_BREAK_US
	);
	if (rc == sizeof(u16RegisterBE)) {
		rc = i2c_read_timeout_per_char_us(this->i2c, this->addr, dest, len, false, LKP_I2C_BREAK_US);
	}
	return rc == len;
}

bool I2CLKPInput::writeReg(uint16_t u16Register, uint8_t newval) {
	uint8_t scratchPad[3];
	fillReg16(scratchPad, u16Register);
	scratchPad[2] = newval;
	auto rc = i2c_write_timeout_per_char_us(
		i2c, addr, scratchPad, sizeof(scratchPad), false, LKP_I2C_BREAK_US
	);
	return rc == sizeof(scratchPad);
}

bool I2CLKPInput::cli() {
	auto result = this->writeReg(LKP_REG_CTL_CLEAR, LKP_REG_CTL_INTR_TRIG);

	this->connected = result;
	return result;
}

bool I2CLKPInput::probe() {
	uint8_t ver[3];
	auto result = false;

	if (this->connected) {
		return true;
	}

	do {
		// Check timeout
		if (this->probeFailed && (getMillis() - this->versionCheckStart) < 100) {
			break;
		}

		this->versionCheckStart = getMillis();
		this->probeFailed = false;

		// Read version register
		if (!this->readReg(LKP_REG_VER, ver, sizeof(ver))) {
			this->probeFailed = true;
			break;
		}

		// For major changes, reject when they are different.
		// For minor changes, reject when we demand newer features that the device couldn't handle.
		if (ver[0] != LKP_PROTO_VER_MAJOR || ver[1] < LKP_PROTO_VER_MINOR) {
			this->probeFailed = true;
			break;
		}

		// After version check passes, enable scan
		if (!this->writeReg(LKP_REG_CTL, LKP_REG_CTL_INTR_EN)) {
			this->probeFailed = true;
			break;
		}

		result = true;
	} while (false);

	this->connected = result;
	return result;
}

bool I2CLKPInput::available() {
	const DisplayOptions& displayOptions = Storage::getInstance().getDisplayOptions();
	const auto& options = Storage::getInstance().getAddonOptions().lkpOptions;

	return (!displayOptions.enabled) &&
		options.enabled &&
		isValidPin(options.interruptPin) &&
		options.has_i2cAddress &&
		options.has_i2cBlock &&
		isValidPin(options.i2cSCLPin) &&
		isValidPin(options.i2cSDAPin) &&
		options.has_i2cSpeed;
}

void I2CLKPInput::setup() {
	const auto& options = Storage::getInstance().getAddonOptions().lkpOptions;

	// Setup addon parameters
	this->addr = options.i2cAddress;
	this->interruptPin = options.interruptPin;
	this->i2c = options.i2cBlock == 0 ? i2c0 : i2c1;

	// Setup INT pin
	gpio_init(this->interruptPin);
	gpio_set_dir(this->interruptPin, GPIO_IN);
	gpio_pull_up(this->interruptPin);

	// Ensure i2c block is initialized
	// This should not be required once #293 (specifically the global serial block allocation part)
	// is finished.
	i2c_init(this->i2c, options.i2cSpeed);
	gpio_set_function(options.i2cSDAPin, GPIO_FUNC_I2C);
	gpio_set_function(options.i2cSCLPin, GPIO_FUNC_I2C);
	gpio_pull_up(options.i2cSDAPin);
	gpio_pull_up(options.i2cSCLPin);

	// Start probing
	this->probe();
}

void I2CLKPInput::process() {
	// Check for connection and read the sensor states if changed.
	if (!this->connected && !this->probe()) {
		return;
	}

	if (!gpio_get(this->interruptPin)) {
		this->connected = this->readReg(LKP_REG_KEY0, this->keys, sizeof(this->keys)) && this->cli();
	}

	// Emit hori stick push
	// LKP uses left-to-right for consistency with the serial slider,
	// while hori uses right-to-left so they need to be reversed.
	auto* gamepad = Storage::getInstance().GetGamepad();
	gamepad->state.lx = 0x8000 ^ (reverse8(this->keys[3]) << 8);
	gamepad->state.ly = 0x8000 ^ (reverse8(this->keys[2]) << 8);
	gamepad->state.rx = 0x8000 ^ (reverse8(this->keys[1]) << 8);
	gamepad->state.ry = 0x8000 ^ (reverse8(this->keys[0]) << 8);

	// TODO add more modes.
}
