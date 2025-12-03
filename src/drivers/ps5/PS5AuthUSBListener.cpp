#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/ps5/PS5AuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"
#include "mbedtls/des.h"

#include <iostream>

#define PS5_LISTENER_PRINTF_ENABLE                0       // GP0 as UART0_TX
#if PS5_LISTENER_PRINTF_ENABLE
#   define P5LPRINTF_INIT(...)                          stdio_init_all(__VA_ARGS__)
#   define P5LPRINTF(...)                               printf(__VA_ARGS__)
#else
#   define P5LPRINTF_INIT(...)
#   define P5LPRINTF(...)
#endif

// DES Secret Key (Mayflash S5 Unlock)
static constexpr uint8_t S5_DES_SECRET_KEY[16] = {
    0x5C, 0x28, 0xE3, 0x05, 0x97, 0xC5, 0xAD, 0x04,
    0x9E, 0x5D, 0x19, 0xC3, 0x25, 0x40, 0x5B, 0x9D
};

/*enum PowerState : uint8_t {
    Discharging         = 0x00, // Use PowerPercent
    Charging            = 0x01, // Use PowerPercent
    Complete            = 0x02, // PowerPercent not valid? assume 100%?
    AbnormalVoltage     = 0x0A, // PowerPercent not valid?
    AbnormalTemperature = 0x0B, // PowerPercent not valid?
    ChargingError       = 0x0F  // PowerPercent not valid?
}*/
//uint8_t PowerPercent : 4; // 0x00-0x0A
//PowerState PowerState : 4;

//uint8_t PluggedHeadphones : 1;
// uint8_t PluggedMic : 1;
// uint8_t MicMuted: 1; // Mic muted by powersave/mute command
//uint8_t PluggedUsbData : 1;
//uint8_t PluggedUsbPower : 1; // appears that this cannot be 1 if PluggedUsbData is 1
//uint8_t UsbPowerOnBT : 1; // appears this is only 1 if BT connected and USB powered
//uint8_t DockDetect : 1;
// uint8_t PluggedUnk : 1;

//uint8_t PluggedExternalMic : 1; // Is external mic active (automatic in mic auto mode)
//uint8_t HapticLowPassFilter : 1; // Is the Haptic Low-Pass-Filter active?
//uint8_t PluggedUnk3 : 6;

// 0x2A - Complete + 100% Power
// 0x18 - Plugged in USB Data and Power
static constexpr uint8_t batteryReport[] = {
    0x2A, 0x18, 0x00
};

// Touchpad X + Y + Timestamp
static constexpr uint8_t touchpadFake[] = {
    0x80, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00
};

void PS5AuthUSBListener::setup() {
    P5LPRINTF_INIT();
    P5LPRINTF("P5L:setup\n");
    ps5AuthData = nullptr;
    resetHostData();
}

void PS5AuthUSBListener::resetHostData() {
    P5LPRINTF("P5L:resetHostData\n");
    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    dongle_type = PS5_NONE;
    mfs5AuthState = MFS5AuthState::mfs5_no_auth;
    awaiting_cb = false;
}

