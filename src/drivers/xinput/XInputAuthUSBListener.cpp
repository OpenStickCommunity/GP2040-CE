#include "drivers/xinput/XInputAuthUSBListener.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

#include "drivers/shared/xinput_host.h"
#include "drivers/xinput/XInputDescriptors.h"

// Xbox 360 Auth Magic Numbers (wValue in USB Packet)
#define X360_WVALUE_CONSOLE_DATA 0x0003
#define X360_WVALUE_CONTROLLER_DATA 0x5C
#define X360_WVALUE_CONTROLLER_ID 0x5B
#define X360_WVALUE_NO_DATA 0x00

// How long to wait for calling auth state (in microseconds)
#define WAIT_TIME_MS 100

void XInputAuthUSBListener::setAuthData(XInputAuthData * authData ) {
    xinputAuthData = authData;
    xinputAuthData->dongle_ready = false;
    memset(xinputAuthData->dongleSerial, 0, X360_AUTHLEN_DONGLE_SERIAL);
    xinputAuthData->passthruBufferLen = 0;
    xinputAuthData->xinputState = GPAuthState::auth_idle_state;
}

void XInputAuthUSBListener::setup() {
    xinput_dev_addr = 0xFF;
    xinput_instance = 0xFF;
    dongleAuthState = DONGLE_AUTH_STATE::DONGLE_AUTH_IDLE;
    wait_time = 0;
    wait_count = 0;
    sending = false;
    xinputAuthData->dongle_ready = false;
}

bool XInputAuthUSBListener::xinputh_vendor_report(tusb_dir_t dir, uint8_t request, uint16_t value, uint16_t length, uint8_t * buf, uintptr_t user_data){
    const tusb_control_request_t xfer_ctrl_req = {
            .bmRequestType_bit {
                .recipient = TUSB_REQ_RCPT_INTERFACE,
                .type = TUSB_REQ_TYPE_VENDOR,
                .direction = dir,
            },
            .bRequest = request,
            .wValue = value,
            .wIndex = TU_U16(0x01, 0x03),
            .wLength = length
    };

    tuh_xfer_t xfer = {
        .daddr       = xinput_dev_addr,
        .ep_addr     = 0,
        .setup       = &xfer_ctrl_req,
        .buffer      = buf,
        .complete_cb = NULL,
        .user_data   = user_data,
    };

    return tuh_control_xfer(&xfer);
}

void XInputAuthUSBListener::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    if ( controllerType == xinput_type_t::XBOX360) {
        xinput_dev_addr = dev_addr;
        xinput_instance = instance;
        // Get Xbox Security Method 3 (XSM3)
        uint8_t recvBuf[0xB2];
        tuh_descriptor_get_string_sync(xinput_dev_addr, 4, 0x0409, recvBuf, 0xB2);
        // If our dongle has remounted for any reason, trigger a re-auth (Magicboots X360)
        if ( xinputAuthData->hasInitAuth == true ) {
           if ( auth_dongle_init_challenge() == true) {
                auth_dongle_wait(XSM360AuthRequest::XSM360_INIT_AUTH);
            }
        } else {
            auth_dongle_get_serial();
        }
        xinputAuthData->dongle_ready = true;
    }
}

void XInputAuthUSBListener::unmount(uint8_t dev_addr) {
    // Do not reset dongle_ready on unmount (Magic-X will remount but still be ready)
    if ( dev_addr == xinput_dev_addr ) {
        xinputAuthData->dongle_ready = false;
        dongleAuthState = DONGLE_AUTH_STATE::DONGLE_AUTH_IDLE;
    }
}

