/*
 * SPDX-License-Identifier: MIT
 * Switch Pro Controller Bluetooth driver for GP2040-CE
 * Ported from retro-pico-switch by DavidPagels
 */

#ifndef SWITCH_BLUETOOTH_DRIVER_H
#define SWITCH_BLUETOOTH_DRIVER_H

#include <stdint.h>

// D-pad mode enum (matches GP2040 config)
#define SWBT_DPAD_MODE_DIGITAL      0
#define SWBT_DPAD_MODE_LEFT_ANALOG  1
#define SWBT_DPAD_MODE_RIGHT_ANALOG 2

// Simple input struct that doesn't conflict with TinyUSB
typedef struct {
    uint16_t buttons;  // Button bitmask (GP2040 format)
    uint8_t dpad;      // D-pad (GP2040 format)
    uint8_t dpadMode;  // D-pad mode from config
    uint16_t lx, ly;   // Left stick (0-65535)
    uint16_t rx, ry;   // Right stick (0-65535)
} SwitchBTInput;

// GP2040 button masks (copy to avoid including gamepad.h)
#define SWBT_GAMEPAD_MASK_B1    (1U << 0)
#define SWBT_GAMEPAD_MASK_B2    (1U << 1)
#define SWBT_GAMEPAD_MASK_B3    (1U << 2)
#define SWBT_GAMEPAD_MASK_B4    (1U << 3)
#define SWBT_GAMEPAD_MASK_L1    (1U << 4)
#define SWBT_GAMEPAD_MASK_R1    (1U << 5)
#define SWBT_GAMEPAD_MASK_L2    (1U << 6)
#define SWBT_GAMEPAD_MASK_R2    (1U << 7)
#define SWBT_GAMEPAD_MASK_S1    (1U << 8)
#define SWBT_GAMEPAD_MASK_S2    (1U << 9)
#define SWBT_GAMEPAD_MASK_L3    (1U << 10)
#define SWBT_GAMEPAD_MASK_R3    (1U << 11)
#define SWBT_GAMEPAD_MASK_A1    (1U << 12)
#define SWBT_GAMEPAD_MASK_A2    (1U << 13)

#define SWBT_GAMEPAD_MASK_UP    (1U << 0)
#define SWBT_GAMEPAD_MASK_DOWN  (1U << 1)
#define SWBT_GAMEPAD_MASK_LEFT  (1U << 2)
#define SWBT_GAMEPAD_MASK_RIGHT (1U << 3)

// Connection states
enum class SwitchBTState {
    IDLE,
    INITIALIZING,
    DISCOVERABLE,
    RECONNECTING,
    SLEEPING,       // Low power - waiting for button press
    CONNECTED
};

#ifdef __cplusplus
extern "C" {
#endif

// Initialize Bluetooth - call once at startup
void switchbt_init(void);

// Process loop - call every frame with input state
// Returns true if data was sent
bool switchbt_process(const SwitchBTInput* input);

// Get current connection state
SwitchBTState switchbt_get_state(void);

// Check if connected
bool switchbt_is_connected(void);

// Clear stored pairing (to pair with a different Switch)
void switchbt_clear_pairing(void);

#ifdef __cplusplus
}
#endif

#endif // SWITCH_BLUETOOTH_DRIVER_H
