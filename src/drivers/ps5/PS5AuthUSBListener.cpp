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

// 0x28 - Charging + 100% Power
// 0x18 - Plugged in USB Data and Power
static constexpr uint8_t batteryReport[] = {
    0x28, 0x18, 0x00
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
}

static uint8_t sendReport[64];

void PS5AuthUSBListener::process() {
    if ( ps5AuthData == nullptr )
        return;

    // Besavior PS5 input handling
    if (dongle_type == P5General && ps5AuthData->hash_pending && tuh_hid_send_ready(ps_dev_addr, ps_instance)) {
        tuh_hid_send_report(ps_dev_addr, ps_instance, 0, ps5AuthData->hash_pending_buffer, 64);
        ps5AuthData->hash_pending = false;
    } else if ( dongle_type == MAYFLASH_S5 && ps5AuthData->hash_pending && tuh_hid_send_ready(ps_dev_addr, ps_instance)) {
        P5LPRINTF("P5L:sending pending hash over to the Mayflash S5\n");
        
        // manipulate into our S5 format
        PS5Report * ps5Report = (PS5Report*)ps5AuthData->hash_pending_buffer;
        memset(ps5AuthData->mayflash_buffer, 0, 28);
        ps5AuthData->mayflash_buffer[0] = 0x02; // To-Mayflash-Device code (0x01 is from)
        ps5AuthData->mayflash_buffer[1] = 0x04; // key encryption data
        ps5AuthData->mayflash_buffer[3] = 0x18; // 24 bytes
        memcpy(&(ps5AuthData->mayflash_buffer[4]), &(ps5Report->report_id), 12);    // 12-byte keys
        memcpy(&(ps5AuthData->mayflash_buffer[16]), &batteryReport[0], 3);          // 2-byte battery report (1-padding)
        //memcpy(&(ps5AuthData->mayflash_buffer[19]), &ps5Report->touchpad_data.p1, sizeof(TouchpadXY));
        //memcpy(&(ps5AuthData->mayflash_buffer[23]), &ps5Report->touchpad_data.p2, sizeof(TouchpadXY));
        memcpy(&(ps5AuthData->mayflash_buffer[19]), &touchpadFake[0], sizeof(touchpadFake));        
        tuh_hid_send_report(ps_dev_addr, ps_instance, 0, ps5AuthData->mayflash_buffer, 48);
        ps5AuthData->hash_pending = false;
    }

    uint32_t crc32;
    switch ( ps5AuthData->ps5_auth_state ) {
        case PS5AuthState::ps5_auth_send_f0_from_console:
            sendReport[0] = PS5AuthReport::PS5_SET_AUTH_PAYLOAD;
            sendReport[1] = 0x01;
            sendReport[2] = ps5AuthData->auth_frame_id;
            sendReport[3] = ps5AuthData->console_f0_get_index;
            memcpy(&sendReport[4], &ps5AuthData->console_f0_buffer[ps5AuthData->console_f0_get_index*56], 56);
            crc32 = CRC32::calculate(sendReport, 60);
            memcpy(&sendReport[60], &crc32, sizeof(uint32_t));
            P5LPRINTF("P5L:ps5_auth_send_f0_from_console sending chunk %02x frame id %02x\n", ps5AuthData->console_f0_get_index, ps5AuthData->auth_frame_id);
            host_set_report(PS5AuthReport::PS5_SET_AUTH_PAYLOAD, sendReport, 64);
            ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_idle;
            break;
        case PS5AuthState::ps5_auth_send_f0_final_from_console:
            sendReport[0] = PS5AuthReport::PS5_SET_AUTH_PAYLOAD;
            sendReport[1] = 0x02;
            sendReport[2] = ps5AuthData->auth_frame_id;
            sendReport[3] = 0;
            memcpy(&sendReport[4], &ps5AuthData->console_f0_buffer[0], 56);
            crc32 = CRC32::calculate(sendReport, 60);
            memcpy(&sendReport[60], &crc32, sizeof(uint32_t));
            P5LPRINTF("P5L:ps5_auth_send_f0_final_from_console sending chunk %02x frame id %02x\n", sendReport[3], ps5AuthData->auth_frame_id);
            host_set_report(PS5AuthReport::PS5_SET_AUTH_PAYLOAD, sendReport, 64);
            ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_idle;
            break;
        /*case PS5AuthState::ps5_auth_recv_f2_delay_500mS:
            if (getMicro() >= ps5AuthData->auth_recv_f2_us) {
                P5LPRINTF("P5L:ps5_auth_recv_f2_delay_500mS delay elapsed, moving to f2\n");
                ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_recv_f2;
            } else {
                P5LPRINTF("P5L:ps5_auth_recv_f2_delay_500mS not enough time elapsed, continue...\n");
                break;
            }*/
        case PS5AuthState::ps5_auth_recv_f2_signing_state:
            //P5LPRINTF("P5L:ps5_auth_recv_f2_signing_state send to dongle\n");
            host_get_report(PS5AuthReport::PS5_GET_SIGNING_STATE, report_buffer, 16);
            ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_idle;
            break;
        case PS5AuthState::ps5_auth_recv_f1_from_dongle:
            //P5LPRINTF("P5L:ps5_auth_recv_f1_from_dongle %02x\n", ps5AuthData->auth_f1_get_index);
            host_get_report(PS5AuthReport::PS5_GET_SIGNATURE_NONCE, report_buffer, 64);
            ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_idle;
            break;
        default:
            break;
    }
}