void XInputAuthUSBListener::process() {
    // No Auth Data or Dongle is not ready (Unmounted or Not Connected)
    if ( xinputAuthData == nullptr || xinputAuthData->dongle_ready == false) {
        return;
    }

    // Idle State, check for incoming console data
    if ( dongleAuthState == DONGLE_AUTH_STATE::DONGLE_AUTH_IDLE ) {
        // Received a packet from the console to dongle
        if ( xinputAuthData->xinputState == GPAuthState::send_auth_console_to_dongle ) {
            switch(xinputAuthData->passthruBufferID) {
                case XSM360AuthRequest::XSM360_INIT_AUTH:
                    // Copy to our initial auth buffer incase the dongle reconnects
                    if ( xinputAuthData->hasInitAuth == false ) {
                        memcpy(xinputAuthData->consoleInitialAuth, xinputAuthData->passthruBuffer, xinputAuthData->passthruBufferLen);
                        xinputAuthData->hasInitAuth = true;
                    }

                    // Actions are performed in this order
                    if ( auth_dongle_init_challenge() == false) {
                        xinputAuthData->xinputState = GPAuthState::auth_idle_state;
                        return;
                    }
                    auth_dongle_wait(XSM360AuthRequest::XSM360_INIT_AUTH);
                    break;
                case XSM360AuthRequest::XSM360_VERIFY_AUTH:
                    // Challenge Verify (22 bytes)
                    if ( auth_dongle_challenge_verify() == false) {
                        xinputAuthData->xinputState = GPAuthState::auth_idle_state;
                        return;
                    }
                    auth_dongle_wait(XSM360AuthRequest::XSM360_VERIFY_AUTH);
                    break;
                default:
                    break;
            }
        }
    } else if ( dongleAuthState == DONGLE_AUTH_STATE::DONGLE_AUTH_WAIT_STATE ) {
        uint64_t now_time = to_ms_since_boot(get_absolute_time());
        if ( now_time > wait_time ) {
            if ( auth_dongle_wait_get_state() == false ) {
                wait_time = now_time + WAIT_TIME_MS;
                wait_count++;
            } else {
                switch(waitBufferID) {
                    case XSM360AuthRequest::XSM360_INIT_AUTH:
                        // Actions are performed in this order
                        if ( auth_dongle_data_reply(X360_AUTHLEN_DONGLE_INIT) == false ) {
                            xinputAuthData->xinputState = GPAuthState::auth_idle_state;
                        } else {
                            auth_dongle_keepalive();
                            xinputAuthData->xinputState = GPAuthState::send_auth_dongle_to_console;    
                        }        
                        break;
                    case XSM360AuthRequest::XSM360_VERIFY_AUTH:
                        if ( auth_dongle_data_reply(X360_AUTHLEN_CHALLENGE) == false ) {
                            xinputAuthData->xinputState = GPAuthState::auth_idle_state;
                        } else {
                            xinputAuthData->xinputState = GPAuthState::send_auth_dongle_to_console;    
                        }    
                        break;
                    default:
                        break;
                }
                dongleAuthState = DONGLE_AUTH_STATE::DONGLE_AUTH_IDLE;
                wait_count = 0;
            }
            // TIMEOUT after 60 attempts
            if ( wait_count == 60 ) {
                dongleAuthState = DONGLE_AUTH_STATE::DONGLE_AUTH_IDLE;
                wait_count = 0;
                wait_time = 0;
                xinputAuthData->xinputState = GPAuthState::auth_idle_state;
            }
        }
    }
}

bool XInputAuthUSBListener::auth_dongle_get_serial() {
    // Get Serial ID Buffer (0x81)
    xfer_result_t user_result;
    if (xinputh_vendor_report(TUSB_DIR_IN, XSM360AuthRequest::XSM360_GET_SERIAL,
                TU_U16(X360_WVALUE_CONTROLLER_ID, X360_AUTHLEN_DONGLE_SERIAL-6),
                X360_AUTHLEN_DONGLE_SERIAL, xinputAuthData->dongleSerial, (uintptr_t)&user_result) == false
                || user_result != xfer_result_t::XFER_RESULT_SUCCESS) {
        return false;
    }
    return true;
}

