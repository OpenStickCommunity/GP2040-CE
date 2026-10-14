#include "OutputManager.h"

#include "enums.pb.h"
#include "storagemanager.h"
#include "gamepad.h"
#include "drivers/hid/HIDDescriptors.h"

#ifdef ENABLE_BLUETOOTH
#include "BLEHIDManager.h"
#endif

void OutputManager::dispatch(Gamepad* gamepad) {
#ifdef ENABLE_BLUETOOTH
    const GamepadOptions& opts = Storage::getInstance().getGamepadOptions();
    if (opts.inputMode != INPUT_MODE_BLE) return;

    // Map GamepadState to the 7-byte BLE HID report body (no Report ID byte —
    // that is carried by the GATT Report Reference descriptor).
    //   byte 0   : Buttons 1..8   (B1, B2, B3, B4, L1, R1, L2, R2)
    //   byte 1   : Buttons 9..16  (S1, S2, L3, R3, Up, Down, Left, Right)
    //   byte 2   : Buttons 17..18 (A1, A2) + 6 padding bits
    //   byte 3-6 : X, Y, Z, Rz sticks — sent centered (no analog input yet)
    // Order follows the W3C Standard Gamepad layout; the D-pad is sent as discrete buttons.
    uint8_t report[7] = {};
    const GamepadState& state = gamepad->state;

    // Byte 0: Buttons 1..8
    report[0] =
        (gamepad->pressedB1() ? (1u << 0) : 0) |  // Button 1
        (gamepad->pressedB2() ? (1u << 1) : 0) |  // Button 2
        (gamepad->pressedB3() ? (1u << 2) : 0) |  // Button 3
        (gamepad->pressedB4() ? (1u << 3) : 0) |  // Button 4
        (gamepad->pressedL1() ? (1u << 4) : 0) |  // Button 5
        (gamepad->pressedR1() ? (1u << 5) : 0) |  // Button 6
        (gamepad->pressedL2() ? (1u << 6) : 0) |  // Button 7
        (gamepad->pressedR2() ? (1u << 7) : 0);   // Button 8

    // Byte 1: Buttons 9..16 (D-pad uses the SOCD-cleaned state)
    report[1] =
        (gamepad->pressedS1() ? (1u << 0) : 0) |               // Button 9
        (gamepad->pressedS2() ? (1u << 1) : 0) |               // Button 10
        (gamepad->pressedL3() ? (1u << 2) : 0) |               // Button 11
        (gamepad->pressedR3() ? (1u << 3) : 0) |               // Button 12
        ((state.dpad & GAMEPAD_MASK_UP)    ? (1u << 4) : 0) |  // Button 13
        ((state.dpad & GAMEPAD_MASK_DOWN)  ? (1u << 5) : 0) |  // Button 14
        ((state.dpad & GAMEPAD_MASK_LEFT)  ? (1u << 6) : 0) |  // Button 15
        ((state.dpad & GAMEPAD_MASK_RIGHT) ? (1u << 7) : 0);   // Button 16

    // Byte 2: Buttons 17..18 (bits 2..7 = padding = 0)
    report[2] =
        (gamepad->pressedA1() ? (1u << 0) : 0) |  // Button 17
        (gamepad->pressedA2() ? (1u << 1) : 0);   // Button 18

    // Bytes 3..6: sticks centered
    report[3] = 0x80;
    report[4] = 0x80;
    report[5] = 0x80;
    report[6] = 0x80;

    BLEHIDManager::getInstance().sendReport(report, sizeof(report));
#else
    (void)gamepad;
#endif
}