bool PS5AuthUSBListener::host_get_report(uint8_t report_id, void* report, uint16_t len) {
    //P5LPRINTF("P5L:host_get_report\n");
    return tuh_hid_get_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

bool PS5AuthUSBListener::host_set_report(uint8_t report_id, void* report, uint16_t len) {
    //P5LPRINTF("P5L:host_set_report\n");
    return tuh_hid_set_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

void PS5AuthUSBListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    P5LPRINTF("P5L:Try Mount\n");
    
    if ( ps5AuthData->dongle_mounted == true ) {
        P5LPRINTF("P5L:Already Mounted, don't do it again\n");
        return;
    }

    uint16_t controller_pid, controller_vid;
    tuh_vid_pid_get(dev_addr, &controller_vid, &controller_pid);
    P5LPRINTF("P5L:controller_vid %04x, controller_pid %04x\n", controller_vid, controller_pid);
    if ((controller_vid == 0x2B81) && (controller_pid == 0x0101)) {
        P5LPRINTF("P5L:Match for P5General. dongle_ready\n");
        ps_dev_addr = dev_addr;
        ps_instance = instance;
        dongle_type = P5General;
        ps5AuthData->dongle_mounted = true;
        ps5AuthData->dongle_ready = true; // P5General is ready as soon as its mounted
        P5LPRINTF("P5L:Hash pending for some reason is true? %02x\n", ps5AuthData->hash_pending);
    } else if ( (controller_vid == 0x054C) && (controller_pid == 0x0CE6)) {
        P5LPRINTF("P5L:Match for Mayflash S5.\n");
        ps_dev_addr = dev_addr;
        ps_instance = instance;
        dongle_type = MAYFLASH_S5;
        ps5AuthData->dongle_mounted = true; // mount only, not ready
        // get challenge from dongle (DES Key)
        P5LPRINTF("P5L:Getting Mayflash Challenge (DES Key)\n");
        if (!tuh_hid_get_report(ps_dev_addr, ps_instance, 0x01, HID_REPORT_TYPE_INPUT, ps5AuthData->mayflash_buffer, 64))
        {
            P5LPRINTF("P5L:GET fail");
            return;
        }
    }
}

void PS5AuthUSBListener::unmount(uint8_t dev_addr) {
    P5LPRINTF("P5L:unmount\n");
    if ( ps5AuthData->dongle_mounted == false ||
        (dev_addr != ps_dev_addr) ) {
        return;
    }

    resetHostData();
    ps5AuthData->dongle_ready = false;
    ps5AuthData->dongle_mounted = false;
}

void PS5AuthUSBListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
	//P5LPRINTF("P5L:received report id:%02x len:%02x\n", report[0], len);
    
    if (!ps5AuthData || !ps5AuthData->dongle_mounted || (dev_addr != ps_dev_addr) || (instance != ps_instance))
	{
		return;
	}

    // Do we ignore it if we're in the middle of processing an input hash
    if (!ps5AuthData->hash_ready) {
        // Always log to see if interrupt endpoint is working at all
        //P5LPRINTF("P5L: Report Received REPORT_ID:%d len: %d\n", report[0], len);
        if ( dongle_type == MAYFLASH_S5 ) {
            memset(ps5AuthData->hash_finish_buffer, 0, 64);
            memcpy(&(ps5AuthData->hash_finish_buffer[0x00]), &report[0x0D], 16); // 12-byte key data, 4-bytes of incount
            memcpy(&(ps5AuthData->hash_finish_buffer[0x1C]), &report[0x25], 5); // Sensor Timestamp + Temperature
            memcpy(&(ps5AuthData->hash_finish_buffer[0x21]), &touchpadFake[0], sizeof(TouchpadData)); // fake our touchpad data for now
            memcpy(&(ps5AuthData->hash_finish_buffer[0x2A]), &report[0x2A], 2); // Triggers for 4-bit + 4-bit L and R (https://controllers.fandom.com/wiki/Sony_DualSense)
            memcpy(&(ps5AuthData->hash_finish_buffer[0x31]), &report[0x31], 4); // Device Timestamp (Host timestamp is left blank)
            memcpy(&(ps5AuthData->hash_finish_buffer[0x35]), &batteryReport[0], 3); // Power Percent + Mic Info (https://controllers.fandom.com/wiki/Sony_DualSense)
            memcpy(&(ps5AuthData->hash_finish_buffer[0x38]), &report[0x1D], 8); // 8-Byte AES CMAC
            ps5AuthData->hash_ready = true;
        } else if ( dongle_type == P5General ) {
            memcpy(ps5AuthData->hash_finish_buffer, report, sizeof(ps5AuthData->hash_finish_buffer));
            ps5AuthData->hash_ready = true;
        }
    } else {
        P5LPRINTF("P5L:report_received Hash is already ready, ignore this request!\n");
    }
}

