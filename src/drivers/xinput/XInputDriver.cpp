/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/xinput/XInputDriver.h"

static XInputReport xinputReport
{
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


XInputReport *Gamepad::getXInputReport()
{
	xinputReport.buttons1 = 0
		| (pressedUp()    ? XBOX_MASK_UP    : 0)
		| (pressedDown()  ? XBOX_MASK_DOWN  : 0)
		| (pressedLeft()  ? XBOX_MASK_LEFT  : 0)
		| (pressedRight() ? XBOX_MASK_RIGHT : 0)
		| (pressedS2()    ? XBOX_MASK_START : 0)
		| (pressedS1()    ? XBOX_MASK_BACK  : 0)
		| (pressedL3()    ? XBOX_MASK_LS    : 0)
		| (pressedR3()    ? XBOX_MASK_RS    : 0)
	;

	xinputReport.buttons2 = 0
		| (pressedL1() ? XBOX_MASK_LB   : 0)
		| (pressedR1() ? XBOX_MASK_RB   : 0)
		| (pressedA1() ? XBOX_MASK_HOME : 0)
		| (pressedB1() ? XBOX_MASK_A    : 0)
		| (pressedB2() ? XBOX_MASK_B    : 0)
		| (pressedB3() ? XBOX_MASK_X    : 0)
		| (pressedB4() ? XBOX_MASK_Y    : 0)
	;

	xinputReport.lx = static_cast<int16_t>(state.lx) + INT16_MIN;
	xinputReport.ly = static_cast<int16_t>(~state.ly) + INT16_MIN;
	xinputReport.rx = static_cast<int16_t>(state.rx) + INT16_MIN;
	xinputReport.ry = static_cast<int16_t>(~state.ry) + INT16_MIN;

	if (hasAnalogTriggers)
	{
		xinputReport.lt = pressedL2() ? 0xFF : state.lt;
		xinputReport.rt = pressedR2() ? 0xFF : state.rt;
	}
	else
	{
		xinputReport.lt = pressedL2() ? 0xFF : 0;
		xinputReport.rt = pressedR2() ? 0xFF : 0;
	}

	return &xinputReport;
}

// For X-Input interface initialization
static uint8_t ednpoint_in;
static uint8_t ednpoint_out;

static void xinput_init(void);
static void xinput_reset(uint8_t rhport);
static uint16_t xinput_open(uint8_t rhport, tusb_desc_interface_t const *itf_descriptor, uint16_t max_length);
static bool xinput_device_control_request(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request);
static bool xinput_xfer_callback(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes);

virtual void XInputDriver::initialize(InputMode mode) {
    class_driver = new usbd_class_driver_t {
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

    inputMode = mode;
}

virtual void XInputDriver::update() {
    // Nothing
}

virtual bool XInputDriver::send_report(void *report, uint16_t report_size) {
	bool sent = false;

	if (
		tud_ready() &&											// Is the device ready?
		(endpoint_in != 0) && (!usbd_edpt_busy(0, endpoint_in)) // Is the IN endpoint available?
	)
	{
		usbd_edpt_claim(0, endpoint_in);								// Take control of IN endpoint
		usbd_edpt_xfer(0, endpoint_in, (uint8_t *)report, report_size); // Send report buffer
		usbd_edpt_release(0, endpoint_in);								// Release control of IN endpoint
		sent = true;
	}

	return sent;
}

virtual void XInputDriver::receive_report(int8_t *buffer) {
    
}

virtual void XInputDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    
}

virtual void XInputDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
    
}

virtual void XInputDriver::control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    
}

// Local USB Callbacks
static void xinput_init(void) {
}

static void xinput_reset(uint8_t rhport) {
	(void)rhport;
}

static uint16_t xinput_open(uint8_t rhport, tusb_desc_interface_t const *itf_descriptor, uint16_t max_length) {
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

static bool xinput_device_control_request(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
	(void)rhport;
	(void)stage;
	(void)request;
	return true;
}

static bool xinput_control_complete(uint8_t rhport, tusb_control_request_t const *request) {
	(void)rhport;
	(void)request;
	return true;
}

static bool xinput_xfer_callback(uint8_t rhport, uint8_t ep_addr, xfer_result_t result, uint32_t xferred_bytes) {
	(void)rhport;
	(void)result;
	(void)xferred_bytes;
	if (ep_addr == endpoint_out)
		usbd_edpt_xfer(0, endpoint_out, xinput_out_buffer, XINPUT_OUT_SIZE);
	return true;
}
