#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/ps4/PS4AuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

static const uint8_t output_0xf3[] = { 0x0, 0x38, 0x38, 0, 0, 0, 0 };

void PS4AuthUSBListener::setup() {
    nonce_page = 0; // no nonce yet
    send_nonce_part = 0; // which part of the nonce are we getting from send?
    awaiting_cb = false; // did we receive the sign state yet
    passthrough_state = PS4State::no_nonce;
    ps4_auth_host_ready = false;
    ps4_auth_buffer = nullptr;
    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    ps_mounted = false;
}

void PS4AuthUSBListener::process(PS4State pState, uint8_t pNonceId, uint8_t * pNonceBuffer) {
    if ( awaiting_cb == true)
        return;

    uint8_t noncelen;
    uint32_t crc32;
    uint8_t buffer[64]; // [0xF0, ID, Page, 0, nonce(54 or 32 with 0 padding), CRC32 of data]

    switch ( passthrough_state ) {
        case PS4State::no_nonce:
            // Did we get the nonce? Let's begin auth
            if ( pState == nonce_ready ) {
                ps4_auth_host_ready = false;
                memcpy(report_buffer, output_0xf3, sizeof(output_0xf3));
                host_get_report(PS4AuthReport::PS4_RESET_AUTH, report_buffer, sizeof(output_0xf3));
            }
            break;
        case PS4State::receiving_nonce:
            buffer[0] = PS4AuthReport::PS4_SET_AUTH_PAYLOAD; // [0xF0, ID, Page, 0, nonce(54 or 32 with 0 padding), CRC32 of data]
            buffer[1] = pNonceId;
            buffer[2] = nonce_page;
            buffer[3] = 0;
            if ( nonce_page == 4 ) {
                noncelen = 32; // from 4 to 64 - 24 - 4
                memcpy(&buffer[4], &pNonceBuffer[nonce_page*56], noncelen);
                memset(&buffer[4+noncelen], 0, 24); // zero padding  
            } else {
                noncelen = 56;
                memcpy(&buffer[4], &pNonceBuffer[nonce_page*56], noncelen);
            }
            nonce_page++;
            crc32 = CRC32::calculate(buffer, 60);
            memcpy(&buffer[60], &crc32, sizeof(uint32_t));
            memcpy(report_buffer, buffer, 64);
            host_set_report(PS4AuthReport::PS4_SET_AUTH_PAYLOAD, report_buffer, 64);
            break;
        case PS4State::signed_nonce_ready:
            buffer[0] = PS4AuthReport::PS4_GET_SIGNING_STATE;
            buffer[1] = pNonceId;
            memset(&buffer[2], 0, 14);
            memcpy(report_buffer, buffer, 16);
            host_get_report(PS4AuthReport::PS4_GET_SIGNING_STATE, report_buffer, 16);
            break;
        case PS4State::sending_nonce:
            buffer[0] = PS4AuthReport::PS4_GET_SIGNATURE_NONCE;
            buffer[1] = pNonceId;    // nonce_id
            buffer[2] = send_nonce_part; // next_part
            memset(&buffer[3], 0, 61); // zero rest of memory
            memcpy(report_buffer, buffer, 64);
            send_nonce_part++; // Nonce Part is reset during callback
            host_get_report(PS4AuthReport::PS4_GET_SIGNATURE_NONCE, report_buffer, 64);
            break;
        case PS4State::waiting_reset:
        default:
            break;
    };
}

void PS4AuthUSBListener::resetHostAuth() {
    nonce_page = 0; // no nonce yet
    send_nonce_part = 0; // which part of the nonce are we getting from send?
    passthrough_state = PS4State::no_nonce;
}

bool PS4AuthUSBListener::host_get_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = true;
    return tuh_hid_get_report(ps_dev_addr, ps_instance, report_id,  HID_REPORT_TYPE_FEATURE, report, len);
}

bool PS4AuthUSBListener::host_set_report(uint8_t report_id, void* report, uint16_t len) {
    awaiting_cb = true;
    return tuh_hid_set_report(ps_dev_addr, ps_instance, report_id,  HID_REPORT_TYPE_FEATURE, report, len);
}

void PS4AuthUSBListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    // Prevent Magic-X double mount
    if ( ps_mounted == true )
        return;

    // Only a PS4 interface has vendor IDs F0, F1, F2, and F3
    tuh_hid_report_info_t report_info[4];
    uint8_t report_count = tuh_hid_parse_report_descriptor(report_info, 4, desc_report, desc_len);
    bool ps4Dongle = false;
    for(uint8_t i = 0; i < report_count; i++) {
        if ( report_info[i].usage_page == 0xFFF0 && 
                (report_info[i].report_id == 0xF3) ) {
            ps4Dongle = true;
            break;
        }
    }
    if (ps4Dongle == false )
        return;

    ps_dev_addr = dev_addr;
    ps_instance = instance;
    ps_mounted = true;

    // Reset as soon as its connected
    memset(report_buffer, 0, sizeof(report_buffer));
    report_buffer[0] = PS4AuthReport::PS4_DEFINITION;
    host_get_report(PS4AuthReport::PS4_DEFINITION, report_buffer, 48);
}

void PS4AuthUSBListener::unmount(uint8_t dev_addr) {
    if ( ps_mounted == false || dev_addr != ps_dev_addr )
        return;
    
    nonce_page = 0; // no nonce yet
    send_nonce_part = 0; // which part of the nonce are we getting from send?
    awaiting_cb = false; // did we receive the sign state yet
    passthrough_state = PS4State::no_nonce;
    ps4_auth_host_ready = false;
    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    ps_mounted = false;
}

void PS4AuthUSBListener::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( ps_mounted == false || dev_addr != ps_dev_addr || instance != ps_instance )
        return;

    switch(report_id) {
        case PS4AuthReport::PS4_SET_AUTH_PAYLOAD:
            if (nonce_page == 5) {
                nonce_page = 0;
                passthrough_state = PS4State::signed_nonce_ready;
            }
            break;
        default:
            break;
    };
    awaiting_cb = false;
}

void PS4AuthUSBListener::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    if ( ps_mounted == false || dev_addr != ps_dev_addr || instance != ps_instance )
        return;
    
    switch(report_id) {
        case PS4AuthReport::PS4_DEFINITION:
            break;
        case PS4AuthReport::PS4_RESET_AUTH:
            passthrough_state = PS4State::receiving_nonce;
            break;
        case PS4AuthReport::PS4_GET_SIGNING_STATE:
            if (report_buffer[2] == 0) // 0 = ready, 1 = error in signing, 16 = not ready
                passthrough_state = PS4State::sending_nonce;
            break;
        case PS4AuthReport::PS4_GET_SIGNATURE_NONCE:
            // probably should mutex lock
            memcpy(&ps4_auth_buffer[(send_nonce_part-1)*56], &report_buffer[4], 56);
            if (send_nonce_part == 19) {
                send_nonce_part = 0;
                passthrough_state = PS4State::waiting_reset; // something we don't support
                ps4_auth_host_ready = true;
            }
            break;
        default:
            break;
    };
    awaiting_cb = false;
}