void PS5AuthUSBListener::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    //P5LPRINTF("P5L:set_report_complete report_id:%02x report_type:%02x\n", report_id, report_type);
    if ( ps5AuthData->dongle_mounted == false ||
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }
    // Are we checkign the report id here correctly??
    switch(report_id) {
        case MFS5AuthReport::PS5_MAYFLASH_AUTH_COMPLETE:
            // Authentication data sent
            ps5AuthData->dongle_ready = true;
            P5LPRINTF("P5L: Mayflash S5 All Done and Ready, Let's pull our MAC address\n");
            host_get_report(PS5AuthReport::PS5_GET_PAIRINFO, report_buffer, 16);
            break;
        case PS5AuthReport::PS5_SET_AUTH_PAYLOAD:
            if ( ps5AuthData->console_f0_type == 1 ) { // 4 block chunk
                if ( ps5AuthData->console_f0_get_index == 3 ) { // ??
                    P5LPRINTF("P5L: PS5_SET_AUTH_PAYLOAD F0 console to dongle is done, now lets loop for the dongle state\n");
                    ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_recv_f2_signing_state;
                } else {
                    //P5LPRINTF("P5L: PS5_SET_AUTH_PAYLOAD F0 still receiving, update our index and get the next one\n");
                    ps5AuthData->console_f0_get_index++;
                    ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_send_f0_from_console;
                }
            } else if ( ps5AuthData->console_f0_type == 2 ) { // 1 block chunk
                P5LPRINTF("P5L: PS5_SET_AUTH_PAYLOAD F0 final F0 done, get signing state (Should be 0x40)\n");
                ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_recv_f2_signing_state;
            }
            break;
        default:
            break;
    };
}

