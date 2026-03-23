/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/sinput/SInputDriver.h"
#include "drivers/sinput/SInputDescriptors.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"

static bool hid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
	return hidd_control_xfer_cb(rhport, stage, request);
}

void SInputDriver::initialize() {
	sinputReport = {
		.report_id = 0x01,
		.plug_status = 0x00,
		.charge_percent = 100,
		.buttons = 0,
		.left_x = 0,
		.left_y = 0,
		.right_x = 0,
		.right_y = 0,
		.trigger_l = -32768,
		.trigger_r = -32768,
		.imu_timestamp_us = 0,
		.accel_x = 0,
		.accel_y = 0,
		.accel_z = 0,
		.gyro_x = 0,
		.gyro_y = 0,
		.gyro_z = 0,
		.touchpad_1_x = 0,
		.touchpad_1_y = 0,
		.touchpad_1_pressure = 0,
        .touchpad_2_x = 0,
		.touchpad_2_y = 0,
		.touchpad_2_pressure = 0,
		.reserved_bulk = {0},
	};

	class_driver = {
	#if CFG_TUSB_DEBUG >= 2
		.name = "SInput",
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
bool SInputDriver::process(Gamepad * gamepad) {
	sinputReport.report_id = 0x01;
	sinputReport.imu_timestamp_us = static_cast<uint32_t>(getMicro());
	sinputReport.left_x = static_cast<int16_t>(gamepad->state.lx - 0x8000);
	sinputReport.left_y = static_cast<int16_t>(gamepad->state.ly - 0x8000);
	sinputReport.right_x = static_cast<int16_t>(gamepad->state.rx - 0x8000);
	sinputReport.right_y = static_cast<int16_t>(gamepad->state.ry - 0x8000);

	sinputReport.buttons = 0
		| (gamepad->pressedB1()    ? SINPUT_MASK_SOUTH       : 0)
		| (gamepad->pressedB2()    ? SINPUT_MASK_EAST        : 0)
		| (gamepad->pressedB3()    ? SINPUT_MASK_WEST        : 0)
		| (gamepad->pressedB4()    ? SINPUT_MASK_NORTH       : 0)
		| (gamepad->pressedUp()    ? SINPUT_MASK_DPAD_UP     : 0)
		| (gamepad->pressedDown()  ? SINPUT_MASK_DPAD_DOWN   : 0)
		| (gamepad->pressedLeft()  ? SINPUT_MASK_DPAD_LEFT   : 0)
		| (gamepad->pressedRight() ? SINPUT_MASK_DPAD_RIGHT  : 0)
		| (gamepad->pressedL3()    ? SINPUT_MASK_STICK_LEFT  : 0)
		| (gamepad->pressedR3()    ? SINPUT_MASK_STICK_RIGHT : 0)
		| (gamepad->pressedL1()    ? SINPUT_MASK_L_BUMPER    : 0)
		| (gamepad->pressedR1()    ? SINPUT_MASK_R_BUMPER    : 0)
		| (gamepad->pressedL2()    ? SINPUT_MASK_L_TRIGGER   : 0)
		| (gamepad->pressedR2()    ? SINPUT_MASK_R_TRIGGER   : 0)
		| (gamepad->pressedE1()    ? SINPUT_MASK_L_PADDLE_1  : 0)
		| (gamepad->pressedE2()    ? SINPUT_MASK_R_PADDLE_1  : 0)
		| (gamepad->pressedS2()    ? SINPUT_MASK_PLUS        : 0)
		| (gamepad->pressedS1()    ? SINPUT_MASK_MINUS       : 0)
		| (gamepad->pressedA1()    ? SINPUT_MASK_HOME        : 0)
		| (gamepad->pressedA2()    ? SINPUT_MASK_CAPTURE     : 0)
		| (gamepad->pressedE3()    ? SINPUT_MASK_L_PADDLE_2  : 0)
		| (gamepad->pressedE4()    ? SINPUT_MASK_R_PADDLE_2  : 0)
		| (gamepad->pressedA3()    ? SINPUT_MASK_TOUCHPAD_1  : 0)
		| (gamepad->pressedA4()    ? SINPUT_MASK_TOUCHPAD_2  : 0)
		| (gamepad->pressedE5()    ? SINPUT_MASK_MISC_3      : 0)
		| (gamepad->pressedE6()    ? SINPUT_MASK_MISC_4      : 0)
		| (gamepad->pressedE7()    ? SINPUT_MASK_MISC_5      : 0)
		| (gamepad->pressedE8()    ? SINPUT_MASK_MISC_6      : 0)
		| (gamepad->pressedE9()    ? SINPUT_MASK_MISC_7      : 0)
		| (gamepad->pressedE10()   ? SINPUT_MASK_MISC_8      : 0)
		| (gamepad->pressedE11()   ? SINPUT_MASK_MISC_9      : 0)
		| (gamepad->pressedE12()   ? SINPUT_MASK_MISC_10     : 0)
	;
	if (gamepad->hasAnalogTriggers || gamepad->hasLeftAnalogStick) {
		if (gamepad->state.lt > 0)
			sinputReport.buttons |= SINPUT_MASK_L_TRIGGER;
	}
	if (gamepad->hasAnalogTriggers || gamepad->hasRightAnalogStick) {
		if (gamepad->state.rt > 0)
			sinputReport.buttons |= SINPUT_MASK_R_TRIGGER;
	}

	if (gamepad->hasAnalogTriggers) {
		sinputReport.trigger_l = static_cast<int16_t>((static_cast<uint16_t>(gamepad->state.lt) << 8) - 32768);
		sinputReport.trigger_r = static_cast<int16_t>((static_cast<uint16_t>(gamepad->state.rt) << 8) - 32768);
	} else {
		sinputReport.trigger_l = gamepad->pressedL2() ? 32767 : -32768;
		sinputReport.trigger_r = gamepad->pressedR2() ? 32767 : -32768;
	}

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	void * report = &sinputReport;
	uint16_t report_size = sizeof(sinputReport);
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
uint16_t SInputDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
	memcpy(buffer, &sinputReport, sizeof(sinputReport));
	return sizeof(sinputReport);
}

// Only PS4 does anything with set report
void SInputDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool SInputDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
	return false;
}

const uint16_t * SInputDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
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
                value = (char *)sinput_string_descriptors[index];
                break;
        }
    } else {
        value = (char *)sinput_string_descriptors[index];
    }

	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * SInputDriver::get_descriptor_device_cb() {
    // Check for override settings
    GamepadOptions & gamepadOptions = Storage::getInstance().getGamepadOptions();
    if ( gamepadOptions.usbOverrideID == true ) {
        static uint8_t modified_device_descriptor[18];
        memcpy(modified_device_descriptor, sinput_device_descriptor, sizeof(sinput_device_descriptor));
        memcpy(&modified_device_descriptor[8], (uint8_t*)&gamepadOptions.usbVendorID, sizeof(uint16_t)); // Vendor ID
        memcpy(&modified_device_descriptor[10], (uint8_t*)&gamepadOptions.usbProductID, sizeof(uint16_t)); // Product ID
        return (const uint8_t*)modified_device_descriptor;
    }

	return sinput_device_descriptor;
}

const uint8_t * SInputDriver::get_hid_descriptor_report_cb(uint8_t itf) {
	return sinput_report_descriptor;
}

const uint8_t * SInputDriver::get_descriptor_configuration_cb(uint8_t index) {
	return sinput_configuration_descriptor;
}

const uint8_t * SInputDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t SInputDriver::GetJoystickMidValue() {
	return 0x8000;
}
