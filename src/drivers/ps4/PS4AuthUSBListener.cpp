#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/ps4/PS4AuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"
#include "pico/time.h"

static const uint8_t output_0xf3[] = { 0x0, 0x38, 0x38, 0, 0, 0, 0 };

// A control transfer that has not completed within this window is treated as
// lost (failed completions with len == 0 are dropped by USBHostManager, and a
// dongle can reset mid-sequence). Normal transfers finish in a few ms.
static const uint32_t PS4_AUTH_CB_TIMEOUT_MS = 500;

// Signing-state byte returned by the dongle in PS4_GET_SIGNING_STATE
static const uint8_t PS4_SIGNING_READY = 0;
static const uint8_t PS4_SIGNING_ERROR = 1;

void PS4AuthUSBListener::setup() {
    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    ps4AuthData = nullptr;
    resetHostData();
}

void PS4AuthUSBListener::process() {
    if ( ps4AuthData == nullptr )
        return;

    if ( awaiting_cb == true ) {
        if ( (to_ms_since_boot(get_absolute_time()) - awaiting_since_ms) < PS4_AUTH_CB_TIMEOUT_MS )
            return;
        // Completion never arrived: restart the dongle exchange. If the console
        // is still waiting on this nonce, no_nonce re-sends it from page 0.
        resetHostData();
    }

    switch ( dongle_state ) {
        case PS4State::no_nonce:
            // Once Console is ready with the nonce, begin!
            if ( ps4AuthData->passthrough_state == GPAuthState::send_auth_console_to_dongle ) {
                memcpy(report_buffer, output_0xf3, sizeof(output_0xf3));
                host_get_report(PS4AuthReport::PS4_RESET_AUTH, report_buffer, sizeof(output_0xf3) + 1);
            }
            break;
        case PS4State::receiving_nonce:
            report_buffer[0] = PS4AuthReport::PS4_SET_AUTH_PAYLOAD; // [0xF0, ID, Page, 0, nonce(54 or 32 with 0 padding), CRC32 of data]
            report_buffer[1] = ps4AuthData->nonce_id;
            report_buffer[2] = nonce_page;
            report_buffer[3] = 0;
            if ( nonce_page == 4 ) {
                noncelen = 32; // from 4 to 64 - 24 - 4
                memcpy(&report_buffer[4], &ps4AuthData->ps4_auth_buffer[nonce_page*56], noncelen);
                memset(&report_buffer[4+noncelen], 0, 24); // zero padding  
            } else {
                noncelen = 56;
                memcpy(&report_buffer[4], &ps4AuthData->ps4_auth_buffer[nonce_page*56], noncelen);
            }
            crc32 = CRC32::calculate(report_buffer, 60);
            memcpy(&report_buffer[60], &crc32, sizeof(uint32_t));
            // Only move to the next page once this one is actually queued;
            // otherwise the same page is retried on the next pass.
            if ( host_set_report(PS4AuthReport::PS4_SET_AUTH_PAYLOAD, report_buffer, 64) ) {
                nonce_page++;
            }
            break;
        case PS4State::signed_nonce_ready:
            report_buffer[0] = PS4AuthReport::PS4_GET_SIGNING_STATE;
            report_buffer[1] = ps4AuthData->nonce_id;
            memset(&report_buffer[2], 0, 14);
            host_get_report(PS4AuthReport::PS4_GET_SIGNING_STATE, report_buffer, 16);
            break;
        case PS4State::sending_nonce:
            report_buffer[0] = PS4AuthReport::PS4_GET_SIGNATURE_NONCE;
            report_buffer[1] = ps4AuthData->nonce_id;    // nonce_id
            report_buffer[2] = nonce_chunk; // next_part
            memset(&report_buffer[3], 0, 61); // zero rest of memory
            // Nonce Part is reset during callback; advance only once queued.
            if ( host_get_report(PS4AuthReport::PS4_GET_SIGNATURE_NONCE, report_buffer, 64) ) {
                nonce_chunk++;
            }
            break;
        default:
            break;
    };
}

void PS4AuthUSBListener::resetHostData() {
    nonce_page = 0; // no nonce yet
    nonce_chunk = 0; // which part of the nonce are we getting from send?
    awaiting_cb = false;
    awaiting_since_ms = 0;
    dongle_state = PS4State::no_nonce;
}