void PS5AuthUSBListener::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    //P5LPRINTF("P5L:get_report_complete report_id %02x len %d\n", report_id, len);
    if ( ps5AuthData->dongle_mounted == false || 
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }
    
    if ( dongle_type == MAYFLASH_S5 ) {
        switch(report_id) {
            case MFS5AuthReport::PS5_MAYFLASH_GET_AUTH:
                P5LPRINTF("P5L: Checking auth data[7]: %02x  data[12]: %02x\n", ps5AuthData->mayflash_buffer[7], ps5AuthData->mayflash_buffer[12]);
                // 0x08 0x10 means we got our encrypted buffer
                if (len == 64 && ps5AuthData->mayflash_buffer[7] == 0x08 && ps5AuthData->mayflash_buffer[12] == 0x10)
                {
                    P5LPRINTF("P5L: Auth Data OK, Sending encrypted buffer\n");
                    // Incoming buffer [13] through [29] = incoming nonce to encrypt
                    uint8_t * nonceToEncrypt = &(ps5AuthData->mayflash_buffer[13]);
                    uint8_t * nonceOutput = &(ps5AuthData->mayflash_buffer[4]);
                    performS5Encryption(nonceToEncrypt, ps5AuthData->hash_finish_buffer);
                    ps5AuthData->mayflash_buffer[0] = 0x02;
                    ps5AuthData->mayflash_buffer[1] = 0x08;
                    ps5AuthData->mayflash_buffer[2] = 0x00;
                    ps5AuthData->mayflash_buffer[3] = 0x10;
                    memcpy(nonceOutput, ps5AuthData->hash_finish_buffer, 16);
                    if (!tuh_hid_set_report(ps_dev_addr, ps_instance, 0x02, HID_REPORT_TYPE_OUTPUT, ps5AuthData->mayflash_buffer, 48))
                    {
                        P5LPRINTF("P5L: Auth enc SET fail\n");
                        return;
                    }
                }
                break;
            default:
                break;
        }
    } 

    switch(report_id) {
        case PS5AuthReport::PS5_GET_PAIRINFO:
            memcpy(ps5AuthData->MAC_pair_report, report_buffer, len);
            ps5AuthData->pair_ready = true;
            P5LPRINTF("P5L: PS5_GET_PAIRINFO Got Mac Address\n");
            break;
        case PS5AuthReport::PS5_GET_SIGNATURE_NONCE:
            if ( ps5AuthData->auth_frame_id == report_buffer[2] ) {
                P5LPRINTF("P5L: PS5_GET_SIGNATURE_NONCE got signature nonce for index:%02x\n", report_buffer[3]);
                memcpy(&ps5AuthData->auth_f1_buffer[report_buffer[3]*56], &report_buffer[4], 56);
                if ( report_buffer[3] >= 3 ) { // we just got the last chunk, we're done!
                    P5LPRINTF("P5L: PS5_GET_SIGNATURE_NONCE done, we're all done on the dongle!\n");
                    ps5AuthData->auth_f1_ready = true;
                    ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_idle;
                } else {
                    P5LPRINTF("P5L: PS5_GET_SIGNATURE_NONCE still receiving, update our index and get the next one\n");
                    ps5AuthData->auth_f1_get_index = report_buffer[3]+1;
                    ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_recv_f1_from_dongle;
                }
            } else {
                P5LPRINTF("P5L: PS5_GET_SIGNATURE_NONCE wrong frame id\n");
            }
            break;
        case PS5AuthReport::PS5_GET_SIGNING_STATE:
            //P5LPRINTF("P5L:signing_state status: %02x %02x\n", report_buffer[3], len);
            if ( report_buffer[3] == 0x12 ) { // READY!
                P5LPRINTF("P5L: Get signing state completed, lets get the F1 buffer\n");
                ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_recv_f1_from_dongle;
            } else if ( report_buffer[3] == 0x11 ) { // STILL SIGNING!
                //P5LPRINTF("P5L: Get signing state still waiting\n");
                ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_recv_f2_signing_state;
            } else if ( report_buffer[3] == 0x40 ) {
                P5LPRINTF("P5L: Signing is complete on dongle! Do nothing, 0x40\n");
                ps5AuthData->ps5_auth_state = PS5AuthState::ps5_auth_idle;
                ps5AuthData->auth_f1_done = true;
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
