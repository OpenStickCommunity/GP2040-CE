#include "host/usbh.h"
#include "class/hid/hid.h"
#include "class/hid/hid_host.h"
#include "drivers/p5general/P5GeneralAuthUSBListener.h"
#include "CRC32.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

#include <iostream>

#define P5GENERAL_LISTENER_PRINTF_ENABLE                0       // GP0 as UART0_TX
#if P5GENERAL_LISTENER_PRINTF_ENABLE
#   define P5LRPINTF_INIT(...)                          stdio_init_all(__VA_ARGS__)
#   define P5LRPINTF(...)                               printf(__VA_ARGS__)
#else
#   define P5LRPINTF_INIT(...)
#   define P5LRPINTF(...)
#endif

void P5GeneralAuthUSBListener::setup() {
    P5LRPINTF_INIT();
    P5LRPINTF("P5L:setup\n");

    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    p5GeneralAuthData = nullptr;
    resetHostData();
}

void P5GeneralAuthUSBListener::resetHostData() {
    P5LRPINTF("P5L:resetHostData\n");
}

void P5GeneralAuthUSBListener::process() {
    if ( p5GeneralAuthData == nullptr )
        return;

    if (p5GeneralAuthData->hash_pending && tuh_hid_send_ready(ps_dev_addr, ps_instance)) {
        tuh_hid_send_report(ps_dev_addr, ps_instance, 0, p5GeneralAuthData->hash_pending_buffer, 64);
        p5GeneralAuthData->hash_pending = false;
    }

    switch ( p5GeneralAuthData->passthrough_state ) {
    case P5GeneralGPAuthState::p5g_auth_send_f0:
        memcpy(report_buffer, p5GeneralAuthData->auth_buffer, 64);
        P5LRPINTF("P5L:p5g_auth_send_f0 %d %d %d %d\n", report_buffer[0], report_buffer[1], report_buffer[2], report_buffer[3]);
        host_set_report(P5GeneralAuthReport::P5GENERAL_SET_AUTH_PAYLOAD, report_buffer, 64);
        p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_send_f0_wait;
        break;
    case P5GeneralGPAuthState::p5g_auth_recv_f1:
        P5LRPINTF("P5L:p5g_auth_recv_f1\n");
        if (f1_num) {
            host_get_report(P5GeneralAuthReport::P5GENERAL_GET_SIGNATURE_NONCE, report_buffer, 64);
            p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_recv_f1_wait;
            f1_num--;
        } else {
            p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_idle;
        }
        break;
    case P5GeneralGPAuthState::p5g_auth_recv_f2_delay_500mS:
        if (getMicro() >= p5GeneralAuthData->auth_recv_f2_us) {
            p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_recv_f2;
        } else {
            break;
        }
    case P5GeneralGPAuthState::p5g_auth_recv_f2:
        P5LRPINTF("P5L:p5g_auth_recv_f2\n");
        host_get_report(P5GeneralAuthReport::P5GENERAL_GET_SIGNING_STATE, report_buffer, 16);
        p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_recv_f2_wait;
        break;
    }
}

bool P5GeneralAuthUSBListener::host_get_report(uint8_t report_id, void* report, uint16_t len) {
    P5LRPINTF("P5L:host_get_report\n");
    return tuh_hid_get_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

bool P5GeneralAuthUSBListener::host_set_report(uint8_t report_id, void* report, uint16_t len) {
    P5LRPINTF("P5L:host_set_report\n");
    return tuh_hid_set_report(ps_dev_addr, ps_instance, report_id, HID_REPORT_TYPE_FEATURE, report, len);
}

void P5GeneralAuthUSBListener::mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len) {
    P5LRPINTF("P5L:Try Mount\n");
    
    if ( p5GeneralAuthData->dongle_ready == true ) {
        return;
    }

    uint16_t controller_pid, controller_vid;
    tuh_vid_pid_get(dev_addr, &controller_vid, &controller_pid);
    P5LRPINTF("P5L:controller_vid %d, controller_pid %d\n", controller_vid, controller_pid);
    if ((controller_vid == 0x2B81) && (controller_pid == 0x0101)) {
        P5LRPINTF("P5L:Match. dongle_ready\n");
        ps_dev_addr = dev_addr;
        ps_instance = instance;
        p5GeneralAuthData->dongle_ready = true;
    }
}

void P5GeneralAuthUSBListener::unmount(uint8_t dev_addr) {
    P5LRPINTF("P5L:unmount\n");
    if ( p5GeneralAuthData->dongle_ready == false ||
        (dev_addr != ps_dev_addr) ) {
        return;
    }

    ps_dev_addr = 0xFF;
    ps_instance = 0xFF;
    resetHostData();
    p5GeneralAuthData->dongle_ready = false;
}

void P5GeneralAuthUSBListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if (!p5GeneralAuthData->hash_ready) {
        memcpy(p5GeneralAuthData->hash_finish_buffer, report, sizeof(p5GeneralAuthData->hash_finish_buffer));
        p5GeneralAuthData->hash_ready = true;
    }
}

void P5GeneralAuthUSBListener::set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    P5LRPINTF("P5L:set_report_complete\n");
    if ( p5GeneralAuthData->dongle_ready == false ||
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }

    switch(report_id) {
        case P5GeneralAuthReport::P5GENERAL_SET_AUTH_PAYLOAD:
            if (p5GeneralAuthData->passthrough_state == P5GeneralGPAuthState::p5g_auth_send_f0_wait) {
                switch (p5GeneralAuthData->auth_buffer[1]) {
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
                if (((p5GeneralAuthData->auth_buffer[1] == 0x01) && (p5GeneralAuthData->auth_buffer[3] == 3)) ||
                        (p5GeneralAuthData->auth_buffer[1] == 0x02) ||
                        (p5GeneralAuthData->auth_buffer[1] == 0x03)) {
                    P5LRPINTF("P5L:Auto F2\n");
                    p5GeneralAuthData->auth_recv_f2_us = getMicro() + 500 * 1000;
                    p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_recv_f2_delay_500mS;
                } else {
                    P5LRPINTF("P5L:Back Idel, Wait F0\n");
                    p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_idle;
                }
            } else {
                // unexpect
            }
            break;
        default:
            break;
    };
}

void P5GeneralAuthUSBListener::get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len) {
    P5LRPINTF("P5L:get_report_complete report_id %d len %d\n", report_id, len);
    if ( p5GeneralAuthData->dongle_ready == false || 
        (dev_addr != ps_dev_addr) || (instance != ps_instance) ) {
        return;
    }
    
    switch(report_id) {
        case P5GeneralAuthReport::P5GENERAL_GET_SIGNATURE_NONCE:
            if (p5GeneralAuthData->passthrough_state == P5GeneralGPAuthState::p5g_auth_recv_f1_wait) {
                memcpy(p5GeneralAuthData->auth_buffer, report_buffer, len);
                p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_idle;
            } else {
                // unexpect
            }
            break;
        case P5GeneralAuthReport::P5GENERAL_GET_SIGNING_STATE:
            if (p5GeneralAuthData->passthrough_state == P5GeneralGPAuthState::p5g_auth_recv_f2_wait) {
                memcpy(p5GeneralAuthData->auth_buffer, report_buffer, len);
                p5GeneralAuthData->passthrough_state = P5GeneralGPAuthState::p5g_auth_idle;
            } else {
                // unexpect
            }
            break;
        default:
            break;
    };
}
