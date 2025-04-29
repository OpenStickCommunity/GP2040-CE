/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/xinput/XInputDriver.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"

#define USB_SETUP_DEVICE_TO_HOST 0x80
#define USB_SETUP_HOST_TO_DEVICE 0x00
#define USB_SETUP_TYPE_VENDOR    0x40
#define USB_SETUP_TYPE_CLASS     0x20
#define USB_SETUP_TYPE_STANDARD  0x00
#define USB_SETUP_RECIPIENT_INTERFACE   0x01
#define USB_SETUP_RECIPIENT_DEVICE      0x00
#define USB_SETUP_RECIPIENT_ENDPOINT    0x02
#define USB_SETUP_RECIPIENT_OTHER       0x03

#define REQ_GET_OS_FEATURE_DESCRIPTOR 0x20
#define DESC_EXTENDED_COMPATIBLE_ID_DESCRIPTOR 0x0004
#define DESC_EXTENDED_PROPERTIES_DESCRIPTOR 0x0005

#define XINPUT_OUT_SIZE 32

#define XINPUT_DESC_TYPE_RESERVED 0x21
#define XINPUT_SECURITY_DESC_TYPE_RESERVED 0x41

static uint8_t endpoint_in = 0;
static uint8_t endpoint_out = 0;
static uint8_t xinput_out_buffer[XINPUT_OUT_SIZE] = {};
static XInputAuthData * xinputAuthData = nullptr;

// Move to Proto Enums
typedef enum
{
    XINPUT_PLED_OFF       = 0x00, // All off
    XINPUT_PLED_BLINKALL  = 0x01, // All blinking
    XINPUT_PLED_FLASH1    = 0x02, // 1 flashes, then on
    XINPUT_PLED_FLASH2    = 0x03, // 2 flashes, then on
    XINPUT_PLED_FLASH3    = 0x04, // 3 flashes, then on
    XINPUT_PLED_FLASH4    = 0x05, // 4 flashes, then on
    XINPUT_PLED_ON1       = 0x06, // 1 on
    XINPUT_PLED_ON2       = 0x07, // 2 on
    XINPUT_PLED_ON3       = 0x08, // 3 on
    XINPUT_PLED_ON4       = 0x09, // 4 on
    XINPUT_PLED_ROTATE    = 0x0A, // Rotating (e.g. 1-2-4-3)
    XINPUT_PLED_BLINK     = 0x0B, // Blinking*
    XINPUT_PLED_SLOWBLINK = 0x0C, // Slow blinking*
    XINPUT_PLED_ALTERNATE = 0x0D, // Alternating (e.g. 1+4-2+3), then back to previous*
} XInputPLEDPattern;

static void xinput_init(void) {
}

static void xinput_reset(uint8_t rhport) {
    (void)rhport;
}

