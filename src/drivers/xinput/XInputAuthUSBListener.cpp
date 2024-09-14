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
    xinputAuthData->xinputState = XInputAuthState::auth_idle_state;
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
        printf("%llu: [XInputAuthUSBListner] Mounted!\n", to_ms_since_boot(get_absolute_time()));
        // Get Xbox Security Method 3 (XSM3)
        uint8_t recvBuf[0xB2];
        tuh_descriptor_get_string_sync(xinput_dev_addr, 4, 0x0409, recvBuf, 0xB2);
        auth_dongle_get_serial();
        if ( xinputAuthData->hasInitAuth == true ) {
            printf("[XInputAuthUSBListener] REMOUNT! Send init challenge again to start auth");
            // Actions are performed in this order
            if ( auth_dongle_init_challenge() == false) {
                printf("[XInputAuthUSBListener] ERROR: FAILED to re-init authentication\n");
            } else {
                auth_dongle_wait(XSM360AuthRequest::XSM360_INIT_AUTH);
            }
        }
        xinputAuthData->dongle_ready = true;
    }
}

void XInputAuthUSBListener::unmount(uint8_t dev_addr) {
    // Do not reset dongle_ready on unmount (Magic-X will remount but still be ready)
    if ( dev_addr == xinput_dev_addr ) {
        xinputAuthData->dongle_ready = false;
        printf("%llu: [XInputAuthUSBListner] Unmounted!\n", to_ms_since_boot(get_absolute_time()));
    }
}

void XInputAuthUSBListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
}

void XInputAuthUSBListener::report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
}

