/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/xinput/XInputDriver.h"
#include "drivers/xinput/XInputAuth.h"
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
#define CFG_TUD_XINPUT 4

#define XINPUT_DESC_TYPE_RESERVED 0x21
#define XINPUT_SECURITY_DESC_TYPE_RESERVED 0x41

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF
//--------------------------------------------------------------------+
typedef struct {
    uint8_t itf_num;
    uint8_t ep_in;
    uint8_t ep_out;         // optional Out endpoint
    uint8_t boot_protocol;  // Boot mouse or keyboard
    bool boot_mode;         // default = false (Report)

    CFG_TUSB_MEM_ALIGN uint8_t epin_buf[XINPUT_OUT_SIZE];
    CFG_TUSB_MEM_ALIGN uint8_t epout_buf[XINPUT_OUT_SIZE];
} xinputd_interface_t;

CFG_TUSB_MEM_SECTION static xinputd_interface_t _xinputd_itf[CFG_TUD_XINPUT];

uint8_t endpoint_in = 0;
uint8_t endpoint_out = 0;
uint8_t xinput_out_buffer[XINPUT_OUT_SIZE] = {};

static bool authDriverPresent = false;

static void xinput_init(void) {
}

static void xinput_reset(uint8_t rhport) {
	(void)rhport;
}