static uint16_t xinput_open(uint8_t rhport, tusb_desc_interface_t const *itf_descriptor, uint16_t max_length) {
    uint16_t driver_length = 0;
    // Xbox 360 Vendor USB Interfaces: Control, Audio, Plug-in, Security
    if ( TUSB_CLASS_VENDOR_SPECIFIC == itf_descriptor->bInterfaceClass) {
        driver_length = sizeof(tusb_desc_interface_t) + (itf_descriptor->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
        TU_VERIFY(max_length >= driver_length, 0);

        tusb_desc_interface_t *p_desc = (tusb_desc_interface_t *)itf_descriptor;
        // Xbox 360 Interfaces (Control 0x01, Audio 0x02, Plug-in 0x03)
        if (itf_descriptor->bInterfaceSubClass == 0x5D &&
                ((itf_descriptor->bInterfaceProtocol == 0x01 ) ||
                (itf_descriptor->bInterfaceProtocol == 0x02 ) ||
                (itf_descriptor->bInterfaceProtocol == 0x03 )) ) {
            // Get Xbox 360 Definition
            p_desc = (tusb_desc_interface_t *)tu_desc_next(p_desc);
            TU_VERIFY(XINPUT_DESC_TYPE_RESERVED == p_desc->bDescriptorType, 0);
            driver_length += p_desc->bLength;
            p_desc = (tusb_desc_interface_t *)tu_desc_next(p_desc);
            // Control Endpoints are used for gamepad input/output
            if ( itf_descriptor->bInterfaceProtocol == 0x01 ) {
                TU_ASSERT(usbd_open_edpt_pair(rhport, (const uint8_t*)p_desc, itf_descriptor->bNumEndpoints,
                            TUSB_XFER_INTERRUPT, &endpoint_out, &endpoint_in), 0);
            }
        // Xbox 360 Security Interface
        } else if (itf_descriptor->bInterfaceSubClass == 0xFD &&
                itf_descriptor->bInterfaceProtocol == 0x13) {
            // Xinput reserved endpoint
            //-------------- Xinput Descriptor --------------//
            p_desc = (tusb_desc_interface_t *)tu_desc_next(p_desc);
            TU_VERIFY(XINPUT_SECURITY_DESC_TYPE_RESERVED == p_desc->bDescriptorType, 0);
            driver_length += p_desc->bLength;
        }
    }

    return driver_length;
}

static bool xinput_device_control_request(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    (void)rhport;
    (void)stage;
    (void)request;

    return true;
}

static bool xinput_control_complete(uint8_t rhport, tusb_control_request_t const *request)
{
    (void)rhport;
    (void)request;

    return true;
}

static bool xinput_xfer_callback(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes)
{
    (void)rhport;
    (void)result;
    (void)xferred_bytes;

    if (ep_addr == endpoint_out)
        usbd_edpt_xfer(0, endpoint_out, xinput_out_buffer, XINPUT_OUT_SIZE);

    return true;
}

void XInputDriver::initialize() {
    xinputReport = {
        .report_id = 0,
        .report_size = XINPUT_ENDPOINT_SIZE,
        .buttons1 = 0,
        .buttons2 = 0,
        .lt = 0,
        .rt = 0,
        .lx = GAMEPAD_JOYSTICK_MID,
        .ly = GAMEPAD_JOYSTICK_MID,
        .rx = GAMEPAD_JOYSTICK_MID,
        .ry = GAMEPAD_JOYSTICK_MID,
        ._reserved = { },
    };

    class_driver = {
    #if CFG_TUSB_DEBUG >= 2
        .name = "XINPUT",
    #endif
        .init = xinput_init,
        .reset = xinput_reset,
        .open = xinput_open,
        .control_xfer_cb = xinput_device_control_request,
        .xfer_cb = xinput_xfer_callback,
        .sof = NULL
    };

    xAuthDriver = nullptr;
}

void XInputDriver::initializeAux() {
    xAuthDriver = nullptr;
    // AUTH DRIVER NON-FUNCTIONAL FOR NOW
    GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
    if ( gamepadOptions.xinputAuthType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB )  {
        xAuthDriver = new XInputAuth();
        if ( xAuthDriver->available() ) {
            xAuthDriver->initialize();
            xinputAuthData = xAuthDriver->getAuthData();
        }
    }
}

USBListener * XInputDriver::get_usb_auth_listener() {
    if ( xAuthDriver != nullptr && xAuthDriver->available() ) {
        return xAuthDriver->getListener();
    }
    return nullptr;
}

bool XInputDriver::getAuthEnabled() {
    return (xAuthDriver != nullptr);
}

bool XInputDriver::process(Gamepad * gamepad) {
    Gamepad * processedGamepad = Storage::getInstance().GetProcessedGamepad();

    xinputReport.buttons1 = 0
        | (gamepad->pressedUp()    ? XBOX_MASK_UP    : 0)
        | (gamepad->pressedDown()  ? XBOX_MASK_DOWN  : 0)
        | (gamepad->pressedLeft()  ? XBOX_MASK_LEFT  : 0)
        | (gamepad->pressedRight() ? XBOX_MASK_RIGHT : 0)
        | (gamepad->pressedS2()    ? XBOX_MASK_START : 0)
        | (gamepad->pressedS1()    ? XBOX_MASK_BACK  : 0)
        | (gamepad->pressedL3()    ? XBOX_MASK_LS    : 0)
        | (gamepad->pressedR3()    ? XBOX_MASK_RS    : 0)
    ;

    xinputReport.buttons2 = 0
        | (gamepad->pressedL1() ? XBOX_MASK_LB   : 0)
        | (gamepad->pressedR1() ? XBOX_MASK_RB   : 0)
        | (gamepad->pressedA1() ? XBOX_MASK_HOME : 0)
        | (gamepad->pressedB1() ? XBOX_MASK_A    : 0)
        | (gamepad->pressedB2() ? XBOX_MASK_B    : 0)
        | (gamepad->pressedB3() ? XBOX_MASK_X    : 0)
        | (gamepad->pressedB4() ? XBOX_MASK_Y    : 0)
    ;

    xinputReport.lx = static_cast<int16_t>(gamepad->state.lx) + INT16_MIN;
    xinputReport.ly = static_cast<int16_t>(~gamepad->state.ly) + INT16_MIN;
    xinputReport.rx = static_cast<int16_t>(gamepad->state.rx) + INT16_MIN;
    xinputReport.ry = static_cast<int16_t>(~gamepad->state.ry) + INT16_MIN;

    if (gamepad->hasAnalogTriggers)
    {
        xinputReport.lt = gamepad->pressedL2() ? 0xFF : gamepad->state.lt;
        xinputReport.rt = gamepad->pressedR2() ? 0xFF : gamepad->state.rt;
    }
    else
    {
        xinputReport.lt = gamepad->pressedL2() ? 0xFF : 0;
        xinputReport.rt = gamepad->pressedR2() ? 0xFF : 0;
    }

    bool reportSent = false;

    // compare against previous report and send new
    if ( memcmp(last_report, &xinputReport, sizeof(XInputReport)) != 0) {
        if ( tud_ready() &&											// Is the device ready?
            (endpoint_in != 0) && (!usbd_edpt_busy(0, endpoint_in)) ) // Is the IN endpoint available?
        {
            usbd_edpt_claim(0, endpoint_in);								// Take control of IN endpoint
            usbd_edpt_xfer(0, endpoint_in, (uint8_t *)&xinputReport, sizeof(XInputReport)); // Send report buffer
            usbd_edpt_release(0, endpoint_in);								// Release control of IN endpoint
            memcpy(last_report, &xinputReport, sizeof(XInputReport)); // save if we sent it
            reportSent = true;
        }
    }

    // clear potential initial uncaught data in endpoint_out from before registration of xfer_cb
    if (tud_ready() &&
        (endpoint_out != 0) && (!usbd_edpt_busy(0, endpoint_out)))
    {
        usbd_edpt_claim(0, endpoint_out);									 // Take control of OUT endpoint
        usbd_edpt_xfer(0, endpoint_out, xinput_out_buffer, XINPUT_OUT_SIZE); 		 // Retrieve report buffer
        usbd_edpt_release(0, endpoint_out);									 // Release control of OUT endpoint
    }

    //---------------
    if (memcmp(xinput_out_buffer, featureBuffer, XINPUT_OUT_SIZE) != 0) { // check if new write to xinput_out_buffer from xinput_xfer_callback
        memcpy(featureBuffer, xinput_out_buffer, XINPUT_OUT_SIZE);
        switch (featureBuffer[0]) {
            case 0x00:
                if (featureBuffer[1] == 0x08) {
                    if (processedGamepad->auxState.haptics.leftActuator.enabled) {
                        processedGamepad->auxState.haptics.leftActuator.active = (featureBuffer[3] > 0);
                        processedGamepad->auxState.haptics.leftActuator.intensity = featureBuffer[3];
                    }
                    if (processedGamepad->auxState.haptics.rightActuator.enabled) {
                        processedGamepad->auxState.haptics.rightActuator.active = (featureBuffer[4] > 0);
                        processedGamepad->auxState.haptics.rightActuator.intensity = featureBuffer[4];
                    }
                }
                break;
            case 0x01:
                // Player LED
                if (featureBuffer[1] == 0x03) {
                    // determine the player ID based on LED status
                    processedGamepad->auxState.playerID.active = true;
                    processedGamepad->auxState.playerID.ledValue = featureBuffer[2];

                    if ( featureBuffer[2] == XINPUT_PLED_ON1 ) {
                        processedGamepad->auxState.playerID.value = 1;
                    } else if ( featureBuffer[2] == XINPUT_PLED_ON2 ) {
                        processedGamepad->auxState.playerID.value = 2;
                    } else if ( featureBuffer[2] == XINPUT_PLED_ON3 ) {
                        processedGamepad->auxState.playerID.value = 3;
                    } else if ( featureBuffer[2] == XINPUT_PLED_ON4 ) {
                        processedGamepad->auxState.playerID.value = 4;
                    } else {
                        processedGamepad->auxState.playerID.value = 0;
                    }
                }
                break;
        }
    }

    return reportSent;
}

void XInputDriver::processAux() {
    if ( xAuthDriver != nullptr && xAuthDriver->available() ) {
        xAuthDriver->process();
    }
}

// tud_hid_get_report_cb
uint16_t XInputDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    memcpy(buffer, &xinputReport, sizeof(XInputReport));
    return sizeof(XInputReport);
}