// tuh_hid_*_report() returns false when the transfer could not be queued
// (the host's single control slot is busy, or the device is gone). Only wait
// for a completion if one was actually requested; otherwise process() retries.
bool PS4AuthUSBListener::host_get_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = tuh_hid_get_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
    if ( awaiting_cb )
        awaiting_since_ms = to_ms_since_boot(get_absolute_time());
    return awaiting_cb;
}

bool PS4AuthUSBListener::host_set_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = tuh_hid_set_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
    if ( awaiting_cb )
        awaiting_since_ms = to_ms_since_boot(get_absolute_time());
    return awaiting_cb;
}

void PS4AuthUSBListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    // Prevent Magic-X double mount
    if ( ps4AuthData->dongle_ready == true ) {
        return;
    }

    // Only a PS4 interface has vendor IDs F0, F1, F2, and F3. A full DS4-style
    // descriptor lists the vendor reports after many input/output reports, so
    // parse enough entries to reach them.
    static const uint8_t MAX_REPORT_INFO = 32;
    tuh_hid_report_info_t report_info[MAX_REPORT_INFO];
    uint8_t report_count = tuh_hid_parse_report_descriptor(report_info, MAX_REPORT_INFO, desc_report, desc_len);
    bool isPS4Dongle = false;
    for(uint8_t i = 0; i < report_count; i++) {
        if ( report_info[i].usage_page == 0xFFF0 && 
                (report_info[i].report_id == 0xF3) ) {
            isPS4Dongle = true;
            break;
        }
    }
    if (isPS4Dongle == false )
        return;

    ps_dev_addr = dev_addr;
    ps_instance = instance;
    ps4AuthData->dongle_ready = true;

    // Reset as soon as its connected
    memset(report_buffer, 0, PS4_ENDPOINT_SIZE);
    report_buffer[0] = PS4AuthReport::PS4_DEFINITION;
    host_get_report(PS4AuthReport::PS4_DEFINITION, report_buffer, 48);
}

void PS4AuthUSBListener::unmount(uint8_t dev_addr) {
    if ( ps4AuthData->dongle_ready == false ||
        (dev_addr != ps_dev_addr) ) {
        return;
    }

    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    resetHostData();
    ps4AuthData->dongle_ready = false;
}

void PS4AuthUSBListener::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    // Ignore completions we are not waiting for (e.g. one that arrives after
    // the timeout in process() already restarted the exchange).
    if ( ps4AuthData->dongle_ready == false || awaiting_cb == false ||
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }

    switch(report_id) {
        case PS4AuthReport::PS4_SET_AUTH_PAYLOAD:
            if (nonce_page == 5) {
                nonce_page = 0;
                dongle_state = PS4State::signed_nonce_ready;
            }
            break;
        default:
            break;
    };
    awaiting_cb = false;
}

void PS4AuthUSBListener::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    // Ignore completions we are not waiting for (see set_report_complete).
    if ( ps4AuthData->dongle_ready == false || awaiting_cb == false ||
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }
    
    switch(report_id) {
        case PS4AuthReport::PS4_DEFINITION:
            break;
        case PS4AuthReport::PS4_RESET_AUTH:
            nonce_page = 0;
            nonce_chunk = 0;
            dongle_state = PS4State::receiving_nonce;
            break;
        case PS4AuthReport::PS4_GET_SIGNING_STATE:
            // report_buffer[2]: 0 = ready, 1 = error in signing, 16 = not ready (keep polling)
            if (report_buffer[2] == PS4_SIGNING_READY) {
                dongle_state = PS4State::sending_nonce;
            } else if (report_buffer[2] == PS4_SIGNING_ERROR) {
                // Abandon this nonce and wait for the console to send a new one;
                // re-sending the same nonce would just fail again.
                resetHostData();
                ps4AuthData->passthrough_state = GPAuthState::auth_idle_state;
                return;
            }
            break;
        case PS4AuthReport::PS4_GET_SIGNATURE_NONCE:
            // probably should mutex lock
            if (nonce_chunk == 0 || nonce_chunk > 19) {
                break; // out-of-sequence completion; never index before the buffer
            }
            memcpy(&ps4AuthData->ps4_auth_buffer[(nonce_chunk-1)*56], &report_buffer[4], 56);
            if (nonce_chunk == 19) {
                nonce_chunk = 0;
                dongle_state = PS4State::no_nonce; // something we don't support
                ps4AuthData->passthrough_state = GPAuthState::send_auth_dongle_to_console;
            }
            break;
        default:
            break;
    };
    awaiting_cb = false;
}