static uint16_t xinput_open(uint8_t rhport, tusb_desc_interface_t const *itf_descriptor, uint16_t max_length)
{
	uint16_t driver_length;
    // Xbox 360 Vendor USB Interfaces: Control, Audio, Plug-in, Security
    if ( TUSB_CLASS_VENDOR_SPECIFIC == itf_descriptor->bInterfaceClass) {
        driver_length = sizeof(tusb_desc_interface_t) + (itf_descriptor->bNumEndpoints * sizeof(tusb_desc_endpoint_t));
        TU_VERIFY(max_length >= driver_length, 0);
        tusb_desc_interface_t *p_desc = (tusb_desc_interface_t *)itf_descriptor;
        // Xbox 360 Interfaces (Control 0x01, Audio 0x02, Plug-in 0x03)
        if (itf_descriptor->bInterfaceSubClass == 0x5D &&
                (itf_descriptor->bInterfaceProtocol == 0x01 ) ||
                (itf_descriptor->bInterfaceProtocol == 0x02 ) ||
                (itf_descriptor->bInterfaceProtocol == 0x03 ) ) {
            // Get Xbox 360 Definition
            p_desc = (tusb_desc_interface_t *)tu_desc_next(p_desc);
            TU_VERIFY(XINPUT_DESC_TYPE_RESERVED == p_desc->bDescriptorType, 0);
            driver_length += p_desc->bLength;
            // Xbox 360 CONTROL ONLY: ignore audio and plug-in for our purposes
            if ( itf_descriptor->bInterfaceProtocol == 0x01 ) {
                uint8_t const *current_descriptor = tu_desc_next(itf_descriptor);
                uint8_t found_endpoints = 0;
                while ((found_endpoints < itf_descriptor->bNumEndpoints) && (driver_length <= max_length))
                {
                    tusb_desc_endpoint_t const *endpoint_descriptor = (tusb_desc_endpoint_t const *)current_descriptor;
                    if (TUSB_DESC_ENDPOINT == tu_desc_type(endpoint_descriptor))
                    {
                        TU_ASSERT(usbd_edpt_open(rhport, endpoint_descriptor));

                        if (tu_edpt_dir(endpoint_descriptor->bEndpointAddress) == TUSB_DIR_IN)
                            endpoint_in = endpoint_descriptor->bEndpointAddress;
                        else
                            endpoint_out = endpoint_descriptor->bEndpointAddress;

                        ++found_endpoints;
                    }

                    current_descriptor = tu_desc_next(current_descriptor);
                }
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

	// if authentication is present
	if ( authDriverPresent &&
		request->bmRequestType == (USB_SETUP_DEVICE_TO_HOST | USB_SETUP_RECIPIENT_INTERFACE | USB_SETUP_TYPE_VENDOR)) {
		switch(request->bRequest) {
			case 0x81:
				uint8_t serial[0x0B];
                printf("[xinput_device_control_request] 0x81 [Get Serial] %08x ...", serial);
            	//return sizeof(id_data_ms_controller);
                break;
			case 0x82:
                printf("[xinput_device_control_request] 0x82 [???]");
				//return 0;
			case 0x83:
                printf("[xinput_device_control_request] 0x83 [Challenge Response]");
				//memcpy(requestBuffer, challenge_response, sizeof(challenge_response));
            	//return sizeof(challenge_response);
                break;
			case 0x84:
                printf("[xinput_device_control_request] 0x84 [???]");
				break;
			case 0x86:
				//short state = 2;  // 1 = in-progress, 2 = complete
				//memcpy(&request->wValue, &state, sizeof(state));
				//return sizeof(state);
                printf("[xinput_device_control_request] 0x84 [Get Challenge State]");
                break;
			case 0x87:
				break;
			default:
				break;
		};
	}
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

	authDriver = nullptr;
}

void XInputDriver::initializeAux() {
	authDriver = nullptr;
	// AUTH DRIVER NON-FUNCTIONAL FOR NOW
	GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
	//if ( gamepadOptions.xinputAuthType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB )  {
		authDriver = new XInputAuth();
		if ( authDriver->available() ) {
			authDriver->initialize();
			authDriverPresent = true; // for callbacks
		}
	//}
}

USBListener * XInputDriver::get_usb_auth_listener() {
	if ( authDriver != nullptr && authDriver->available() ) {
		return authDriver->getListener();
	}
	return nullptr;
}

void XInputDriver::process(Gamepad * gamepad, uint8_t * outBuffer) {
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

	// compare against previous report and send new
	if ( memcmp(last_report, &xinputReport, sizeof(XInputReport)) != 0) {
		if ( tud_ready() &&											// Is the device ready?
			(endpoint_in != 0) && (!usbd_edpt_busy(0, endpoint_in)) ) // Is the IN endpoint available?
		{
			usbd_edpt_claim(0, endpoint_in);								// Take control of IN endpoint
			usbd_edpt_xfer(0, endpoint_in, (uint8_t *)&xinputReport, sizeof(XInputReport)); // Send report buffer
			usbd_edpt_release(0, endpoint_in);								// Release control of IN endpoint
			memcpy(last_report, &xinputReport, sizeof(XInputReport)); // save if we sent it
		}
	}

	// check for player LEDs
	if (tud_ready() &&
		(endpoint_out != 0) && (!usbd_edpt_busy(0, endpoint_out)))
	{
		usbd_edpt_claim(0, endpoint_out);									 // Take control of OUT endpoint
		usbd_edpt_xfer(0, endpoint_out, outBuffer, XINPUT_OUT_SIZE); 		 // Retrieve report buffer
		usbd_edpt_release(0, endpoint_out);									 // Release control of OUT endpoint
	}
}

void XInputDriver::processAux() {
	if ( authDriver != nullptr && authDriver->available() ) {
		((XInputAuth*)authDriver)->process();
	}
}

// tud_hid_get_report_cb
uint16_t XInputDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	memcpy(buffer, &xinputReport, sizeof(XInputReport));
	return sizeof(XInputReport);
}

// Only PS4 does anything with set report
void XInputDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

static const uint8_t xsm3_id_data_ms_controller[] = {
    0x49, 0x4B, 0x00, 0x00, 0x17, 0xFA, 0xAB, 0x2D, 0xFA, 0xFB, 0x55, 0x49, 0xCE, 0x02, 0x03, 0x20,
    0x00, 0x00, 0x80, 0x02, 0x5E, 0x04, 0x8E, 0x02, 0x03, 0x00, 0x01, 0x01, 0xD9
};

// Class variable
static uint8_t buf[255];

// Only respond to vendor control xfers if we have a mounted x360 device
bool XInputDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    // Do nothing if we have no auth driver
	if ( authDriver == nullptr || !authDriver->available() ) {
        printf("[XInputDriver] No auth driver available\n");
        return true;
	}

    printf("[XInputDriver] Got Vendor Control XFER\nrequest->bmRequestType_bit.direction == %02x\nstage == %02x\nrequest->bRequest=%02x\n\n", request->bmRequestType_bit.direction, stage);

    //if (request->bmRequestType == (USB_SETUP_DEVICE_TO_HOST | USB_SETUP_RECIPIENT_INTERFACE | USB_SETUP_TYPE_VENDOR) 
    //    && request->bRequest == XSM360_RESPOND_CHALLENGE) {
        //if (xbox_360_state == Auth1) {
        //    xbox_360_state = Auth2;
        //} else if (xbox_360_state == Auth2) {
        //    xbox_360_state = Authenticated;
        //    handle_auth_led();
        //}
    //}
    uint16_t len = 0;
    if (request->bmRequestType_bit.direction == TUSB_DIR_IN) {
        if (stage == CONTROL_STAGE_SETUP) {
            // we generate the buffer and send using a tud_control_xfer request
            uint8_t serial[0x0B];

            // read from auth driver
            short state = 2;  // 1 = in-progress, 2 = complete            

            // Generate a serial number from the pico's unique ID
		    //pico_unique_board_id_t id;

            switch (request->bRequest) {
                    case XSM360_GET_SERIAL:
                        printf("[XInputDriver] Serial Request!\n");
//                        pico_get_unique_board_id(&id);
//                        memcpy(uniqueSerial, (uint8_t*)&serial, PICO_UNIQUE_BOARD_ID_SIZE_BYTES);
                        //read_serial(serial, sizeof(serial));
                        //xsm3_set_serial(serial);
                        //xsm3_initialise_state();
                        //xsm3_set_identification_data(xsm3_id_data_ms_controller);
                        memcpy(buf, xsm3_id_data_ms_controller, sizeof(xsm3_id_data_ms_controller));
                        len = sizeof(xsm3_id_data_ms_controller);
                        break;
                    case XSM360_INIT_AUTH:
                        //xsm3_do_challenge_init((uint8_t *)requestBuffer);
                        len = 0;
                        break;
                    case XSM360_RESPOND_CHALLENGE:
                        //memcpy(requestBuffer, xsm3_challenge_response, sizeof(xsm3_challenge_response));
                        len = 0x30; // sizeof(xsm3_challenge_response);
                        break;
                    case XSM360_AUTH_KEEPALIVE:
                        len = 0;
                        break;
                    case XSM360_VERIFY_AUTH:
                        //xsm3_do_challenge_verify((uint8_t *)requestBuffer);
                        len = 0;
                        break;
                    case XSM360_REQUEST_STATE:
                        memcpy(buf, &state, sizeof(state));
                        len = sizeof(state);
                        break;
                    default:
                        return true;
            };
            tud_control_xfer(rhport, request, buf, len);
        }
    } else if (request->bmRequestType_bit.direction == TUSB_DIR_OUT) {
        if (stage == CONTROL_STAGE_SETUP ) {
            // Request buf in setup stage
            switch (request->bRequest) {
                    case XSM360_INIT_AUTH:
                        // request buf
                        len = request->wLength;
                        break;
                    case XSM360_VERIFY_AUTH:
                        // fill xsm3 with buf
                        len = request->wLength;
                        break;
                    default:
                        return true;
            };
            tud_control_xfer(rhport, request, buf, len);
        } else if ( stage == CONTROL_STAGE_DATA ) {
            // Buf is filled, we can save the data to our auth
            switch (request->bRequest) {
                    case XSM360_INIT_AUTH:
                        // fill xsm3 with buf
                        break;
                    case XSM360_VERIFY_AUTH:
                        // fill xsm3 with buf
                        break;
                    default:
                        return true;
            };
        }
    }

    return true;
}

const uint16_t * XInputDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)xinput_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * XInputDriver::get_descriptor_device_cb() {
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
