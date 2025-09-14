#include "drivers/xinput/XInputAuth.h"
#include "drivers/xinput/XInputAuthUSBListener.h"
#include "peripheralmanager.h"

#include "xsm3/xsm3.h"

void XInputAuth::initialize() {
    if ( !available() ) {
        return;
    }

    listener = nullptr;
    if (authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) {
        listener = new XInputAuthUSBListener();
        xinputAuthData.xinputState = auth_idle_state;
        xinputAuthData.authCompleted = false;
        ((XInputAuthUSBListener*)listener)->setup();
        ((XInputAuthUSBListener*)listener)->setAuthData(&xinputAuthData);
    } else {
        // Generate a serial number from the pico's unique ID
        uint8_t serial[0x0C];
		pico_unique_board_id_t id;
		pico_get_unique_board_id(&id);
        for(int i = 0; i < 0x0C; i++) {
            serial[i] = 'A' + (id.id[i]%25); // some alphanumeric from 'A' to 'Z'
        }
        xsm3_set_vid_pid(serial, 0x045E, 0x028E);
        xsm3_initialise_state();
        xsm3_set_identification_data(xsm3_id_data_ms_controller);
        xinputAuthData.xinputState = auth_idle_state;
        xinputAuthData.authCompleted = false;
        xinputAuthData.dongle_ready = true;
        memcpy(xinputAuthData.dongleSerial, xsm3_id_data_ms_controller, X360_AUTHLEN_DONGLE_SERIAL);
    }
}

bool XInputAuth::available() {
    // Move options over to their own ps4 data structure or gamepad?
    if ( authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) {
        return (PeripheralManager::getInstance().isUSBEnabled(0));
    } else {
        return true;
    }
}

void XInputAuth::process() {
    // Use USB dongle
    if ( authType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB ) {
        ((XInputAuthUSBListener*)listener)->process(); // send to USB dongle
        return;
    }

    // Process Xbox360 Console Request
    if ( xinputAuthData.xinputState == GPAuthState::send_auth_console_to_dongle ) {
        if ( xinputAuthData.passthruBufferID == XSM360AuthRequest::XSM360_INIT_AUTH ) {
            xsm3_do_challenge_init(xinputAuthData.passthruBuffer);
            memcpy(xinputAuthData.passthruBuffer, xsm3_challenge_response, X360_AUTHLEN_DONGLE_INIT);
            xinputAuthData.passthruBufferLen = X360_AUTHLEN_DONGLE_INIT;
            xinputAuthData.xinputState = GPAuthState::send_auth_dongle_to_console;
        } else if ( xinputAuthData.passthruBufferID == XSM360AuthRequest::XSM360_VERIFY_AUTH ) {
            xsm3_do_challenge_verify(xinputAuthData.passthruBuffer);
            memcpy(xinputAuthData.passthruBuffer, xsm3_challenge_response, X360_AUTHLEN_CHALLENGE);
            xinputAuthData.passthruBufferLen = X360_AUTHLEN_CHALLENGE;
            xinputAuthData.xinputState = GPAuthState::send_auth_dongle_to_console;
        }
    }
}
