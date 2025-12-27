/**
 * @file mcp23017.h
 * @brief Driver for MCP23017 I2C GPIO Expander
 *
 * Provides simple interface for reading switches connected to MCP23017.
 * Used by turbo add-on for hardware per-button turbo controls.
 *
 * Hardware: MCP23017 16-bit I/O expander
 * I2C Address: Configurable via A0-A2 pins (default 0x20)
 * Pins Used: Port A (GPA0-GPA7) for 8 switches
 *
 * Configuration:
 * - Port A configured as inputs with pull-up resistors enabled
 * - Switches should be wired active-low (closed = LOW, open = HIGH)
 * - Port B unused (configured as inputs, no pull-ups)
 *
 * Register Mode: IOCON.BANK = 0 (sequential addressing mode)
 */

#ifndef MCP23017_H
#define MCP23017_H

#include "hardware/i2c.h"
#include <stdint.h>

// MCP23017 Register Addresses (IOCON.BANK = 0, sequential mode)
#define MCP23017_IODIRA   0x00  // I/O direction A
#define MCP23017_IODIRB   0x01  // I/O direction B
#define MCP23017_IPOLA    0x02  // Input polarity A
#define MCP23017_IPOLB    0x03  // Input polarity B
#define MCP23017_GPINTENA 0x04  // Interrupt-on-change A
#define MCP23017_GPINTENB 0x05  // Interrupt-on-change B
#define MCP23017_DEFVALA  0x06  // Default value A
#define MCP23017_DEFVALB  0x07  // Default value B
#define MCP23017_INTCONA  0x08  // Interrupt control A
#define MCP23017_INTCONB  0x09  // Interrupt control B
#define MCP23017_IOCON    0x0A  // I/O expander configuration
#define MCP23017_GPPUA    0x0C  // Pull-up A
#define MCP23017_GPPUB    0x0D  // Pull-up B
#define MCP23017_INTFA    0x0E  // Interrupt flag A
#define MCP23017_INTFB    0x0F  // Interrupt flag B
#define MCP23017_INTCAPA  0x10  // Interrupt captured value A
#define MCP23017_INTCAPB  0x11  // Interrupt captured value B
#define MCP23017_GPIOA    0x12  // Port A
#define MCP23017_GPIOB    0x13  // Port B
#define MCP23017_OLATA    0x14  // Output latch A
#define MCP23017_OLATB    0x15  // Output latch B

class MCP23017 {
public:
    /**
     * @brief Construct a new MCP23017 driver instance
     * @param i2c Pointer to I2C hardware instance (i2c0 or i2c1)
     * @param addr I2C device address (default 0x20)
     */
    MCP23017(i2c_inst_t* i2c, uint8_t addr);

    /**
     * @brief Initialize the MCP23017 chip
     *
     * Configures Port A as inputs with pull-ups enabled.
     * Port B is configured as inputs with no pull-ups (unused).
     *
     * @return true if initialization successful, false otherwise
     */
    bool init();

    /**
     * @brief Set pin direction (input/output)
     * @param pin Pin number (0-15)
     * @param input true for input, false for output
     */
    void setPinMode(uint8_t pin, bool input);

    /**
     * @brief Enable/disable pull-up resistor on a pin
     * @param pin Pin number (0-15)
     * @param enable true to enable pull-up, false to disable
     */
    void setPullup(uint8_t pin, bool enable);

    /**
     * @brief Read state of a single pin
     * @param pin Pin number (0-15)
     * @return true if pin is HIGH, false if LOW
     */
    bool readPin(uint8_t pin);

    /**
     * @brief Read all 16 pins at once
     * @return 16-bit value with pin states (bit 0 = GPA0, bit 15 = GPB7)
     */
    uint16_t readAll();

    /**
     * @brief Read a MCP23017 register directly
     * @param reg Register address
     * @return Register value
     */
    uint8_t readRegister(uint8_t reg);
    
private:
    i2c_inst_t* i2c_;
    uint8_t addr_;
    
    void writeRegister(uint8_t reg, uint8_t value);
};

#endif // MCP23017_H
