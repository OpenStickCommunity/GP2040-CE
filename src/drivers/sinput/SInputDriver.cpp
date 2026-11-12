/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2024 OpenStickCommunity (gp2040-ce.info)
 */

#include "drivers/sinput/SInputDriver.h"
#include "drivers/sinput/SInputDescriptors.h"
#include "drivers/shared/driverhelper.h"
#include "storagemanager.h"

#include <algorithm>

static bool hid_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request)
{
	return hidd_control_xfer_cb(rhport, stage, request);
}

static uint16_t * getSInputStringDescriptor(const char *value, uint8_t index)
{
	static uint16_t descriptorStringBuffer[128];
	if (index == 0) {
		descriptorStringBuffer[0] = (0x03 << 8) | 0x04;
		descriptorStringBuffer[1] = (((uint8_t)value[1]) << 8) | (uint8_t)value[0];
		return descriptorStringBuffer;
	}

	return getStringDescriptor(value, index);
}

static uint16_t getSInputLE16(const uint8_t *data)
{
	return static_cast<uint16_t>(data[0]) | (static_cast<uint16_t>(data[1]) << 8);
}

static uint8_t getSInputHapticIntensity(uint16_t amplitude)
{
	if (amplitude > UINT8_MAX)
		return static_cast<uint8_t>(amplitude >> 8);

	return static_cast<uint8_t>(amplitude);
}

static void setSInputRumble(uint8_t left, uint8_t right)
{
	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();

	if (gamepad->auxState.haptics.leftActuator.enabled) {
		gamepad->auxState.haptics.leftActuator.active = (left > 0);
		gamepad->auxState.haptics.leftActuator.intensity = left;
	}

	if (gamepad->auxState.haptics.rightActuator.enabled) {
		gamepad->auxState.haptics.rightActuator.active = (right > 0);
		gamepad->auxState.haptics.rightActuator.intensity = right;
	}
}

static void processSInputHaptics(uint8_t const *data, uint16_t len)
{
	if (len == 0)
		return;

	switch (data[0]) {
		case 1:
			if (len < 17)
				return;

			setSInputRumble(
				getSInputHapticIntensity(std::max(getSInputLE16(&data[3]), getSInputLE16(&data[7]))),
				getSInputHapticIntensity(std::max(getSInputLE16(&data[11]), getSInputLE16(&data[15])))
			);
			break;

		case 2:
			if (len < 5)
				return;

			setSInputRumble(data[1], data[3]);
			break;

		default:
			break;
	}
}

static void processSInputPlayerLED(uint8_t const *data, uint16_t len)
{
	if (len == 0)
		return;

	Gamepad * gamepad = Storage::getInstance().GetProcessedGamepad();
	gamepad->auxState.playerID.active = true;
	gamepad->auxState.playerID.value = data[0];
	gamepad->auxState.playerID.ledValue = data[0];
}