void PS5AuthUSBListener::process() {
    if ( ps5AuthData == nullptr )
        return;

    // Besavior PS5 input handling
    if (dongle_type == P5General && ps5AuthData->hash_pending && tuh_hid_send_ready(ps_dev_addr, ps_instance)) {
        tuh_hid_send_report(ps_dev_addr, ps_instance, 0, ps5AuthData->hash_pending_buffer, 64);
        ps5AuthData->hash_pending = false;
    } else if ( dongle_type == MAYFLASH_S5 && ps5AuthData->keys_ready == true &&
        ps5AuthData->hash_pending && tuh_hid_send_ready(ps_dev_addr, ps_instance)) {
        // manipulate into our S5 format
        PS5Report * ps5Report = (PS5Report*)ps5AuthData->hash_pending_buffer;
        memset(ps5AuthData->auth_buffer, 0, 28);
        ps5AuthData->auth_buffer[0] = 0x02; // To-Mayflash-Device code (0x01 is from)
        ps5AuthData->auth_buffer[1] = 0x04; // key encryption data
        ps5AuthData->auth_buffer[3] = 0x18; // 24 bytes
        memcpy(&(ps5AuthData->auth_buffer[4]), (&ps5Report->report_id), 12);    // 12-byte keys
        memcpy(&(ps5AuthData->auth_buffer[16]), &batteryReport[0], 3);              // 2-byte battery report (1-padding)
        //memcpy(&(ps5AuthData->auth_buffer[19]), &ps5Report->touchpad_data, sizeof(TouchpadData));
        memcpy(&(ps5AuthData->auth_buffer[19]), &touchpadFake[0], sizeof(touchpadFake));
        if ( tuh_hid_send_report(ps_dev_addr, ps_instance, 0, ps5AuthData->auth_buffer, 48) == true ) {
            ps5AuthData->hash_pending = false;
            P5LPRINTF("P5L: Setting Hash Pending to False in PS5AuthUSBListener::Process()\n");
        }
    }

    switch ( ps5AuthData->ps5_passthrough_state ) {
        case PS5AuthState::ps5_auth_send_f0:
            memcpy(report_buffer, ps5AuthData->auth_buffer, 64);
            P5LPRINTF("P5L:ps5_auth_send_f0 %d %d %d %d\n", report_buffer[0], report_buffer[1], report_buffer[2], report_buffer[3]);
            host_set_report(PS5AuthReport::PS5_SET_AUTH_PAYLOAD, report_buffer, 64);
            ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_send_f0_wait;
            break;
        case PS5AuthState::ps5_auth_recv_f1:
            P5LPRINTF("P5L:ps5_auth_recv_f1\n");
            if (f1_num) {
                host_get_report(PS5AuthReport::PS5_GET_SIGNATURE_NONCE, report_buffer, 64);
                ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_recv_f1_wait;
                f1_num--;
            } else {
                ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_idle;
            }
            break;
        case PS5AuthState::ps5_auth_recv_f2_delay_500mS:
            if (getMicro() >= ps5AuthData->auth_recv_f2_us) {
                ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_recv_f2;
            } else {
                break;
            }
        case PS5AuthState::ps5_auth_recv_f2:
            P5LPRINTF("P5L:ps5_auth_recv_f2\n");
            host_get_report(PS5AuthReport::PS5_GET_SIGNING_STATE, report_buffer, 16);
            ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_recv_f2_wait;
            break;
        default:
            break;
    }
}

bool PS5AuthUSBListener::host_get_report(uint8_t report_id, void* report, uint16_t len) {
    P5LPRINTF("P5L:host_get_report\n");
    return tuh_hid_get_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

bool PS5AuthUSBListener::host_set_report(uint8_t report_id, void* report, uint16_t len) {
    P5LPRINTF("P5L:host_set_report\n");
    return tuh_hid_set_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

void PS5AuthUSBListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    P5LPRINTF("P5L:Try Mount\n");
    
    if ( ps5AuthData->dongle_ready == true ) {
        P5LPRINTF("P5L:Already Mounted, don't do it again\n");
        return;
    }

    uint16_t controller_pid, controller_vid;
    tuh_vid_pid_get(dev_addr, &controller_vid, &controller_pid);
    P5LPRINTF("P5L:controller_vid %d, controller_pid %d\n", controller_vid, controller_pid);
    if ((controller_vid == 0x2B81) && (controller_pid == 0x0101)) {
        P5LPRINTF("P5L:Match for PS5. dongle_ready\n");
        ps_dev_addr = dev_addr;
        ps_instance = instance;
        dongle_type = P5General;
        ps5AuthData->dongle_ready = true;
    } else if ( (controller_vid == 0x054C) && (controller_pid == 0x0CE6)) {
        P5LPRINTF("P5L:Match for Mayflash S5? VID:%04x PID:%04x dongle_ready\n", controller_vid, controller_pid);
        ps_dev_addr = dev_addr;
        ps_instance = instance;
        dongle_type = MAYFLASH_S5;
        ps5AuthData->dongle_ready = true;
        // get challenge from dongle (DES Key)
        // Get the authentication data
        P5LPRINTF("P5L:GET AUTH\n");
        if (!tuh_hid_get_report(ps_dev_addr, ps_instance, 0x01, HID_REPORT_TYPE_INPUT, ps5AuthData->auth_buffer, 64))
        {
            P5LPRINTF("P5L:GET fail");
            return;
        }

        mfs5AuthState = MFS5AuthState::mfs5_auth_requested;
        awaiting_cb = true;
    }
}

void PS5AuthUSBListener::unmount(uint8_t dev_addr) {
    P5LPRINTF("P5L:unmount\n");
    if ( ps5AuthData->dongle_ready == false ||
        (dev_addr != ps_dev_addr) ) {
        return;
    }

    resetHostData();
    ps5AuthData->dongle_ready = false;
}

void PS5AuthUSBListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
	if (!ps5AuthData || !ps5AuthData->dongle_ready || (dev_addr != ps_dev_addr) || (instance != ps_instance))
	{
		return;
	}

    // Do we ignore it if we're in the middle of processing an input hash
    if (!ps5AuthData->hash_ready) {
        // Always log to see if interrupt endpoint is working at all
        P5LPRINTF("P5L: Report Received REPORT_ID:%d\n", report[0]);
        if ( dongle_type == MAYFLASH_S5 ) {
            mutex_enter_timeout_us(&ps5AuthData->hash_mutex, 500);
            memset(ps5AuthData->hash_finish_buffer, 0, 64);
            memcpy(&(ps5AuthData->hash_finish_buffer[0x00]), &report[0x0D], 16); // 12-byte key data, 4-bytes of incount
            memcpy(&(ps5AuthData->hash_finish_buffer[0x1C]), &report[0x25], 5); // Sensor Timestamp + Temperature
            memcpy(&(ps5AuthData->hash_finish_buffer[0x21]), &touchpadFake[0], sizeof(TouchpadData)); // fake our touchpad data for now
            memcpy(&(ps5AuthData->hash_finish_buffer[0x2A]), &report[0x2A], 2); // Triggers for 4-bit + 4-bit L and R (https://controllers.fandom.com/wiki/Sony_DualSense)
            memcpy(&(ps5AuthData->hash_finish_buffer[0x31]), &report[0x31], 4); // Device Timestamp (Host timestamp is left blank)
            memcpy(&(ps5AuthData->hash_finish_buffer[0x35]), &batteryReport[0], 3); // Power Percent + Mic Info (https://controllers.fandom.com/wiki/Sony_DualSense)
            memcpy(&(ps5AuthData->hash_finish_buffer[0x38]), &report[0x1D], 8); // 8-Byte AES CMAC
            mutex_exit(&ps5AuthData->hash_mutex);
            ps5AuthData->hash_ready = true;
        } else if ( dongle_type == P5General ) {
            mutex_enter_timeout_us(&ps5AuthData->hash_mutex, 500);
            memcpy(ps5AuthData->hash_finish_buffer, report, sizeof(ps5AuthData->hash_finish_buffer));
            mutex_exit(&ps5AuthData->hash_mutex);
            ps5AuthData->hash_ready = true;
        }
    }
}

