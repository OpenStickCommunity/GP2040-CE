#include "drivers/ps5/PS5Auth.h"
#include "drivers/ps5/PS5AuthUSBListener.h"
#include "peripheralmanager.h"

void PS5Auth::initialize() {
    if ( !available() ) {
        return;
    }

    ps5AuthData.dongle_ready = false;   // Dongle is ready! (S5 key auth'd, P5G works right away)
    ps5AuthData.dongle_mounted = false; // Dongle is mounted (not all dongles are ready immediately)
    ps5AuthData.hash_pending = false;   // AES-CMAC Hash Pending (raw IO input sent to Dongle)
    ps5AuthData.hash_ready = false;     // AES-CMAC Hash Ready (Dongle returned with signed buffer)
    ps5AuthData.pair_ready = false;     // BT MAC Pairing ready (S5 only)
    ps5AuthData.auth_frame_id = 0;      // Which frame are we authing? (send by PS5)
    ps5AuthData.ps5_auth_state = PS5AuthState::ps5_auth_idle;
    memset(ps5AuthData.auth_f1_buffer, 0, sizeof(ps5AuthData.auth_f1_buffer)); // Dongle Struct
    ps5AuthData.auth_f1_ready = false;
    ps5AuthData.auth_f1_get_index = 0;
    memset(ps5AuthData.console_f0_buffer, 0, sizeof(ps5AuthData.console_f0_buffer)); // Console Struct
    ps5AuthData.console_f0_get_index = 0;
    memset(ps5AuthData.MAC_pair_report, 0, sizeof(ps5AuthData.MAC_pair_report));
    listener = new PS5AuthUSBListener();
    ((PS5AuthUSBListener*)listener)->setup();
    ((PS5AuthUSBListener*)listener)->setAuthData(&ps5AuthData);
}

bool PS5Auth::available() {
    return PeripheralManager::getInstance().isUSBEnabled(0);
}

void PS5Auth::process() {
    ((PS5AuthUSBListener*)listener)->process();
}
