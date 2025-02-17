#include "drivers/xboxog/XboxOriginalDriver.h"
#include "drivers/xboxog/xid/xid.h"
#include "drivers/shared/driverhelper.h"

void XboxOriginalDriver::initialize() {
    xboxOriginalReport = {
        .dButtons = 0,
        .A = 0,
        .B = 0,
        .X = 0,
        .Y = 0,
        .BLACK = 0,
        .WHITE = 0,
        .L = 0,
        .R = 0,
        .leftStickX = 0,
        .leftStickY = 0,
        .rightStickX = 0,
        .rightStickY = 0,
    };

    // Copy XID driver to local class driver
    memcpy(&class_driver, xid_get_driver(), sizeof(usbd_class_driver_t));
}

bool XboxOriginalDriver::process(Gamepad * gamepad) {
	// digital buttons
	xboxOriginalReport.dButtons = 0
		| (gamepad->pressedUp()    ? XID_DUP    : 0)
		| (gamepad->pressedDown()  ? XID_DDOWN  : 0)
		| (gamepad->pressedLeft()  ? XID_DLEFT  : 0)
		| (gamepad->pressedRight() ? XID_DRIGHT : 0)
		| (gamepad->pressedS2()    ? XID_START  : 0)
		| (gamepad->pressedS1()    ? XID_BACK   : 0)
		| (gamepad->pressedL3()    ? XID_LS     : 0)
		| (gamepad->pressedR3()    ? XID_RS     : 0)
	;

    // analog buttons - convert to digital
    xboxOriginalReport.A     = (gamepad->pressedB1() ? 0xFF : 0);
    xboxOriginalReport.B     = (gamepad->pressedB2() ? 0xFF : 0);
    xboxOriginalReport.X     = (gamepad->pressedB3() ? 0xFF : 0);
    xboxOriginalReport.Y     = (gamepad->pressedB4() ? 0xFF : 0);
    xboxOriginalReport.BLACK = (gamepad->pressedR1() ? 0xFF : 0);
    xboxOriginalReport.WHITE = (gamepad->pressedL1() ? 0xFF : 0);

    // analog triggers
	if (gamepad->hasAnalogTriggers) {
		xboxOriginalReport.L = gamepad->pressedL2() ? 0xFF : gamepad->state.lt;
		xboxOriginalReport.R = gamepad->pressedR2() ? 0xFF : gamepad->state.rt;
	} else {
		xboxOriginalReport.L = gamepad->pressedL2() ? 0xFF : 0;
		xboxOriginalReport.R = gamepad->pressedR2() ? 0xFF : 0;
	}

    // analog sticks
	xboxOriginalReport.leftStickX = static_cast<int16_t>(gamepad->state.lx) + INT16_MIN;
	xboxOriginalReport.leftStickY = static_cast<int16_t>(~gamepad->state.ly) + INT16_MIN;
	xboxOriginalReport.rightStickX = static_cast<int16_t>(gamepad->state.rx) + INT16_MIN;
	xboxOriginalReport.rightStickY = static_cast<int16_t>(~gamepad->state.ry) + INT16_MIN;

	if (tud_suspended())
		tud_remote_wakeup();

    bool reportSent = false;
    uint8_t xIndex = xid_get_index_by_type(0, XID_TYPE_GAMECONTROLLER);
	if (memcmp(last_report, &xboxOriginalReport, sizeof(XboxOriginalReport)) != 0) {
        if ( xid_send_report(xIndex, &xboxOriginalReport, sizeof(XboxOriginalReport)) == true ) {
            memcpy(last_report, &xboxOriginalReport, sizeof(XboxOriginalReport));
            reportSent = true;
        }
    }

    if (xid_get_report(xIndex, &xboxOriginalReportOut, sizeof(xboxOriginalReportOut)))
    {
        uint8_t leftValue = (xboxOriginalReportOut.lValue >> 8);
        uint8_t rightValue = (xboxOriginalReportOut.rValue >> 8);
        
        if (gamepad->auxState.haptics.leftActuator.enabled) {
            gamepad->auxState.haptics.leftActuator.active = (leftValue > 0);
            gamepad->auxState.haptics.leftActuator.intensity = leftValue;
        }

        if (gamepad->auxState.haptics.rightActuator.enabled) {
            gamepad->auxState.haptics.rightActuator.active = (rightValue > 0);
            gamepad->auxState.haptics.rightActuator.intensity = rightValue;
        }
    }
    
    return reportSent;
}

// tud_hid_get_report_cb
uint16_t XboxOriginalDriver::get_report(uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    memcpy(buffer, &xboxOriginalReport, sizeof(XboxOriginalReport));
	return sizeof(XboxOriginalReport);
}

// Only PS4 does anything with set report
void XboxOriginalDriver::set_report(uint8_t report_id, hid_report_type_t report_type, uint8_t const *buffer, uint16_t bufsize) {}

// Only XboxOG and Xbox One use vendor control xfer cb
bool XboxOriginalDriver::vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request) {
    return class_driver.control_xfer_cb(rhport, stage, request);
}

const uint16_t * XboxOriginalDriver::get_descriptor_string_cb(uint8_t index, uint16_t langid) {
	const char *value = (const char *)xboxoriginal_string_descriptors[index];
	return getStringDescriptor(value, index); // getStringDescriptor returns a static array
}

const uint8_t * XboxOriginalDriver::get_descriptor_device_cb() {
    return xboxoriginal_device_descriptor;
}

const uint8_t * XboxOriginalDriver::get_hid_descriptor_report_cb(uint8_t itf) {
    return nullptr;
}

const uint8_t * XboxOriginalDriver::get_descriptor_configuration_cb(uint8_t index) {
    return xboxoriginal_configuration_descriptor;
}

const uint8_t * XboxOriginalDriver::get_descriptor_device_qualifier_cb() {
	return nullptr;
}

uint16_t XboxOriginalDriver::GetJoystickMidValue() {
	return GAMEPAD_JOYSTICK_MID;
}
