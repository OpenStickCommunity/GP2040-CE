#include "drivers/ps5/PS5Auth.h"
#include "drivers/ps5/PS5AuthUSBListener.h"
#include "peripheralmanager.h"

void PS5Auth::initialize() {
    if ( !available() ) {
        return;
    }

    ps5AuthData.dongle_ready = false; // Dongle is connected!
    ps5AuthData.hash_pending = false; // AES-CMAC Hash Pending (sent to Dongle)
    ps5AuthData.hash_ready = false; // AES-CMAC Hash Ready
    ps5AuthData.pair_ready = false; // BT MAC Pairing ready
    ps5AuthData.keys_ready = false; // Mayflash S5 keys ready
    ps5AuthData.ps5_passthrough_state = PS5AuthState::ps5_auth_idle;
    memset(ps5AuthData.auth_buffer, 0, sizeof(ps5AuthData.auth_buffer));
    memset(ps5AuthData.MAC_pair_report, 0, sizeof(ps5AuthData.MAC_pair_report));
    mutex_init(&ps5AuthData.hash_mutex);
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