// Xbox 360 Console Auth Challenge Verify - 0x87
bool XInputAuthUSBListener::auth_dongle_init_challenge() {
    // Send Auth Init Data to Dongle
    xfer_result_t user_result;
    if ( xinputh_vendor_report(TUSB_DIR_OUT,
                XSM360AuthRequest::XSM360_INIT_AUTH, X360_WVALUE_CONSOLE_DATA,
                X360_AUTHLEN_CONSOLE_INIT, xinputAuthData->consoleInitialAuth, (uintptr_t)&user_result) == false
                || user_result != xfer_result_t::XFER_RESULT_SUCCESS) {
        return false;        
    }
    return true;
}

// Auth Data reply gets a value of 0x5CXX with XX being the total data length minus 6 bytes for the header
bool XInputAuthUSBListener::auth_dongle_data_reply(uint8_t replyLen) {
    // Get Xbox 360 Challenge Reply from Dongle
    xfer_result_t user_result;
    if ( xinputh_vendor_report(TUSB_DIR_IN,
                XSM360AuthRequest::XSM360_RESPOND_CHALLENGE, TU_U16(X360_WVALUE_CONTROLLER_DATA, replyLen-6),
                replyLen, xinputAuthData->passthruBuffer, (uintptr_t)&user_result) == false
            || user_result != xfer_result_t::XFER_RESULT_SUCCESS) {
        return false;    
    }
    xinputAuthData->passthruBufferLen = replyLen;
    return true;
}

// Xbox 360 Console Auth Challenge Verify - 0x87
bool XInputAuthUSBListener::auth_dongle_challenge_verify() {
    // Send Auth Init Data to Dongle
    xfer_result_t user_result;
    if ( xinputh_vendor_report(TUSB_DIR_OUT,
                XSM360AuthRequest::XSM360_VERIFY_AUTH, X360_WVALUE_CONSOLE_DATA,
                X360_AUTHLEN_CHALLENGE, xinputAuthData->passthruBuffer, (uintptr_t)&user_result) == false
                || user_result != xfer_result_t::XFER_RESULT_SUCCESS) {
        return false;        
    }
    return true;
}

// Xbox 360 Console Asks for Current Signing State
bool XInputAuthUSBListener::auth_dongle_wait_get_state() {
    uint8_t wait_buf[2];
    xfer_result_t user_result = xfer_result_t::XFER_RESULT_SUCCESS;
    if ( xinputh_vendor_report(TUSB_DIR_IN,
            XSM360AuthRequest::XSM360_REQUEST_STATE, X360_WVALUE_NO_DATA,
            2, wait_buf, (uintptr_t)&user_result) == false || user_result != xfer_result_t::XFER_RESULT_SUCCESS ) {
        return false;
    }
    return ( wait_buf[0] == 2 ); // Dongle is ready!
}

// Xbox 360 Console, Send an 0x84 KeepAlive, all is good message
bool XInputAuthUSBListener::auth_dongle_keepalive() {
    // Auth Keepalive does not return anything and stalls on some dongles
    xfer_result_t user_result;
    xinputh_vendor_report(TUSB_DIR_IN,
                XSM360AuthRequest::XSM360_AUTH_KEEPALIVE, X360_WVALUE_CONSOLE_DATA,
                0, NULL, (uintptr_t)&user_result);
    return true;
}

// Wait for X time before checking auth dongle wait-state (ready, not ready)
void XInputAuthUSBListener::auth_dongle_wait(uint8_t waitID) {
    // Setup a wait-for mode for the dongle or controller to finish auth
    wait_count = 0;
    wait_time = to_ms_since_boot(get_absolute_time()) + WAIT_TIME_MS;
    memcpy(waitBuffer, xinputAuthData->passthruBuffer, xinputAuthData->passthruBufferLen);
    dongleAuthState = DONGLE_AUTH_STATE::DONGLE_AUTH_WAIT_STATE;
    waitBufferID = waitID;
}
