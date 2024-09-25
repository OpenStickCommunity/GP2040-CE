#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/ps4/PS4AuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

static const uint8_t output_0xf3[] = { 0x0, 0x38, 0x38, 0, 0, 0, 0 };

void PS4AuthUSBListener::setup() {
    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    ps4AuthData = nullptr;
    resetHostData();
}

void PS4AuthUSBListener::process() {
    if ( awaiting_cb == true || ps4AuthData == nullptr )
        return;

    switch ( dongle_state ) {
        case PS4State::no_nonce:
            // Once Console is ready with the nonce, begin!
            if ( ps4AuthData->passthrough_state == GPAuthState::send_auth_console_to_dongle ) {
                memcpy(report_buffer, output_0xf3, sizeof(output_0xf3));
                host_get_report(PS4AuthReport::PS4_RESET_AUTH, report_buffer, sizeof(output_0xf3));
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
            nonce_page++;
            crc32 = CRC32::calculate(report_buffer, 60);
            memcpy(&report_buffer[60], &crc32, sizeof(uint32_t));
            host_set_report(PS4AuthReport::PS4_SET_AUTH_PAYLOAD, report_buffer, 64);
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
            nonce_chunk++; // Nonce Part is reset during callback
            host_get_report(PS4AuthReport::PS4_GET_SIGNATURE_NONCE, report_buffer, 64);
            break;
        default:
            break;
    };
}

void PS4AuthUSBListener::resetHostData() {
    nonce_page = 0; // no nonce yet
    nonce_chunk = 0; // which part of the nonce are we getting from send?
    awaiting_cb = false;
    dongle_state = PS4State::no_nonce;
}

bool PS4AuthUSBListener::host_get_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = true;
    return tuh_hid_get_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

bool PS4AuthUSBListener::host_set_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = true;
    return tuh_hid_set_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

void PS4AuthUSBListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    // Prevent Magic-X double mount
    if ( ps4AuthData->dongle_ready == true ) {
        return;
    }

    // Only a PS4 interface has vendor IDs F0, F1, F2, and F3
    tuh_hid_report_info_t report_info[4];
    uint8_t report_count = tuh_hid_parse_report_descriptor(report_info, 4, desc_report, desc_len);
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
    if ( ps4AuthData->dongle_ready == false ||
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
    if ( ps4AuthData->dongle_ready == false || 
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
            if (report_buffer[2] == 0) // 0 = ready, 1 = error in signing, 16 = not ready
                dongle_state = PS4State::sending_nonce;
            break;
        case PS4AuthReport::PS4_GET_SIGNATURE_NONCE:
            // probably should mutex lock
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