void XInputAuthUSBListener::process() {
    // No Auth Data or Dongle is not ready (Unmounted or Not Connected)
    if ( xinputAuthData == nullptr || xinputAuthData->dongle_ready == false) {
        return;
    }

    // Idle State, check for incoming console data
    if ( dongleAuthState == DONGLE_AUTH_STATE::DONGLE_AUTH_IDLE ) {
        // Received a packet from the console to dongle
        if ( xinputAuthData->xinputState == XInputAuthState::send_auth_console_to_dongle ) {
            switch(xinputAuthData->passthruBufferID) {
                case XSM360AuthRequest::XSM360_INIT_AUTH:
                    // Copy to our initial auth buffer incase the dongle reconnects
                    if ( xinputAuthData->hasInitAuth == false ) {
                        printf("[XInputAuthUSBListener] First auth init seen, copying to buffer\n");
                        memcpy(xinputAuthData->consoleInitialAuth, xinputAuthData->passthruBuffer, xinputAuthData->passthruBufferLen);
                        xinputAuthData->hasInitAuth = true;
                    }

                    // Actions are performed in this order
                    if ( auth_dongle_init_challenge() == false) {
                        printf("[XInputAuthUSBListener] ERROR: FAILED initializing authentication\n");
                        xinputAuthData->xinputState = XInputAuthState::auth_idle_state;
                        return;
                    }
                    auth_dongle_wait(XSM360AuthRequest::XSM360_INIT_AUTH);
                    break;
                case XSM360AuthRequest::XSM360_VERIFY_AUTH:
                    // Challenge Verify (22 bytes)
                    if ( auth_dongle_challenge_verify() == false) {
                        printf("[XInputAuthUSBListener] ERROR: FAILED challenge verify\n");
                        xinputAuthData->xinputState = XInputAuthState::auth_idle_state;
                        return;
                    }
                    auth_dongle_wait(XSM360AuthRequest::XSM360_VERIFY_AUTH);
                    break;
                default:
                    //printf("[XInputAuthUSBListener] ERROR: Passthrough Console should not be sending dongle: %02x\n", xinputAuthData->passthruBufferID);
                    break;
            }
        }
    } else if ( dongleAuthState == DONGLE_AUTH_STATE::DONGLE_AUTH_WAIT_STATE ) {
        uint64_t now_time = to_ms_since_boot(get_absolute_time());
        if ( now_time > wait_time ) {
            if ( auth_dongle_wait_get_state() == false ) {
               // printf("%llu: [XInputAuthUSBListener] WAIT still not ready! Time: %llu > %llu\n", to_ms_since_boot(get_absolute_time()), now_time, wait_time);
                wait_time = now_time + WAIT_TIME_MS;
                //printf("%llu: [XInputAuthUSBListener] Updating to Wait Time: %llu\n", to_ms_since_boot(get_absolute_time()), wait_time);
                wait_count++;
            } else {
                printf("%llu: [XInputAuthUSBListener] WAIT is done!\n", to_ms_since_boot(get_absolute_time()));
                switch(waitBufferID) {
                    case XSM360AuthRequest::XSM360_INIT_AUTH:
                        // Actions are performed in this order
                        if ( auth_dongle_data_reply(X360_AUTHLEN_DONGLE_INIT) == false ) {
                            printf("[XInputAuthUSBListener] ERROR in initializing authentication\n");
                            xinputAuthData->xinputState = XInputAuthState::auth_idle_state;
                        } else {
                            auth_dongle_keepalive();
                            printf("[XInputAuthUSBListener] INIT AUTH Sending auth dongle back to console\n");
                            xinputAuthData->xinputState = XInputAuthState::send_auth_dongle_to_console;    
                        }        
                        break;
                    case XSM360AuthRequest::XSM360_VERIFY_AUTH:
                        if ( auth_dongle_data_reply(X360_AUTHLEN_CHALLENGE) == false ) {
                            printf("[XInputAuthUSBListener] ERROR in challenge verify\n");
                            xinputAuthData->xinputState = XInputAuthState::auth_idle_state;
                        } else {
                            printf("[XInputAuthUSBListener] CHALLENGE VERIFY Sending auth dongle back to console\n");
                            xinputAuthData->xinputState = XInputAuthState::send_auth_dongle_to_console;    
                        }    
                        break;
                    default:
                        printf("[XInputAuthUSBListener] Passthrough Console should not be sending dongle: %02x\n", waitBuffer[1]);
                        break;
                }
                dongleAuthState = DONGLE_AUTH_STATE::DONGLE_AUTH_IDLE;
            }   
            if ( wait_count == 60 ) {
                printf("[XInputAuthUSBListener] ERROR: Wait exceeded our auth idle, resetting and failing\n");
                dongleAuthState = DONGLE_AUTH_STATE::DONGLE_AUTH_IDLE;
                wait_count = 0;
                wait_time = 0;
                xinputAuthData->xinputState = XInputAuthState::auth_idle_state;
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
        printf("[XInputAuthUSBListner] ERROR COULD NOT GET SERIAL ID\n");
        return false;
    } else {
        // Ready once we get the serial ID
        printf("[XInputAuthUSBListner] Read Serial ID, dongle is ready\n");
    }
    return true;
}

// Xbox 360 Console Auth Challenge Verify - 0x87
bool XInputAuthUSBListener::auth_dongle_init_challenge() {
    xfer_result_t user_result;
    // Send Auth Init Data to Dongle
    printf("[XInputAuthUSBListener] auth_dongle_init_challenge: %02x\n", X360_AUTHLEN_CONSOLE_INIT);
    if ( xinputh_vendor_report(TUSB_DIR_OUT,
                XSM360AuthRequest::XSM360_INIT_AUTH, X360_WVALUE_CONSOLE_DATA,
                X360_AUTHLEN_CONSOLE_INIT, xinputAuthData->consoleInitialAuth, (uintptr_t)&user_result) == false
                || user_result != xfer_result_t::XFER_RESULT_SUCCESS) {
        printf("[XInputAuthUSBListener] ERROR Could not send initial auth buffer to dongle: %08x\n", user_result);
        return false;        
    }
    return true;
}

// Auth Data reply gets a value of 0x5CXX with XX being the total data length minus 6 bytes for the header
bool XInputAuthUSBListener::auth_dongle_data_reply(uint8_t replyLen) {
    xfer_result_t user_result;
    // Get Xbox 360 Challenge Reply from Dongle
    printf("[XInputAuthUSBListener] Getting Dongle Buffer Challenge Reply Length : %02x\n", replyLen);
    xinputAuthData->passthruBufferLen = replyLen;
    if ( xinputh_vendor_report(TUSB_DIR_IN,
                XSM360AuthRequest::XSM360_RESPOND_CHALLENGE, TU_U16(X360_WVALUE_CONTROLLER_DATA, replyLen-6),
                xinputAuthData->passthruBufferLen, xinputAuthData->passthruBuffer, (uintptr_t)&user_result) == false
            || user_result != xfer_result_t::XFER_RESULT_SUCCESS) {
        printf("[XInputAuthUSBListener] ERROR getting buffer challenge reply\n");
        return false;    
    }
    return true;
}

// Xbox 360 Console Auth Challenge Verify - 0x87
bool XInputAuthUSBListener::auth_dongle_challenge_verify() {
    xfer_result_t user_result;
    // Send Auth Init Data to Dongle
    printf("[XInputAuthUSBListener] Sending Challenge Buffer to Dongle Length: %02x\n", xinputAuthData->passthruBufferLen);
    if ( xinputh_vendor_report(TUSB_DIR_OUT,
                XSM360AuthRequest::XSM360_VERIFY_AUTH, X360_WVALUE_CONSOLE_DATA,
                xinputAuthData->passthruBufferLen, xinputAuthData->passthruBuffer, (uintptr_t)&user_result) == false
                || user_result != xfer_result_t::XFER_RESULT_SUCCESS) {
        printf("[XInputAuthUSBListener] ERROR Could not send initial auth buffer to dongle: %08x\n", user_result);
        return false;        
    }
    return true;
}

// Xbox 360 Console Asks for Current Signing State
bool XInputAuthUSBListener::auth_dongle_wait_get_state() {
    uint8_t wait_buf[2];
    xfer_result_t user_result = xfer_result_t::XFER_RESULT_SUCCESS;
    printf("%llu: [XInputAuthUSBListener] Getting wait state\n", to_ms_since_boot(get_absolute_time()));
    if ( xinputh_vendor_report(TUSB_DIR_IN,
            XSM360AuthRequest::XSM360_REQUEST_STATE, X360_WVALUE_NO_DATA,
            2, wait_buf, (uintptr_t)&user_result) == false ) {
        printf("[XInputAuthUSBListener] ERROR xinputh_vendor_report failed in wait_get_state()\n");
        return false;    
    } else if ( user_result != xfer_result_t::XFER_RESULT_SUCCESS ) {
        printf("[XInputAuthUSBListener] ERROR user_result was not success in wait_get_state(): %02x\n", user_result);
        return false;
    }
    if ( wait_buf[0] == 2 ) { // Dongle is ready!
        printf("[XInputAuthUSBListener] SUCCESS No Longer waiting!\n");
        return true;
    }

    return false;
}

// Xbox 360 Console, Send an 0x84 KeepAlive, all is good message
bool XInputAuthUSBListener::auth_dongle_keepalive() {
    xfer_result_t user_result;
    xinputh_vendor_report(TUSB_DIR_IN,
                XSM360AuthRequest::XSM360_AUTH_KEEPALIVE, X360_WVALUE_CONSOLE_DATA,
                0, NULL, (uintptr_t)&user_result);
    //printf("[XInputAuthUSBListener] Sent auth keepalive\n");
    return true;
}

// Wait for X time before checking auth dongle wait-state (ready, not ready)
void XInputAuthUSBListener::auth_dongle_wait(uint8_t waitID) {
    // Wait for 
    wait_count = 0;
    wait_time = to_ms_since_boot(get_absolute_time()) + WAIT_TIME_MS;
    memcpy(waitBuffer, xinputAuthData->passthruBuffer, xinputAuthData->passthruBufferLen);
    dongleAuthState = DONGLE_AUTH_STATE::DONGLE_AUTH_WAIT_STATE;
    waitBufferID = waitID;
}