void PS5AuthUSBListener::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    P5LPRINTF("P5L:set_report_complete report_id:%08x\n", report_id);
    if ( ps5AuthData->dongle_ready == false ||
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }

    switch(report_id) {
        case PS5AuthReport::PS5_SET_AUTH_PAYLOAD:
            if (ps5AuthData->ps5_passthrough_state == PS5AuthState::ps5_auth_send_f0_wait) {
                switch (ps5AuthData->auth_buffer[1]) {
                    case 0x01:
                        f1_num = 4;
                        break;
                    case 0x03:
                        f1_num = 1;
                        break;
                    case 0x02:
                    default:
                        f1_num = 0;
                        break;
                }
                if (((ps5AuthData->auth_buffer[1] == 0x01) && (ps5AuthData->auth_buffer[3] == 3)) ||
                        (ps5AuthData->auth_buffer[1] == 0x02) ||
                        (ps5AuthData->auth_buffer[1] == 0x03)) {
                    P5LPRINTF("P5L:Auto F2\n");
                    ps5AuthData->auth_recv_f2_us = getMicro() + 500 * 1000;
                    ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_recv_f2_delay_500mS;
                } else {
                    P5LPRINTF("P5L:Back Idel, Wait F0\n");
                    ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_idle;
                }
            } else {
                // unexpect
            }
            break;
        default:
            break;
    };
    if ( dongle_type == MAYFLASH_S5) {
        if ( mfs5AuthState == MFS5AuthState::mfs5_auth_complete &&
            ps5AuthData->keys_ready == false) {
            // Authentication data sent
            awaiting_cb = true;
            ps5AuthData->keys_ready = true;
            P5LPRINTF("P5L: Mayflash S5 All Done and Ready, Let's pull our MAC address\n");
            host_get_report(PS5AuthReport::PS5_GET_PAIRINFO, report_buffer, 16);
        }
    }
}

