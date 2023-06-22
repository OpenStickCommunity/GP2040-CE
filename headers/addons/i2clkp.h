#ifndef _I2CLKP_H
#define _I2CLKP_H

#include <hardware/i2c.h>

#include "gpaddon.h"
#include "GamepadEnums.h"

#define I2CLKPName "I2CLKP"

#ifndef LKP_ENABLED
#define LKP_ENABLED 0
#endif

#ifndef LKP_I2C_BLOCK
#define LKP_I2C_BLOCK i2c0
#endif

#ifndef LKP_INTERRUPT_PIN
#define LKP_INTERRUPT_PIN -1
#endif

#ifndef LKP_ADDR
#define LKP_ADDR 0x08
#endif

#ifndef LKP_SDA_PIN
#define LKP_SDA_PIN -1
#endif

#ifndef LKP_SCL_PIN
#define LKP_SCL_PIN -1
#endif

#ifndef LKP_SPEED
#define LKP_SPEED 400000
#endif

class I2CLKPInput : public GPAddon {
	public:
	I2CLKPInput():
		i2c(nullptr), addr(0), keys{0}, versionCheckStart(0),
		probeFailed(false), connected(false) {}
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return I2CLKPName; }

	private:
	/**
	 * @brief Clear the interrupt.
	 * @return Whether the operation was successful or not.
	 */
	bool cli();
	/**
	 * @brief Initiate the device probing.
	 * Check device presence and query the version of the attached device.
	 * @return Whether the operation was successful or not.
	 */
	bool probe();
	/**
	 * @brief Read an 8-bit register value from a device that uses 16-bit big-endian addressing.
	 * @param u16Register Address of the register.
	 * @return The content of that register.
	 */
	int readReg(uint16_t u16Register);
	/**
	 * @brief Read an array of 8-bit registers from a device that uses 16-bit big-endian addressing.
	 * @param u16Register Address of the register.
	 * @param dest Destination buffer.
	 * @param len Size of the destination buffer and number of registers to read.
	 * @return Whether the option was successful or not.
	 */
	bool readReg(uint16_t u16Register, uint8_t* dest, size_t len);
	/**
	 * @brief Write an 8-bit register value to a device that uses 16-bit big-endian addressing.
	 * @param u16Register Address of the register.
	 * @param newval New value to write to the register.
	 * @return Whether the option was successful or not.
	 */
	bool writeReg(uint16_t u16Register, uint8_t newval);
	// Bulk writing is not really used so not implemented as a convenient method

  /** I2C instance. */
	i2c_inst_t *i2c;
	/** Cached device address. */
	int addr;
	/** Cached interrrupt pin number. */
	uint interruptPin;
	/** Most recent sensor state. */
	uint8_t keys[4];

  /** Version check start time. Used to detect timeouts. */
	uint32_t versionCheckStart;
	/** True if the probing failed last time. */
	bool probeFailed;
	/** True if device is connected. */
	bool connected;
};

#endif // _I2CLKP_H
