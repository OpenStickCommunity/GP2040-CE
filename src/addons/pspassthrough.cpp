#include "addons/pspassthrough.h"
#include "storagemanager.h"
#include "usbhostmanager.h"

#include "CRC32.h"

#include "pio_usb.h"

// Data passed between PS Passthrough and TinyUSB Host callbacks
static uint8_t ps_dev_addr = 0;
static uint8_t ps_instance = 0;
static tusb_desc_device_t desc_device;
static int8_t nonce_page;
static PS4State passthrough_state;
static int8_t send_nonce_part;
static uint8_t report_buffer[64];
static bool awaiting_cb;

bool PSPassthroughAddon::available() {
    const PSPassthroughOptions& psOptions = Storage::getInstance().getAddonOptions().psPassthroughOptions;
	  return psOptions.enabled && isValidPin(psOptions.pinDplus) &&
    (psOptions.pin5V == -1 || isValidPin(psOptions.pin5V));
}

void PSPassthroughAddon::setup() {
    const PSPassthroughOptions& psOptions = Storage::getInstance().getAddonOptions().psPassthroughOptions;

    if (psOptions.pin5V != -1) { // Feather USB-A's require this
        const int32_t pin5V = psOptions.pin5V;
        gpio_init(pin5V);
        gpio_set_dir(pin5V, GPIO_IN);
        gpio_pull_up(pin5V);
    }

    USBHostManager::getInstance().init((uint8_t)psOptions.pinDplus);

    nonce_page = 0; // no nonce yet
    send_nonce_part = 0; // which part of the nonce are we getting from send?
    awaiting_cb = false; // did we receive the sign state yet
    passthrough_state = PS4State::no_nonce;
    PS4Data::getInstance().authType = PS4AuthType::PS5_PASSTHROUGH;
}

void PSPassthroughAddon::process() {
    switch ( passthrough_state ) {
      case PS4State::no_nonce:
          // Did we get the nonce? Let's begin auth
          if ( PS4Data::getInstance().ps4State == nonce_ready ) {
            if ( !awaiting_cb ) { // Reset the device auth
                uint8_t output_0xf3[] = { 0xf3, 0x0, 0x38, 0x38, 0, 0, 0, 0 };
                uint8_t* buf = report_buffer;
                uint16_t len = sizeof(output_0xf3);
                memcpy(buf, output_0xf3, len);
                awaiting_cb = true;
                tuh_hid_get_report(ps_dev_addr, ps_instance, PS4AuthReport::PS4_RESET_AUTH, HID_REPORT_TYPE_FEATURE, buf, len);
            }
          }
          break;
      case PS4State::receiving_nonce:
          if ( !awaiting_cb ) {
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
          if ( !awaiting_cb ) { // Get Auth State
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
      case PS4State::sending_nonce:
          if ( !awaiting_cb ) {
            uint8_t nonce_buffer[64];
            uint32_t crc32;
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
    };
}

void PSPassthroughAddon::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    ps_dev_addr = dev_addr;
    ps_instance = instance;
    ps_device_mounted = true;
}

void PSPassthroughAddon::unmount(uint8_t dev_addr) {
    ps_device_mounted = false;
    nonce_page = 0; // no nonce yet
    send_nonce_part = 0; // which part of the nonce are we getting from send?
    awaiting_cb = false; // did we receive the sign state yet
    passthrough_state = PS4State::no_nonce;
}

void PSPassthroughAddon::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    switch(report_id) {
        case PS4AuthReport::PS4_SET_AUTH_PAYLOAD:
            awaiting_cb = false;
            if (nonce_page == 5) {
                nonce_page = 0;
                passthrough_state = PS4State::signed_nonce_ready;
            }
            break;
        default:
            break;
    };
}

void PSPassthroughAddon::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    switch(report_id) {
        case PS4AuthReport::PS4_RESET_AUTH:
            awaiting_cb = false;
            if ( PS4Data::getInstance().ps4State == PS4State::nonce_ready)
                passthrough_state = PS4State::receiving_nonce;
            break;
        case PS4AuthReport::PS4_GET_SIGNING_STATE:
            awaiting_cb = false;
            if (report_buffer[2] == 0)
                passthrough_state = PS4State::sending_nonce;
            break;
        case PS4AuthReport::PS4_GET_SIGNATURE_NONCE:
            awaiting_cb = false;
            memcpy(&PS4Data::getInstance().ps4_auth_buffer[(send_nonce_part-1)*56], &report_buffer[4], 56);
            if (send_nonce_part == 19) { // 0 = ready, 16 = not ready
                send_nonce_part = 0;
                passthrough_state = PS4State::no_nonce; // something we don't support
                PS4Data::getInstance().ps4State = PS4State::signed_nonce_ready;
            }
            break;
        default:
            break;
    };
}