void SInputDriver::initialize() {
	sinputReport = {
		.report_id = SINPUT_INPUT_REPORT_ID,
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
		.accel_x = SINPUT_ACCEL_X_NEUTRAL,
		.accel_y = SINPUT_ACCEL_Y_NEUTRAL,
		.accel_z = SINPUT_ACCEL_Z_NEUTRAL,
		.gyro_x = SINPUT_GYRO_X_NEUTRAL,
		.gyro_y = SINPUT_GYRO_Y_NEUTRAL,
		.gyro_z = SINPUT_GYRO_Z_NEUTRAL,
		.touchpad_1_x = 0,
		.touchpad_1_y = 0,
		.touchpad_1_pressure = 0,
		.touchpad_2_x = 0,
		.touchpad_2_y = 0,
		.touchpad_2_pressure = 0,
		.reserved_bulk = {0},
	};
	isReportQueued = false;
	memset(queuedReport, 0x00, sizeof(queuedReport));

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
	sinputReport.report_id = SINPUT_INPUT_REPORT_ID;
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

	sinputReport.accel_x = SINPUT_ACCEL_X_NEUTRAL;
	sinputReport.accel_y = SINPUT_ACCEL_Y_NEUTRAL;
	sinputReport.accel_z = SINPUT_ACCEL_Z_NEUTRAL;
	sinputReport.gyro_x = SINPUT_GYRO_X_NEUTRAL;
	sinputReport.gyro_y = SINPUT_GYRO_Y_NEUTRAL;
	sinputReport.gyro_z = SINPUT_GYRO_Z_NEUTRAL;
	sinputReport.touchpad_1_x = 0;
	sinputReport.touchpad_1_y = 0;
	sinputReport.touchpad_1_pressure = 0;
	sinputReport.touchpad_2_x = 0;
	sinputReport.touchpad_2_y = 0;
	sinputReport.touchpad_2_pressure = 0;

	if (gamepad->auxState.sensors.accelerometer.enabled) {
		sinputReport.accel_x = static_cast<int16_t>(gamepad->auxState.sensors.accelerometer.x);
		sinputReport.accel_y = static_cast<int16_t>(gamepad->auxState.sensors.accelerometer.y);
		sinputReport.accel_z = static_cast<int16_t>(gamepad->auxState.sensors.accelerometer.z);
	}

	if (gamepad->auxState.sensors.gyroscope.enabled) {
		sinputReport.gyro_x = static_cast<int16_t>(gamepad->auxState.sensors.gyroscope.x);
		sinputReport.gyro_y = static_cast<int16_t>(gamepad->auxState.sensors.gyroscope.y);
		sinputReport.gyro_z = static_cast<int16_t>(gamepad->auxState.sensors.gyroscope.z);
	}

	if (gamepad->auxState.sensors.touchpad[0].enabled) {
		sinputReport.touchpad_1_x = static_cast<int16_t>(gamepad->auxState.sensors.touchpad[0].x);
		sinputReport.touchpad_1_y = static_cast<int16_t>(gamepad->auxState.sensors.touchpad[0].y);
		sinputReport.touchpad_1_pressure = gamepad->auxState.sensors.touchpad[0].active ?
			static_cast<int16_t>(gamepad->auxState.sensors.touchpad[0].z) : 0;
		if (gamepad->auxState.sensors.touchpad[0].active)
			sinputReport.buttons |= SINPUT_MASK_TOUCHPAD_1;
	}

	if (gamepad->auxState.sensors.touchpad[1].enabled) {
		sinputReport.touchpad_2_x = static_cast<int16_t>(gamepad->auxState.sensors.touchpad[1].x);
		sinputReport.touchpad_2_y = static_cast<int16_t>(gamepad->auxState.sensors.touchpad[1].y);
		sinputReport.touchpad_2_pressure = gamepad->auxState.sensors.touchpad[1].active ?
			static_cast<int16_t>(gamepad->auxState.sensors.touchpad[1].z) : 0;
		if (gamepad->auxState.sensors.touchpad[1].active)
			sinputReport.buttons |= SINPUT_MASK_TOUCHPAD_2;
	}

	// Wake up TinyUSB device
	if (tud_suspended())
		tud_remote_wakeup();

	if (isReportQueued) {
		if (tud_hid_ready() && tud_hid_report(0, queuedReport, sizeof(queuedReport)) == true) {
			isReportQueued = false;
			return true;
		}

		return false;
	}

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
	if (report_id == 0 || report_id == SINPUT_INPUT_REPORT_ID) {
		uint16_t report_size = sizeof(sinputReport);
		if (reqlen < report_size)
			report_size = reqlen;

		memcpy(buffer, &sinputReport, report_size);
		return report_size;
	}

	if (report_id == SINPUT_FEATURE_REPORT_ID) {
		uint16_t report_size = sizeof(sinput_features_report);
		if (reqlen < report_size)
			report_size = reqlen;

		memcpy(buffer, &sinput_features_report, report_size);
		return report_size;
	}

	return 0;
}

void SInputDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {
	if (report_type != HID_REPORT_TYPE_OUTPUT)
		return;

	if (bufsize == 0)
		return;

	uint8_t output_report_id = report_id;
	uint8_t command_offset = 0;
	if (buffer[0] == SINPUT_OUTPUT_REPORT_ID) {
		output_report_id = buffer[0];
		command_offset = 1;
	}

	if (output_report_id != SINPUT_OUTPUT_REPORT_ID || bufsize <= command_offset)
		return;

	uint8_t command = buffer[command_offset];
	uint8_t const *command_data = buffer + command_offset + 1;
	uint16_t command_data_len = bufsize - command_offset - 1;

	switch (command) {
		case SINPUT_OUTPUT_CMD_HAPTIC:
			processSInputHaptics(command_data, command_data_len);
			break;

		case SINPUT_OUTPUT_CMD_FEATURES:
			memcpy(queuedReport, &sinput_features_report, sizeof(queuedReport));
			isReportQueued = true;
			break;

		case SINPUT_OUTPUT_CMD_PLAYER_LED:
			processSInputPlayerLED(command_data, command_data_len);
			break;

		default:
			break;
	}
}

bool SInputDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
	return false;
}

const uint16_t * SInputDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value;
	if (index >= (sizeof(sinput_string_descriptors) / sizeof(sinput_string_descriptors[0])))
		return nullptr;

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
                break;
            default:
                value = (const char *)sinput_string_descriptors[index];
                break;
        }
    } else {
        value = (const char *)sinput_string_descriptors[index];
    }

	return getSInputStringDescriptor(value, index);
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