// Only respond to vendor control xfers if we have a mounted x360 device
bool XInputDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
  // Do nothing if we have no auth driver
    if ( xAuthDriver == nullptr || !xAuthDriver->available() ) {
        return false;
    }

    uint16_t len = 0;
    if (request->bmRequestType_bit.direction == TUSB_DIR_IN) {
        // Write IN data on control_stage_setup only
        if (stage == CONTROL_STAGE_SETUP) {
            uint16_t state = 1; // 1 = in-progress, 2 = complete
            switch (request->bRequest) {
                    case XSM360_GET_SERIAL:
                        // Stall if we don't have a dongle ready
                        if ( xinputAuthData->dongle_ready == false ) {
                            return false;
                        }
                        len = X360_AUTHLEN_DONGLE_SERIAL;
                        memcpy(tud_buffer, xinputAuthData->dongleSerial, len);
                        break;
                    case XSM360_RESPOND_CHALLENGE:
                        if ( xinputAuthData->xinputState == GPAuthState::send_auth_dongle_to_console ) {
                            memcpy(tud_buffer, xinputAuthData->passthruBuffer, xinputAuthData->passthruBufferLen);
                            len = xinputAuthData->passthruBufferLen;
                        } else {
                            // Stall if we don't have a dongle ready
                            return false;
                        }
                        break;
                    case XSM360_AUTH_KEEPALIVE:
                        len = 0;
                        break;
                    case XSM360_REQUEST_STATE:
                        // State Ready = 2, Not-Ready = 1
                        if ( xinputAuthData->xinputState == GPAuthState::send_auth_dongle_to_console ) {
                            state = 2;
                        } else {
                            state = 1;
                        }
                        memcpy(tud_buffer, &state, sizeof(state));
                        len = sizeof(state);
                        break;
                    default:
                        break;
            };
            tud_control_xfer(rhport, request, tud_buffer, len);
        }
    } else if (request->bmRequestType_bit.direction == TUSB_DIR_OUT) {
        if (stage == CONTROL_STAGE_SETUP ) { // Pass on output setup in DIR OUT stage
            tud_control_xfer(rhport, request, tud_buffer, request->wLength);
        } else if ( stage == CONTROL_STAGE_DATA ) {
            // Buf is filled, we can save the data to our auth
            switch (request->bRequest) {
                    case XSM360AuthRequest::XSM360_INIT_AUTH:
                        if ( xinputAuthData->xinputState == GPAuthState::auth_idle_state ) {
                            memcpy(xinputAuthData->passthruBuffer, tud_buffer, request->wLength);
                            xinputAuthData->passthruBufferLen = request->wLength;
                            xinputAuthData->passthruBufferID = XSM360AuthRequest::XSM360_INIT_AUTH;
                            xinputAuthData->xinputState = GPAuthState::send_auth_console_to_dongle;
                        }
                        break;
                    case XSM360AuthRequest::XSM360_VERIFY_AUTH:
                        memcpy(xinputAuthData->passthruBuffer, tud_buffer, request->wLength);
                        xinputAuthData->passthruBufferLen = request->wLength;
                        xinputAuthData->passthruBufferID = XSM360AuthRequest::XSM360_VERIFY_AUTH;
                        xinputAuthData->xinputState = GPAuthState::send_auth_console_to_dongle;
                        break;
                    default:
                        break;
            };
        }
    }

    return true;
}

