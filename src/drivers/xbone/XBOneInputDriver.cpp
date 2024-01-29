#include "drivers/xbone/XBOneDriver.h"


#define XBONE_KEEPALIVE_TIMER 15000

static XboxOneGamepad_Data_t xboneReport
{
	.sync = 0,
	.guide = 0,
	.start = 0,
	.back = 0,
	.a = 0,
	.b = 0,
	.x = 0,
	.y = 0,
	.dpadUp = 0,
	.dpadDown = 0,
	.dpadLeft = 0,
	.dpadRight = 0,
	.leftShoulder = 0,
	.rightShoulder = 0,
	.leftThumbClick = 0,
	.rightThumbClick = 0,
	.leftTrigger = 0,
	.rightTrigger = 0,
	.leftStickX = GAMEPAD_JOYSTICK_MID,
	.leftStickY = GAMEPAD_JOYSTICK_MID,
	.rightStickX = GAMEPAD_JOYSTICK_MID,
	.rightStickY = GAMEPAD_JOYSTICK_MID,
	.reserved = {}
};

static uint16_t xboneReportSize;

static uint8_t xb1_guide_on[] = { 0x01, 0x5b };
static uint8_t xb1_guide_off[] = { 0x00, 0x5b };
static uint8_t xboneIdle[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
					   0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0x00, 0x00,
					   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					   0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

/*
// On Send report Success
void Gamepad::sendReportSuccess() {
	switch( (options.inputMode) ) {
		case INPUT_MODE_XBONE:
			if ( xboneReport.Header.command == GIP_KEEPALIVE) {
				keep_alive_timer = to_ms_since_boot(get_absolute_time());
				keep_alive_sequence++; // will rollover
				if ( keep_alive_sequence == 0 )
					keep_alive_sequence = 1;
			} else if ( xboneReport.Header.command == GIP_INPUT_REPORT ) {
				// Successfully sent report, actually increment last report counter!
				if ( memcmp(&last_report[4], &((uint8_t*)&xboneReport)[4], xboneReportSize-4) != 0) {
					last_report_counter++;
					if (last_report_counter == 0)
						last_report_counter = 1;
					memcpy(last_report, &xboneReport, xboneReportSize);
				}
			}
			break;
		default:
			break;
	};
}
*/

XboxOneGamepad_Data_t *Gamepad::getXBOneReport()
{
	// No input until auth is ready
	if ( XboxOneData::getInstance().getAuthCompleted() == false ) {
		GIP_HEADER((&xboneReport), GIP_INPUT_REPORT, false, last_report_counter);
		memcpy((void*)&((uint8_t*)&xboneReport)[4], xboneIdle, sizeof(xboneIdle));
		xboneReportSize = sizeof(XboxOneGamepad_Data_t);
		return &xboneReport;
	}

	uint32_t now = to_ms_since_boot(get_absolute_time());
	// Send Keep-Alive every 15 seconds (keep_alive_timer updates if send is successful)
	if ( (now - keep_alive_timer) > XBONE_KEEPALIVE_TIMER) {
		memset(&xboneReport.Header, 0, sizeof(GipHeader_t));
		GIP_HEADER((&xboneReport), GIP_KEEPALIVE, 1, keep_alive_sequence);
		xboneReport.Header.length = 4;
		static uint8_t keepAlive[] = { 0x80, 0x00, 0x00, 0x00 };
		memcpy(&((uint8_t*)&xboneReport)[4], &keepAlive, sizeof(keepAlive));
		xboneReportSize = sizeof(GipHeader_t) + sizeof(keepAlive);
		return &xboneReport;
	}
	
	// Virtual Keycode for Guide Button
	if ( pressedA1() || xb1_guide_pressed == true ) {
		// In a change-state
		if ( (pressedA1() && xb1_guide_pressed == false) ||
			(!pressedA1() && xb1_guide_pressed == true)) {
			virtual_keycode_sequence++; // will rollover
			if ( virtual_keycode_sequence == 0 )
				virtual_keycode_sequence = 1;
			GIP_HEADER((&xboneReport), GIP_VIRTUAL_KEYCODE, 1, virtual_keycode_sequence);
			xboneReport.Header.length = sizeof(xb1_guide_on);
			if ( pressedA1() ) {
				xb1_guide_pressed = true;
				memcpy(&((uint8_t*)&xboneReport)[4], &xb1_guide_on, sizeof(xb1_guide_on));
			} else {
				xb1_guide_pressed = false;
				memcpy(&((uint8_t*)&xboneReport)[4], &xb1_guide_off, sizeof(xb1_guide_off));
			}
		}
		xboneReportSize = sizeof(GipHeader_t) + sizeof(xb1_guide_on);
		return &xboneReport;
	}

	// Only change xbox one input report if we have different inputs!
	XboxOneGamepad_Data_t newInputReport;

	// This is due to our tusb_driver.cpp checking memcmp(last_report, report, size)
	memset(&newInputReport, 0, sizeof(XboxOneGamepad_Data_t));
	GIP_HEADER((&newInputReport), GIP_INPUT_REPORT, false, last_report_counter);

	newInputReport.a = pressedB1();
	newInputReport.b = pressedB2();
	newInputReport.x = pressedB3();
	newInputReport.y = pressedB4();
	newInputReport.leftShoulder = pressedL1();
	newInputReport.rightShoulder = pressedR1();
	newInputReport.leftThumbClick = pressedL3();
	newInputReport.rightThumbClick = pressedR3();
	newInputReport.start = pressedS2();
	newInputReport.back = pressedS1();
	newInputReport.guide = 0; // always 0
	newInputReport.sync = 0; 
	newInputReport.dpadUp = pressedUp();
	newInputReport.dpadDown = pressedDown();
	newInputReport.dpadLeft = pressedLeft();
	newInputReport.dpadRight = pressedRight();

	newInputReport.leftStickX = static_cast<int16_t>(state.lx) + INT16_MIN;
	newInputReport.leftStickY = static_cast<int16_t>(~state.ly) + INT16_MIN;
	newInputReport.rightStickX = static_cast<int16_t>(state.rx) + INT16_MIN;
	newInputReport.rightStickY = static_cast<int16_t>(~state.ry) + INT16_MIN;

	if (hasAnalogTriggers)
	{
		newInputReport.leftTrigger = pressedL2() ? 0x03FF : state.lt;
		newInputReport.rightTrigger = pressedR2() ? 0x03FF : state.rt;
	}
	else
	{
		newInputReport.leftTrigger = pressedL2() ? 0x03FF : 0;
		newInputReport.rightTrigger = pressedR2() ? 0x03FF : 0;
	}

	// We changed inputs since generating our last report, increment last report counter (but don't update until success)
	if ( memcmp(&last_report[4], &((uint8_t*)&newInputReport)[4], sizeof(XboxOneGamepad_Data_t)-4) != 0 ) {
		xboneReportSize = sizeof(XboxOneGamepad_Data_t);
		memcpy(&xboneReport, &newInputReport, xboneReportSize);
		xboneReport.Header.sequence = last_report_counter + 1;
		if ( xboneReport.Header.sequence == 0 )
			xboneReport.Header.sequence = 1;
	}

	return &xboneReport;
}

