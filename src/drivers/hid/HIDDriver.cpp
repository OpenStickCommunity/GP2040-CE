/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/hid/HIDDriver.h"
#include "drivers/hid/HIDDescriptors.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"

static bool hid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
	return hidd_control_xfer_cb(rhport, stage, request);
}

void HIDDriver::initialize() {
	hidReport = {
		.buttons = 0,
		.direction = HID_HAT_NOTHING,
		.l_x_axis = HID_JOYSTICK_MID, .l_y_axis = HID_JOYSTICK_MID,
		.r_x_axis = HID_JOYSTICK_MID, .r_y_axis = HID_JOYSTICK_MID,
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "HID",
	#endif
		.init = hidd_init,
		.reset = hidd_reset,
		.open = hidd_open,
		.control_xfer_cb = hid_control_xfer_cb,
		.xfer_cb = hidd_xfer_cb,
		.sof = NULL
	};
}

// Generate HID report from gamepad and send to TUSB Device
bool HIDDriver::process(Gamepad * gamepad) {
	switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        hidReport.direction = HID_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   hidReport.direction = HID_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     hidReport.direction = HID_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: hidReport.direction = HID_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      hidReport.direction = HID_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  hidReport.direction = HID_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      hidReport.direction = HID_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    hidReport.direction = HID_HAT_UPLEFT;    break;
		default:                                     hidReport.direction = HID_HAT_NOTHING;   break;
	}

	hidReport.l_x_axis = static_cast<uint8_t>(gamepad->state.lx >> 8);
	hidReport.l_y_axis = static_cast<uint8_t>(gamepad->state.ly >> 8);
	hidReport.r_x_axis = static_cast<uint8_t>(gamepad->state.rx >> 8);
	hidReport.r_y_axis = static_cast<uint8_t>(gamepad->state.ry >> 8);

	// these first three buttons are in this unintuitive order to be compatible with
	// expectations, e.g. both PS3/4/5 modes and Switch modes map to HID as
	// B3 B4  ==  1 4
	// B1 B2  ==  2 3
	hidReport.buttons = 0
		| (gamepad->pressedB1()    ? GAMEPAD_MASK_B2     : 0)
		| (gamepad->pressedB2()    ? GAMEPAD_MASK_B3     : 0)
		| (gamepad->pressedB3()    ? GAMEPAD_MASK_B1     : 0)
		| (gamepad->pressedB4()    ? GAMEPAD_MASK_B4     : 0)
		| (gamepad->pressedL1()    ? GAMEPAD_MASK_L1     : 0)
		| (gamepad->pressedR1()    ? GAMEPAD_MASK_R1     : 0)
		| (gamepad->pressedL2()    ? GAMEPAD_MASK_L2     : 0)
		| (gamepad->pressedR2()    ? GAMEPAD_MASK_R2     : 0)
		| (gamepad->pressedS1()    ? GAMEPAD_MASK_S1     : 0)
		| (gamepad->pressedS2()    ? GAMEPAD_MASK_S2     : 0)
		| (gamepad->pressedL3()    ? GAMEPAD_MASK_L3     : 0)
		| (gamepad->pressedR3()    ? GAMEPAD_MASK_R3     : 0)
		| (gamepad->pressedA1()    ? GAMEPAD_MASK_A1     : 0)
		| (gamepad->pressedA2()    ? GAMEPAD_MASK_A2     : 0)
		| (gamepad->pressedA3()    ? GAMEPAD_MASK_A3     : 0)
		| (gamepad->pressedA4()    ? GAMEPAD_MASK_A4     : 0)
		| (gamepad->pressedUp()    ? GAMEPAD_MASK_DU     : 0)
		| (gamepad->pressedDown()  ? GAMEPAD_MASK_DD     : 0)
		| (gamepad->pressedLeft()  ? GAMEPAD_MASK_DL     : 0)
		| (gamepad->pressedRight() ? GAMEPAD_MASK_DR     : 0)
		| (gamepad->pressedE1()    ? GAMEPAD_MASK_E1     : 0)
		| (gamepad->pressedE2()    ? GAMEPAD_MASK_E2     : 0)
		| (gamepad->pressedE3()    ? GAMEPAD_MASK_E3     : 0)
		| (gamepad->pressedE4()    ? GAMEPAD_MASK_E4     : 0)
		| (gamepad->pressedE5()    ? GAMEPAD_MASK_E5     : 0)
		| (gamepad->pressedE6()    ? GAMEPAD_MASK_E6     : 0)
		| (gamepad->pressedE7()    ? GAMEPAD_MASK_E7     : 0)
		| (gamepad->pressedE8()    ? GAMEPAD_MASK_E8     : 0)
		| (gamepad->pressedE9()    ? GAMEPAD_MASK_E9     : 0)
		| (gamepad->pressedE10()   ? GAMEPAD_MASK_E10    : 0)
		| (gamepad->pressedE11()   ? GAMEPAD_MASK_E11    : 0)
		| (gamepad->pressedE12()   ? GAMEPAD_MASK_E12    : 0)
	;

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &hidReport;
	uint16_t report_size = sizeof(hidReport);
	if (memcmp(last_report, report, report_size) != 0)
	{
		// HID ready + report sent, copy previous report
		if (tud_hid_ready() && tud_hid_report(0, report, report_size) == true ) {
			memcpy(last_report, report, report_size);
			return true;
		}
	}
	
	return false;
}

// tud_hid_get_report_cb
uint16_t HIDDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	memcpy(buffer, &hidReport, sizeof(HIDReport));
	return sizeof(HIDReport);
}

// Only PS4 does anything with set report
void HIDDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool HIDDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
	return false;
}

const uint16_t * HIDDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
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
                value = (char *)hid_string_descriptors[index];
                break;
        }
    } else {
        value = (char *)hid_string_descriptors[index];
    }

	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * HIDDriver::get_descriptor_device_cb() {
    // Check for override settings
    GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
    if ( gamepadOptions.usbOverrideID == true ) {
        static uint8_t modified_device_descriptor[18];
        memcpy(modified_device_descriptor, hid_device_descriptor, sizeof(hid_device_descriptor));
        memcpy(&modified_device_descriptor[8], (uint8_t*)&gamepadOptions.usbVendorID, sizeof(uint16_t)); // Vendor ID
        memcpy(&modified_device_descriptor[10], (uint8_t*)&gamepadOptions.usbProductID, sizeof(uint16_t)); // Product ID
        return (const uint8_t*)modified_device_descriptor;
    }

	return hid_device_descriptor;
}

const uint8_t * HIDDriver::get_hid_descriptor_report_cb(uint8_t itf) {
	return hid_report_descriptor;
}

const uint8_t * HIDDriver::get_descriptor_configuration_cb(uint8_t index) {
	return hid_configuration_descriptor;
}

const uint8_t * HIDDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t HIDDriver::GetJoystickMidValue() {
	return HID_JOYSTICK_MID << 8;
}
