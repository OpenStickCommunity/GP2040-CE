#include "addons/pspassthrough.h"
#include "storagemanager.h"
#include "usbhostmanager.h"
#include "peripheralmanager.h"

#include "CRC32.h"

// Data passed between PS Passthrough and TinyUSB Host callbacks

bool PSPassthroughAddon::available() {
    const PSPassthroughOptions& psOptions = Storage::getInstance().getAddonOptions().psPassthroughOptions;
    const GamepadOptions& gamepadOptions = Storage::getInstance().GetGamepad()->getOptions();
	return psOptions.enabled && PeripheralManager::getInstance().isUSBEnabled(0) && (gamepadOptions.inputMode == INPUT_MODE_PS4);
}

void PSPassthroughAddon::setup() {
    nonce_page = 0; // no nonce yet
    send_nonce_part = 0; // which part of the nonce are we getting from send?
    awaiting_cb = false; // did we receive the sign state yet
    passthrough_state = PS4State::no_nonce;
}

void PSPassthroughAddon::process() {
    if (awaiting_cb)
        return;

    switch ( passthrough_state ) {
        case PS4State::no_nonce:
            // Did we get the nonce? Let's begin auth
            if ( PS4Data::getInstance().ps4State == nonce_ready ) {
                uint8_t const output_0xf3[] = { 0x0, 0x38, 0x38, 0, 0, 0, 0 };
                uint8_t* buf = report_buffer;
                uint16_t len = sizeof(output_0xf3);
                memcpy(buf, output_0xf3, len);
                awaiting_cb = true;
                tuh_hid_get_report(ps_dev_addr, ps_instance, PS4AuthReport::PS4_RESET_AUTH, HID_REPORT_TYPE_FEATURE, buf, len);
            }
            break;
        case PS4State::receiving_nonce:
            {
                uint8_t noncelen;
                uint32_t crc32;
                uint8_t nonce_buffer[64]; // [0xF0, ID, Page, 0, nonce(54 or 32 with 0 padding), CRC32 of data]
                nonce_buffer[0] = PS4AuthReport::PS4_SET_AUTH_PAYLOAD;
                nonce_buffer[1] = PS4Data::getInstance().nonce_id;
                nonce_buffer[2] = nonce_page;
                nonce_buffer[3] = 0;
                if ( nonce_page == 4 ) {
                    noncelen = 32; // from 4 to 64 - 24 - 4
                    memcpy(&nonce_buffer[4], &PS4Data::getInstance().nonce_buffer[nonce_page*56], noncelen);
                    memset(&nonce_buffer[4+noncelen], 0, 24); // zero padding  
                } else {
                    noncelen = 56;
                    memcpy(&nonce_buffer[4], &PS4Data::getInstance().nonce_buffer[nonce_page*56], noncelen);
                }
                nonce_page++;
                crc32 = CRC32::calculate(nonce_buffer, 60);
                memcpy(&nonce_buffer[60], &crc32, sizeof(uint32_t));
                uint8_t* buf = report_buffer;
                uint16_t len = sizeof(nonce_buffer);
                memcpy(buf, nonce_buffer, len);
                awaiting_cb = true;
                tuh_hid_set_report(ps_dev_addr, ps_instance, PS4AuthReport::PS4_SET_AUTH_PAYLOAD, HID_REPORT_TYPE_FEATURE, buf, len);
            }
            break;
        case PS4State::signed_nonce_ready:
            {
                uint8_t state_buffer[16];
                memset(state_buffer, 0, 16);
                state_buffer[0] = PS4AuthReport::PS4_GET_SIGNING_STATE;
                state_buffer[1] = PS4Data::getInstance().nonce_id;
                memset(&state_buffer[2], 0, 14);
                uint8_t* buf = report_buffer;
                uint16_t len = sizeof(state_buffer);
                memcpy(buf, state_buffer, len);
                awaiting_cb = true;
                tuh_hid_get_report(ps_dev_addr, ps_instance, PS4AuthReport::PS4_GET_SIGNING_STATE, HID_REPORT_TYPE_FEATURE, buf, len);
            }
            break;
        case PS4State::sending_nonce:
            {
                uint8_t nonce_buffer[64];
                nonce_buffer[0] = PS4AuthReport::PS4_GET_SIGNATURE_NONCE;
                nonce_buffer[1] = PS4Data::getInstance().nonce_id;    // nonce_id
                nonce_buffer[2] = send_nonce_part; // next_part
                memset(&nonce_buffer[3], 0, 61); // zero rest of memory
                uint8_t* buf = report_buffer;
                uint16_t len = sizeof(nonce_buffer);
                memcpy(buf, nonce_buffer, len);
                send_nonce_part++; // Nonce Part is reset during callback
                awaiting_cb = true;
                tuh_hid_get_report(ps_dev_addr, ps_instance, PS4AuthReport::PS4_GET_SIGNATURE_NONCE, HID_REPORT_TYPE_FEATURE, buf, len);
            }
            break;
        default:
        break;
    };
}

void PSPassthroughAddon::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    ps_dev_addr = dev_addr;
    ps_instance = instance;

    // Reset as soon as its connected
    memset(report_buffer, 0, sizeof(report_buffer));
    report_buffer[0] = PS4AuthReport::PS4_DEFINITION;
    uint8_t* buf = report_buffer;
    uint16_t len = 48;
    awaiting_cb = true;
    tuh_hid_get_report(ps_dev_addr, ps_instance, PS4AuthReport::PS4_DEFINITION, HID_REPORT_TYPE_FEATURE, buf, len);
}

void PSPassthroughAddon::unmount(uint8_t dev_addr) {
    nonce_page = 0; // no nonce yet
    send_nonce_part = 0; // which part of the nonce are we getting from send?
    awaiting_cb = false; // did we receive the sign state yet
    passthrough_state = PS4State::no_nonce;
}

void PSPassthroughAddon::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
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

void PSPassthroughAddon::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    switch(report_id) {
        case PS4AuthReport::PS4_DEFINITION:
            break;
        case PS4AuthReport::PS4_RESET_AUTH:
            if ( PS4Data::getInstance().ps4State == PS4State::nonce_ready)
                passthrough_state = PS4State::receiving_nonce;
            break;
        case PS4AuthReport::PS4_GET_SIGNING_STATE:
            if (report_buffer[2] == 0) // 0 = ready, 1 = error in signing, 16 = not ready
                passthrough_state = PS4State::sending_nonce;
            break;
        case PS4AuthReport::PS4_GET_SIGNATURE_NONCE:
            memcpy(&PS4Data::getInstance().ps4_auth_buffer[(send_nonce_part-1)*56], &report_buffer[4], 56);
            if (send_nonce_part == 19) {
                send_nonce_part = 0;
                passthrough_state = PS4State::no_nonce; // something we don't support
                PS4Data::getInstance().ps4State = PS4State::signed_nonce_ready;
            }
            break;
        default:
            break;
    };
    awaiting_cb = false;
}