const uint16_t * XInputDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
    char *value;
    // Check for override settings
    GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
    if ( gamepadOptions.usbDescOverride == true ) {
        switch(index) {
            case 1:
                value = gamepadOptions.usbDescManufacturer;
                break;
            case 2:
                value = gamepadOptions.usbDescProduct;
                break;
            case 3:
                value = gamepadOptions.usbDescVersion;
            default:
                value = (char *)xinput_get_string_descriptor(index);
                break;
        }
    } else {
        value = (char *)xinput_get_string_descriptor(index);
    }
    return getStringDescriptor((const char*)value, index); // getStringDescriptor returns a static array
}

const uint8_t * XInputDriver::get_descriptor_device_cb() {
    // Check for override settings
    GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
    if ( gamepadOptions.usbOverrideID == true ) {
        static uint8_t modified_device_descriptor[18];
        memcpy(modified_device_descriptor, xinput_device_descriptor, sizeof(xinput_device_descriptor));
        memcpy(&modified_device_descriptor[8], (uint8_t*)&gamepadOptions.usbVendorID, sizeof(uint16_t)); // Vendor ID
        memcpy(&modified_device_descriptor[10], (uint8_t*)&gamepadOptions.usbProductID, sizeof(uint16_t)); // Product ID
        return (const uint8_t*)modified_device_descriptor;
    }
    return xinput_device_descriptor;
}

const uint8_t * XInputDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return nullptr;
}

const uint8_t * XInputDriver::get_descriptor_configuration_cb(uint8_t index) {
    return xinput_configuration_descriptor;
}

const uint8_t * XInputDriver::get_descriptor_device_qualifier_cb() {
    return nullptr;
}

uint16_t XInputDriver::GetJoystickMidValue() {
    return GAMEPAD_JOYSTICK_MID;
}
