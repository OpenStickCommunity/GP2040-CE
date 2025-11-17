#include "mcp23017.h"
#include "pico/time.h"

MCP23017::MCP23017(i2c_inst_t* i2c, uint8_t addr) 
    : i2c_(i2c), addr_(addr) {}

bool MCP23017::init() {
    // Configure IOCON register first to ensure we're in the right mode
    // IOCON.BANK = 0 (sequential register addresses)
    // IOCON.SEQOP = 0 (sequential operation enabled)
    writeRegister(MCP23017_IOCON, 0x00);
    sleep_ms(10);  // Small delay for chip to process
    
    // Configure all pins on Port A as inputs with pull-ups
    writeRegister(MCP23017_IODIRA, 0xFF);  // All inputs
    sleep_ms(1);
    
    // Enable pull-ups on Port A
    writeRegister(MCP23017_GPPUA, 0xFF);   // All pull-ups enabled
    sleep_ms(1);
    
    // Verify the configuration by reading back
    uint8_t verifyDir = readRegister(MCP23017_IODIRA);
    uint8_t verifyPullup = readRegister(MCP23017_GPPUA);
    
    if (verifyDir != 0xFF || verifyPullup != 0xFF) {
        return false;  // MCP23017 not configured correctly
    }
    
    // Port B unused (all inputs, no pull-ups)
    writeRegister(MCP23017_IODIRB, 0xFF);
    writeRegister(MCP23017_GPPUB, 0x00);
    
    return true;
}

void MCP23017::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t buf[2] = {reg, value};
    i2c_write_blocking(i2c_, addr_, buf, 2, false);
}

uint8_t MCP23017::readRegister(uint8_t reg) {
    uint8_t value = 0xFF;  // Default to all high (unpulled state)
    int result = i2c_write_blocking(i2c_, addr_, &reg, 1, true);
    if (result == 1) {
        i2c_read_blocking(i2c_, addr_, &value, 1, false);
    }
    return value;
}

uint16_t MCP23017::readAll() {
    uint8_t portA = readRegister(MCP23017_GPIOA);
    uint8_t portB = readRegister(MCP23017_GPIOB);
    return (portB << 8) | portA;
}

bool MCP23017::readPin(uint8_t pin) {
    if (pin < 8) {
        uint8_t portA = readRegister(MCP23017_GPIOA);
        return (portA & (1 << pin)) != 0;
    } else {
        uint8_t portB = readRegister(MCP23017_GPIOB);
        return (portB & (1 << (pin - 8))) != 0;
    }
}
