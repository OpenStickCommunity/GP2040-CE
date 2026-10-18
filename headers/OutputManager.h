#ifndef OUTPUT_MANAGER_H
#define OUTPUT_MANAGER_H

class Gamepad;

class OutputManager {
public:
    // Dispatch a gamepad state update to the active wireless output transport.
    // Only performs work when ENABLE_BLUETOOTH is defined and the active input
    // mode is INPUT_MODE_BLE. USB dispatch remains in gp2040.cpp via DriverManager.
    static void dispatch(Gamepad* gamepad);
};

#endif // OUTPUT_MANAGER_H
