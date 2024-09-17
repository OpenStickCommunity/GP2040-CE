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

uint8_t endpoint_in = 0;
uint8_t endpoint_out = 0;
uint8_t xinput_out_buffer[XINPUT_OUT_SIZE] = {};

#include <stdio.h>
#include "pico/stdlib.h"

static bool authDriverPresent = false;

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

static uint16_t xinput_open(uint8_t rhport, tusb_desc_interface_t const *itf_descriptor, uint16_t max_length)
{
	uint16_t driver_length = sizeof(tusb_desc_interface_t) + (itf_descriptor->bNumEndpoints * sizeof(tusb_desc_endpoint_t)) + 16;

	TU_VERIFY(max_length >= driver_length, 0);

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
	return driver_length;
}

static bool xinput_device_control_request(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
	(void)rhport;
	(void)stage;
	(void)request;
/*
	// if authentication is present
	if ( authDriverPresent &&
		request->bmRequestType == (USB_SETUP_DEVICE_TO_HOST | USB_SETUP_RECIPIENT_INTERFACE | USB_SETUP_TYPE_VENDOR)) {
		switch(request->bRequest) {
			case 0x81:
				uint8_t serial[0x0B];
				return sizeof(id_data_ms_controller);
			case 0x82:
				return 0;
			case 0x83:
				memcpy(requestBuffer, challenge_response, sizeof(challenge_response));
				return sizeof(challenge_response);
			case 0x84:
				break;
			case 0x86:
				short state = 2;  // 1 = in-progress, 2 = complete
				memcpy(&request->wValue, &state, sizeof(state));
				return sizeof(state);
			case 0x87:
				break;
			default:
				break;
		};
	}
*/
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
	/*
	GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
	if ( gamepadOptions.xinputAuthType == InputModeAuthType::INPUT_MODE_AUTH_TYPE_USB )  {
		authDriver = new XInputAuth();
		if ( authDriver->available() ) {
			authDriver->initialize();
			authDriverPresent = true; // for callbacks
		}
	}
	*/
}

USBListener * XInputDriver::get_usb_auth_listener() {
	if ( authDriver != nullptr && authDriver->available() ) {
		return authDriver->getListener();
	}
	return nullptr;
}

void XInputDriver::process(Gamepad * gamepad) {
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

// Only XboxOG and Xbox One use vendor control xfer cb
bool XInputDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
	return false;
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
