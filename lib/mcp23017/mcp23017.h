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
    MCP23017(i2c_inst_t* i2c, uint8_t addr);
    
    bool init();                    // Initialize chip
    void setPinMode(uint8_t pin, bool input);
    void setPullup(uint8_t pin, bool enable);
    bool readPin(uint8_t pin);
    uint16_t readAll();             // Read all 16 pins
    uint8_t readRegister(uint8_t reg);
    
private:
    i2c_inst_t* i2c_;
    uint8_t addr_;
    
    void writeRegister(uint8_t reg, uint8_t value);
};

#endif // MCP23017_H
