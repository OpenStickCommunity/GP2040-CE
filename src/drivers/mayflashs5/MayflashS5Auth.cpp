#include "drivers/mayflashs5/MayflashS5Auth.h"
#include "drivers/mayflashs5/MayflashS5AuthUSBListener.h"
#include "peripheralmanager.h"

void MayflashS5Auth::initialize() {
    if ( !available() ) {
        return;
    }

    ps5AuthData.dongle_ready = false;   // Dongle is ready! (S5 key auth'd, P5G works right away)
    ps5AuthData.dongle_mounted = false; // Dongle is mounted (not all dongles are ready immediately)
    ps5AuthData.pair_info_ready = false;
    ps5AuthData.init_stage = 0;
    ps5AuthData.hash_pending = false;   // AES-CMAC Hash Pending (raw IO input sent to Dongle)
    ps5AuthData.hash_ready = false;     // AES-CMAC Hash Ready (Dongle returned with signed buffer)
    ps5AuthData.hash_request_count = 0;
    ps5AuthData.hash_response_count = 0;
    ps5AuthData.console_report_count = 0;
    ps5AuthData.auth_console_f0_raw_count = 0;
    ps5AuthData.auth_console_f0_count = 0;
    ps5AuthData.auth_console_f0_crc_valid_count = 0;
    ps5AuthData.auth_console_f0_submit_count = 0;
    ps5AuthData.auth_console_f0_complete_count = 0;
    ps5AuthData.auth_console_f0_accept_mask = 0;
    ps5AuthData.auth_console_f0_submit_mask = 0;
    ps5AuthData.auth_console_f0_last_type = 0;
    ps5AuthData.auth_dongle_f1_count = 0;
    ps5AuthData.auth_dongle_f1_crc_valid_count = 0;
    ps5AuthData.auth_dongle_f1_meta_valid_count = 0;
    ps5AuthData.auth_console_f1_count = 0;
    ps5AuthData.auth_console_f2_count = 0;
    ps5AuthData.auth_dongle_f2_status = 0;
    ps5AuthData.proxy_get_request = 0;
    ps5AuthData.proxy_get_pending = false;
    ps5AuthData.proxy_get_inflight = false;
    ps5AuthData.proxy_response_ready = false;
    ps5AuthData.proxy_response_len = 0;
    memset(ps5AuthData.proxy_response, 0,
        sizeof(ps5AuthData.proxy_response));
    ps5AuthData.S5_reports_ready = false; // S5 Reports ready?
    ps5AuthData.auth_frame_id = 0;      // Which frame are we authing? (send by PS5)
    ps5AuthData.console_f0_recv_count = 0; // Last frame received (stagger receive)
    ps5AuthData.ps5_auth_state = PS5AuthState::ps5_auth_idle;
    memset(ps5AuthData.hash_pending_buffer, 0, sizeof(ps5AuthData.hash_pending_buffer));
    memset(ps5AuthData.hash_finish_buffer, 0, sizeof(ps5AuthData.hash_finish_buffer));
    memset(ps5AuthData.send_hid_buffer, 0, sizeof(ps5AuthData.send_hid_buffer));
    memset(ps5AuthData.mayflash_buffer, 0, sizeof(ps5AuthData.mayflash_buffer));
    memset(ps5AuthData.auth_f1_buffer, 0, sizeof(ps5AuthData.auth_f1_buffer)); // Dongle Struct
    memset(ps5AuthData.auth_f1_raw, 0, sizeof(ps5AuthData.auth_f1_raw));
    memset(ps5AuthData.auth_f2_raw, 0, sizeof(ps5AuthData.auth_f2_raw));
    ps5AuthData.auth_f2_valid = false;
    ps5AuthData.auth_f1_ready = false;
    ps5AuthData.auth_f1_get_index = 0;
    memset(ps5AuthData.console_f0_buffer, 0, sizeof(ps5AuthData.console_f0_buffer)); // Console Struct
    memset(ps5AuthData.auth_f0_raw, 0, sizeof(ps5AuthData.auth_f0_raw));
    ps5AuthData.console_f0_get_index = 0;
    memset(ps5AuthData.MAC_pair_report, 0, sizeof(ps5AuthData.MAC_pair_report));
    memset(ps5AuthData.calibration_report, 0, sizeof(ps5AuthData.calibration_report));
    memset(ps5AuthData.firmware_report, 0, sizeof(ps5AuthData.firmware_report));
    listener = new MayflashS5AuthUSBListener();
    ((MayflashS5AuthUSBListener*)listener)->setup();
    ((MayflashS5AuthUSBListener*)listener)->setAuthData(&ps5AuthData);
}

bool MayflashS5Auth::available() {
    return PeripheralManager::getInstance().isUSBEnabled(0);
}

void MayflashS5Auth::process() {
    ((MayflashS5AuthUSBListener*)listener)->process();
}
