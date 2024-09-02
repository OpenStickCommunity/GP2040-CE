#include "drivers/xinput/XInputAuthUSBListener.h"
#include "peripheralmanager.h"
#include "usbhostmanager.h"

#include "drivers/shared/xinput_host.h"

typedef enum {
    X360State_GetSerial = 0,
    X360State_Idle
} X360State;

static bool controllerIdRecv;
static X360State state;

void XInputAuthUSBListener::setAuthData(XInputAuthData * authData ) {
    xinputAuthData = authData;
    xinputAuthData->dongle_ready = false;
}

void XInputAuthUSBListener::setup() {
    state = X360State_GetSerial;
}

void XInputAuthUSBListener::xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype) {
    if ( controllerType == xinput_type_t::XBOX360) {
        xinput_dev_addr = dev_addr;
        xinput_instance = instance;
        mounted = true;
        printf("[XInputAuthUSBListner] Mounted!\n");
        controllerIdRecv = false;
                
        // Get Xbox Security Method 3 (XSM3)
        uint8_t recvBuf[0xB2];
        const tusb_control_request_t xfer_ctrl_req = {
            .bmRequestType_bit {
                .recipient = TUSB_REQ_RCPT_DEVICE,
                .type = TUSB_REQ_TYPE_STANDARD,
                .direction = TUSB_DIR_IN,
            },
            .bRequest = TUSB_REQ_GET_DESCRIPTOR,
            .wValue = TU_U16(0x03,0x04),
            .wIndex = TU_U16(0x04,0x09),
            .wLength = 0xB2
        };
        tuh_xfer_t xfer = {
            .daddr       = dev_addr,
            .ep_addr     = 0,//xid_itf->ep_in,
            .setup       = &xfer_ctrl_req,
            .buffer      = recvBuf,
            .complete_cb = NULL,
            //.user_data   = user_data
        };
        tuh_control_xfer(&xfer);
    }
}

void XInputAuthUSBListener::unmount(uint8_t dev_addr) {
    // Do not reset dongle_ready on unmount (Magic-X will remount but still be ready)
    if ( dev_addr == xinput_dev_addr ) {
        mounted = false;
        printf("[XInputAuthUSBListner] Unmounted!\n");
        controllerIdRecv = false;
    }
}

void XInputAuthUSBListener::report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {
    if ( mounted == false )
        return;

/*
    size_t retSize = 0;
    switch(report[0]) {
        case 0x81: // Get Serial
            uint8_t serial[0x0B];
            read_serial(serial, sizeof(serial));
            xsm3_set_serial(serial);
            xsm3_initialise_state();
            xsm3_set_identification_data(xsm3_id_data_ms_controller);
            memcpy(requestBuffer, xsm3_id_data_ms_controller, sizeof(xsm3_id_data_ms_controller));
            retSize = sizeof(xsm3_id_data_ms_controller);
        case 0x82: // Do Challenge Init
            xsm3_do_challenge_init((uint8_t *)requestBuffer);
            break;
        case 0x83: // Challenge Response
            memcpy(requestBuffer, xsm3_challenge_response, sizeof(xsm3_challenge_response));
            retSize = sizeof(xsm3_challenge_response);
        case 0x87: // Verify Challenge
            xsm3_do_challenge_verify((uint8_t *)requestBuffer);
            break;
        case 0x84: // Nothing??
            break;
        case 0x86: // Get Current State
            short state = 2;  // 1 = in-progress, 2 = complete
            memcpy(requestBuffer, &state, sizeof(state));
            retSize = sizeof(state);
        default:
    }
*/
}

void XInputAuthUSBListener::process() {
    if ( mounted != true )
        return;

    // Get Serial ID - 0x81
    if ( state == X360State_GetSerial ) {
        uint8_t idBuf[29];
        memset(idBuf, 0, 29);
        if ( tuh_xinput_receive_vendor_report(xinput_dev_addr, xinput_instance, 0x81, 0x5B17, 0x01, 29, idBuf) == true ) {
            printf("Received id buf!\n");
        } else {
            printf("Could not received ID Buf\n");
        }
        state = X360State_Idle;
    }

    // Do Challenge Init - 0x82 (send console -> host)

    // Get Challenge Response - 0x83 (send console -> host)

    // ?? - 0x84

    // Get State - 0x86 (1 = in-progress, 2 = complete)

    // ?? - 0x87
}
