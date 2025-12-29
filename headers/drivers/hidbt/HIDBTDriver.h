/*
 * SPDX-License-Identifier: MIT
 * Generic HID Bluetooth Gamepad driver
 */

#ifndef HIDBT_DRIVER_H
#define HIDBT_DRIVER_H

#include <stdint.h>
#include <stdbool.h>

// D-pad mode enum (matches GP2040 config)
#define HIDBT_DPAD_MODE_DIGITAL      0
#define HIDBT_DPAD_MODE_LEFT_ANALOG  1
#define HIDBT_DPAD_MODE_RIGHT_ANALOG 2

// Input structure matching GP2040 gamepad state
struct HIDBTInput {
    uint32_t buttons;
    uint8_t dpad;
    uint8_t dpadMode;  // D-pad mode from config
    uint16_t lx;
    uint16_t ly;
    uint16_t rx;
    uint16_t ry;
};

// Button masks (matching GP2040 gamepad masks)
#define HIDBT_GAMEPAD_MASK_B1     (1U << 0)
#define HIDBT_GAMEPAD_MASK_B2     (1U << 1)
#define HIDBT_GAMEPAD_MASK_B3     (1U << 2)
#define HIDBT_GAMEPAD_MASK_B4     (1U << 3)
#define HIDBT_GAMEPAD_MASK_L1     (1U << 4)
#define HIDBT_GAMEPAD_MASK_R1     (1U << 5)
#define HIDBT_GAMEPAD_MASK_L2     (1U << 6)
#define HIDBT_GAMEPAD_MASK_R2     (1U << 7)
#define HIDBT_GAMEPAD_MASK_S1     (1U << 8)
#define HIDBT_GAMEPAD_MASK_S2     (1U << 9)
#define HIDBT_GAMEPAD_MASK_L3     (1U << 10)
#define HIDBT_GAMEPAD_MASK_R3     (1U << 11)
#define HIDBT_GAMEPAD_MASK_A1     (1U << 12)
#define HIDBT_GAMEPAD_MASK_A2     (1U << 13)

// D-pad masks
#define HIDBT_GAMEPAD_MASK_UP     (1U << 0)
#define HIDBT_GAMEPAD_MASK_DOWN   (1U << 1)
#define HIDBT_GAMEPAD_MASK_LEFT   (1U << 2)
#define HIDBT_GAMEPAD_MASK_RIGHT  (1U << 3)

// Connection states
enum class HIDBTState {
    IDLE,
    INITIALIZING,
    DISCOVERABLE,
    RECONNECTING,
    CONNECTED,
    SLEEPING
};

// Public API
void hidbt_init(void);
bool hidbt_process(const HIDBTInput* input);
HIDBTState hidbt_get_state(void);
bool hidbt_is_connected(void);
void hidbt_clear_pairing(void);

#endif // HIDBT_DRIVER_H


