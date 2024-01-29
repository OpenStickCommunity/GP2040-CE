#include "drivers/xboxog/XboxOriginalDriver.h"

static XboxOriginalReport xboxOriginalReport
{
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



XboxOriginalReport *Gamepad::getXboxOriginalReport()
{
    // digital buttons
	xboxOriginalReport.dButtons = 0
		| (pressedUp()    ? XID_DUP    : 0)
		| (pressedDown()  ? XID_DDOWN  : 0)
		| (pressedLeft()  ? XID_DLEFT  : 0)
		| (pressedRight() ? XID_DRIGHT : 0)
		| (pressedS2()    ? XID_START  : 0)
		| (pressedS1()    ? XID_BACK   : 0)
		| (pressedL3()    ? XID_LS     : 0)
		| (pressedR3()    ? XID_RS     : 0)
	;

    // analog buttons - convert to digital
    xboxOriginalReport.A     = (pressedB1() ? 0xFF : 0);
    xboxOriginalReport.B     = (pressedB2() ? 0xFF : 0);
    xboxOriginalReport.X     = (pressedB3() ? 0xFF : 0);
    xboxOriginalReport.Y     = (pressedB4() ? 0xFF : 0);
    xboxOriginalReport.BLACK = (pressedL1() ? 0xFF : 0);
    xboxOriginalReport.WHITE = (pressedR1() ? 0xFF : 0);

    // analog triggers
	if (hasAnalogTriggers)
	{
		xboxOriginalReport.L = pressedL2() ? 0xFF : state.lt;
		xboxOriginalReport.R = pressedR2() ? 0xFF : state.rt;
	}
	else
	{
		xboxOriginalReport.L = pressedL2() ? 0xFF : 0;
		xboxOriginalReport.R = pressedR2() ? 0xFF : 0;
	}

    // analog sticks
	xboxOriginalReport.leftStickX = static_cast<int16_t>(state.lx) + INT16_MIN;
	xboxOriginalReport.leftStickY = static_cast<int16_t>(~state.ly) + INT16_MIN;
	xboxOriginalReport.rightStickX = static_cast<int16_t>(state.rx) + INT16_MIN;
	xboxOriginalReport.rightStickY = static_cast<int16_t>(~state.ry) + INT16_MIN;

    return &xboxOriginalReport;
}
