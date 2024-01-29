#include "drivers/psclassic/PSClassicDriver.h"

// force a report to be sent every X ms
#define PS4_KEEPALIVE_TIMER 1000

// Xbox One & PS4/PS5 Timing
static TouchpadData touchpadData;

static PS4Report ps4Report
{
	.report_id = 0x01,
	.left_stick_x = PS4_JOYSTICK_MID,
	.left_stick_y = PS4_JOYSTICK_MID,
	.right_stick_x = PS4_JOYSTICK_MID,
	.right_stick_y = PS4_JOYSTICK_MID,
	.dpad = 0x08,
	.button_west = 0, .button_south = 0, .button_east = 0, .button_north = 0,
	.button_l1 = 0, .button_r1 = 0, .button_l2 = 0, .button_r2 = 0,
	.button_select = 0, .button_start = 0, .button_l3 = 0, .button_r3 = 0, .button_home = 0,
	.padding = 0,
	.mystery = { },
	.touchpad_data = TouchpadData(),
	.mystery_2 = { }
};

virtual void PS4Driver::initialize() {
}

virtual void PS4Driver::setup() {
	// setup PS5 compatibility
	//PS4Data::getInstance().ps4ControllerType = gamepadOptions.ps4ControllerType;
	//last_report_counter = 0;
	//last_axis_counter = 0;

	// Xbox One/PS4 Keep-Alive
	//keep_alive_timer = to_ms_since_boot(get_absolute_time());
	//keep_alive_sequence = 1;
	//virtual_keycode_sequence = 0;
	//xb1_guide_pressed = false;
}

virtual void PS4Driver::send_report(Gamepad * gamepad)
{
	switch (gamepad->state.dpad & GAMEPAD_MASK_DPAD)
	{
		case GAMEPAD_MASK_UP:                        ps4Report.dpad = PS4_HAT_UP;        break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_RIGHT:   ps4Report.dpad = PS4_HAT_UPRIGHT;   break;
		case GAMEPAD_MASK_RIGHT:                     ps4Report.dpad = PS4_HAT_RIGHT;     break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_RIGHT: ps4Report.dpad = PS4_HAT_DOWNRIGHT; break;
		case GAMEPAD_MASK_DOWN:                      ps4Report.dpad = PS4_HAT_DOWN;      break;
		case GAMEPAD_MASK_DOWN | GAMEPAD_MASK_LEFT:  ps4Report.dpad = PS4_HAT_DOWNLEFT;  break;
		case GAMEPAD_MASK_LEFT:                      ps4Report.dpad = PS4_HAT_LEFT;      break;
		case GAMEPAD_MASK_UP | GAMEPAD_MASK_LEFT:    ps4Report.dpad = PS4_HAT_UPLEFT;    break;
		default:                                     ps4Report.dpad = PS4_HAT_NOTHING;   break;
	}

	ps4Report.button_south    = gamepad->pressedB1();
	ps4Report.button_east     = gamepad->pressedB2();
	ps4Report.button_west     = gamepad->pressedB3();
	ps4Report.button_north    = gamepad->pressedB4();
	ps4Report.button_l1       = gamepad->pressedL1();
	ps4Report.button_r1       = gamepad->pressedR1();
	ps4Report.button_l2       = gamepad->pressedL2();
	ps4Report.button_r2       = gamepad->pressedR2();
	ps4Report.button_select   = options.switchTpShareForDs4 ? gamepad->pressedA2() : gamepad->pressedS1();
	ps4Report.button_start    = gamepad->pressedS2();
	ps4Report.button_l3       = gamepad->pressedL3();
	ps4Report.button_r3       = gamepad->pressedR3();
	ps4Report.button_home     = gamepad->pressedA1();
	ps4Report.button_touchpad = options.switchTpShareForDs4 ? gamepad->pressedS1() : gamepad->pressedA2();

	ps4Report.left_stick_x = static_cast<uint8_t>(gamepad->state.lx >> 8);
	ps4Report.left_stick_y = static_cast<uint8_t>(gamepad->state.ly >> 8);
	ps4Report.right_stick_x = static_cast<uint8_t>(gamepad->state.rx >> 8);
	ps4Report.right_stick_y = static_cast<uint8_t>(gamepad->state.ry >> 8);

	if (hasAnalogTriggers)
	{
		ps4Report.left_trigger = gamepad->state.lt;
		ps4Report.right_trigger = gamepad->state.rt;
	}
	else
	{
		ps4Report.left_trigger = gamepad->pressedL2() ? 0xFF : 0;
		ps4Report.right_trigger = gamepad->pressedR2() ? 0xFF : 0;
	}

	// set touchpad to nothing
	touchpadData.p1.unpressed = 1;
	touchpadData.p2.unpressed = 1;
	ps4Report.touchpad_data = touchpadData;

	// some games apparently can miss reports, or they rely on official behavior of getting frequent
	// updates. we normally only send a report when the value changes; if we increment the counters
	// every time we generate the report (every GP2040::run loop), we apparently overburden
	// TinyUSB and introduce roughly 1ms of latency. but we want to loop often and report on every
	// true update in order to achieve our tight <1ms report timing when we *do* have a different
	// report to send.
	// the "PS4 Hack" disables the counters so that we only report on changes, but this
	// means we never report the same data twice, and games that expected it would get stuck
	// inputs. the below code is a compromise: keep the tight report timing, but occasionally change
	// the report counter and axis timing values in order to force a changed report --- this should
	// eliminate the need for the PS4 Hack, but it's kept here at the moment for A/B testing purposes
	if ( !options.ps4ReportHack ) {
		uint32_t now = to_ms_since_boot(get_absolute_time());
		if ((now - keep_alive_timer) > PS4_KEEPALIVE_TIMER) {
			last_report_counter = (last_report_counter+1) & 0x3F;
			ps4Report.report_counter = last_report_counter;		// report counter is 6 bits
			ps4Report.axis_timing = now;		 		// axis counter is 16 bits
			keep_alive_timer = now;
		}
	}

	return &ps4Report;
}