void PS5AuthUSBListener::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    P5LPRINTF("P5L:get_report_complete report_id %d len %d\n", report_id, len);
    if ( ps5AuthData->dongle_ready == false || 
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }
    
    if ( dongle_type == MAYFLASH_S5 ) {
        switch(report_id) {
            case MFS5AuthReport::PS5_MAYFLASH_GET_AUTH:
                P5LPRINTF("P5L: Checking auth data[7]: %02x  data[12]: %02x\n", ps5AuthData->auth_buffer[7], ps5AuthData->auth_buffer[12]);
                // First GET - check for authentication data: auth_buffer[7] = 0x08, auth_buffer[12] = 0x10
                if (len == 64 && 
                    ps5AuthData->auth_buffer[7] == 0x08 && ps5AuthData->auth_buffer[12] == 0x10)
                {
                    P5LPRINTF("P5L: Auth Data OK\n");
                    memcpy(ps5AuthData->hash_pending_buffer, &ps5AuthData->auth_buffer[13], 16);
                    performS5Encryption(ps5AuthData->hash_pending_buffer, ps5AuthData->hash_finish_buffer);
                    ps5AuthData->auth_buffer[0] = 0x02;
                    ps5AuthData->auth_buffer[1] = 0x08;
                    ps5AuthData->auth_buffer[2] = 0x00;
                    ps5AuthData->auth_buffer[3] = 0x10;
                    memcpy((uint8_t*)&ps5AuthData->auth_buffer[4], ps5AuthData->hash_finish_buffer, 16);
                    P5LPRINTF("P5L: Auth enc\n");
                    if (!tuh_hid_set_report(ps_dev_addr, ps_instance, 0x02, HID_REPORT_TYPE_OUTPUT, ps5AuthData->auth_buffer, 48))
                    {
                        P5LPRINTF("P5L: Auth enc SET fail\n");
                        return;
                    }
                    awaiting_cb = true;
                    mfs5AuthState = MFS5AuthState::mfs5_auth_complete;
                }
                else if (len == 64 && 
                    ps5AuthData->auth_buffer[7] == 0x04 && ps5AuthData->auth_buffer[12] == 0x28)
                {
                    P5LPRINTF("P5L: Key Encryption Data RECEIVED!!! WORKING!!1\n");
                }
                else
                {
                    P5LPRINTF("P5L: Not regular data?");
                    for(int i = 0; i < len; i++) {
                        P5LPRINTF("%02x ", ps5AuthData->auth_buffer[i]);

                    }
                    P5LPRINTF("\n");
                }
                break;
            case MFS5AuthReport::PS5_MAYFLASH_ENC_READY:
                // GET encrypted keys response
                P5LPRINTF("P5L: Got enc keys (get_report_complete)");
                break;
            default:
                break;
        }
    } 

    switch(report_id) {
        case PS5AuthReport::PS5_GET_PAIRINFO:
            if ( len == 16 ) {
                memcpy(ps5AuthData->MAC_pair_report, report_buffer, len);
                for(int i = 0; i < len; i++ ) {
                    P5LPRINTF("%02x ", report_buffer[i]);
                }
                P5LPRINTF("\n");
                ps5AuthData->pair_ready = true;
            } else {
                // P5LPRINTF("P5L: INVALID GET PAIR INFO %02x", len);
            }
            break;
        case PS5AuthReport::PS5_GET_SIGNATURE_NONCE:
            if (ps5AuthData->ps5_passthrough_state == PS5AuthState::ps5_auth_recv_f1_wait) {
                memcpy(ps5AuthData->auth_buffer, report_buffer, len);
                ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_idle;
            } else {
                // unexpect
            }
            break;
        case PS5AuthReport::PS5_GET_SIGNING_STATE:
            if (ps5AuthData->ps5_passthrough_state == PS5AuthState::ps5_auth_recv_f2_wait) {
                memcpy(ps5AuthData->auth_buffer, report_buffer, len);
                ps5AuthData->ps5_passthrough_state = PS5AuthState::ps5_auth_idle;
            } else {
                // unexpect
            }
            break;
        default:
            break;
    };
}

// Encrypt 16-bytes using 8-byte split from Mayflash S5 des keys
void PS5AuthUSBListener::performS5Encryption(uint8_t * inData, uint8_t *outData)
{
	mbedtls_des_context des_ctx;
	mbedtls_des_init(&des_ctx);
	mbedtls_des_setkey_enc(&des_ctx, &S5_DES_SECRET_KEY[0]);
    mbedtls_des_crypt_ecb(&des_ctx, &inData[0], &outData[0]);
    mbedtls_des_setkey_enc(&des_ctx, &S5_DES_SECRET_KEY[8]);
    mbedtls_des_crypt_ecb(&des_ctx, &inData[8], &outData[8]);
    mbedtls_des_free(&des_ctx);
}
