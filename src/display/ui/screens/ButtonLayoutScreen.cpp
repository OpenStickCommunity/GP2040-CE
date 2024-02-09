#include "ButtonLayoutScreen.h"

void ButtonLayoutScreen::drawScreen() {
    ButtonLayoutCustomOptions buttonLayoutCustomOptions = getDisplayOptions().buttonLayoutCustomOptions;

    switch (getDisplayOptions().buttonLayout) {
        case BUTTON_LAYOUT_STICK:
            drawArcadeStick(8, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_STICKLESS:
            drawStickless(8, 20, 8, 2);
            break;
        case BUTTON_LAYOUT_BUTTONS_ANGLED:
            drawWasdBox(8, (isInputHistoryEnabled ? 22 : 28), 7, 3);
            break;
        case BUTTON_LAYOUT_BUTTONS_BASIC:
            drawUDLR(8, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_KEYBOARD_ANGLED:
            drawKeyboardAngled(18, (isInputHistoryEnabled ? 24 : 28), 5, 2);
            break;
        case BUTTON_LAYOUT_KEYBOARDA:
            drawMAMEA(8, (isInputHistoryEnabled ? 22 : 28), 10, 1);
            break;
        case BUTTON_LAYOUT_OPENCORE0WASDA:
            drawOpenCore0WASDA(16, (isInputHistoryEnabled ? 22 : 28), 10, 1);
            break;
        case BUTTON_LAYOUT_DANCEPADA:
            drawDancepadA(39, (isInputHistoryEnabled ? 10 : 12), (isInputHistoryEnabled ? 13 : 15), 2);
            break;
        case BUTTON_LAYOUT_TWINSTICKA:
            drawTwinStickA(8, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_BLANKA:
            drawBlankA(0, 0, 0, 0);
            break;
        case BUTTON_LAYOUT_VLXA:
            //drawVLXA(7, (isInputHistoryEnabled ? 22 : 28), 7, 2);
            //drawVLXA(14, 36, 10, 2);
            break;
        case BUTTON_LAYOUT_CUSTOMA:
            drawButtonLayoutLeft(buttonLayoutCustomOptions.paramsLeft);
            break;
        case BUTTON_LAYOUT_FIGHTBOARD_STICK:
            drawArcadeStick(18, 22, 8, 2);
            break;
        case BUTTON_LAYOUT_FIGHTBOARD_MIRRORED:
            drawFightboardMirrored(0, 22, 7, 2);
            break;
    }

    switch (getDisplayOptions().buttonLayoutRight) {
        case BUTTON_LAYOUT_ARCADE:
            drawArcadeButtons(8, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_STICKLESSB:
            drawSticklessButtons(8, 20, 8, 2);
            break;
        case BUTTON_LAYOUT_BUTTONS_ANGLEDB:
            drawWasdButtons(8, (isInputHistoryEnabled ? 22 : 28), 7, 3);
            break;
        case BUTTON_LAYOUT_VEWLIX:
            drawVewlix(8, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_VEWLIX7:
            drawVewlix7(8, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_CAPCOM:
            drawCapcom(6, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_CAPCOM6:
            drawCapcom6(16, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_SEGA2P:
            drawSega2p(8, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_NOIR8:
            drawNoir8(8, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_KEYBOARDB:
            drawMAMEB(68, (isInputHistoryEnabled ? 22 : 28), 10, 1);
            break;
        case BUTTON_LAYOUT_KEYBOARD8B:
            drawMAME8B(68, (isInputHistoryEnabled ? 22 : 28), 10, 1);
            break;
        case BUTTON_LAYOUT_OPENCORE0WASDB:
            drawOpenCore0WASDB(68, (isInputHistoryEnabled ? 22 : 28), 10, 1);
            break;
        case BUTTON_LAYOUT_DANCEPADB:
            drawDancepadB(39, (isInputHistoryEnabled ? 10 : 12), (isInputHistoryEnabled ? 13 : 15), 2);
            break;
        case BUTTON_LAYOUT_TWINSTICKB:
            drawTwinStickB(100, (isInputHistoryEnabled ? 22 : 28), 8, 2);
            break;
        case BUTTON_LAYOUT_BLANKB:
            drawSticklessButtons(0, 0, 0, 0);
            break;
        case BUTTON_LAYOUT_VLXB:
            //drawVLXB(6, (isInputHistoryEnabled ? 22 : 28), 7, 2);
            break;
        case BUTTON_LAYOUT_CUSTOMB:
            drawButtonLayoutRight(buttonLayoutCustomOptions.paramsRight);
            break;
        case BUTTON_LAYOUT_FIGHTBOARD:
            drawFightboard(8, 22, 7, 3);
            break;
        case BUTTON_LAYOUT_FIGHTBOARD_STICK_MIRRORED:
            drawArcadeStick(90, 22, 8, 2);
            break;
    }

    getRenderer()->drawText(0, 0, header);
    getRenderer()->drawText(0, 57, footer);
}

void ButtonLayoutScreen::drawButtonLayoutLeft(ButtonLayoutParamsLeft& options)
{
	int32_t& startX    = options.common.startX;
	int32_t& startY    = options.common.startY;
	int32_t& buttonRadius  = options.common.buttonRadius;
	int32_t& buttonPadding = options.common.buttonPadding;

	switch (options.layout)
		{
			case BUTTON_LAYOUT_STICK:
				drawArcadeStick(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_STICKLESS:
				drawStickless(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_BUTTONS_ANGLED:
				drawWasdBox(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_BUTTONS_BASIC:
				drawUDLR(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_KEYBOARD_ANGLED:
				drawKeyboardAngled(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_KEYBOARDA:
				drawMAMEA(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_OPENCORE0WASDA:
				drawOpenCore0WASDA(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_DANCEPADA:
				drawDancepadA(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_TWINSTICKA:
				drawTwinStickA(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_BLANKA:
				drawBlankA(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_VLXA:
				drawVLXA(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_FIGHTBOARD_STICK:
				drawArcadeStick(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_FIGHTBOARD_MIRRORED:
				drawFightboardMirrored(startX, startY, buttonRadius, buttonPadding);
				break;
		}
}

void ButtonLayoutScreen::drawButtonLayoutRight(ButtonLayoutParamsRight& options)
{
	int32_t& startX        = options.common.startX;
	int32_t& startY        = options.common.startY;
	int32_t& buttonRadius  = options.common.buttonRadius;
	int32_t& buttonPadding = options.common.buttonPadding;

	switch (options.layout)
		{
			case BUTTON_LAYOUT_ARCADE:
				drawArcadeButtons(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_STICKLESSB:
				drawSticklessButtons(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_BUTTONS_ANGLEDB:
				drawWasdButtons(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_VEWLIX:
				drawVewlix(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_VEWLIX7:
				drawVewlix7(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_CAPCOM:
				drawCapcom(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_CAPCOM6:
				drawCapcom6(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_SEGA2P:
				drawSega2p(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_NOIR8:
				drawNoir8(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_KEYBOARDB:
				drawMAMEB(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_KEYBOARD8B:
				drawMAME8B(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_OPENCORE0WASDB:
				drawOpenCore0WASDB(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_DANCEPADB:
				drawDancepadB(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_TWINSTICKB:
				drawTwinStickB(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_BLANKB:
				drawSticklessButtons(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_VLXB:
				drawVLXB(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_FIGHTBOARD:
				drawFightboard(startX, startY, buttonRadius, buttonPadding);
				break;
			case BUTTON_LAYOUT_FIGHTBOARD_STICK_MIRRORED:
				drawArcadeStick(startX, startY, buttonRadius, buttonPadding);
				break;
		}
}

void ButtonLayoutScreen::drawDiamond(int cx, int cy, int size, uint8_t colour, uint8_t filled)
{
	if (filled) {
		int i;
		for (i = 0; i < size; i++) {
			getRenderer()->drawLine(cx - i, cy - size + i, cx + i, cy - size + i, colour, 0);
			getRenderer()->drawLine(cx - i, cy + size - i, cx + i, cy + size - i, colour, 0);
		}
		getRenderer()->drawLine(cx - size, cy, cx + size, cy, colour, 0); // Fill in the middle
	}
	getRenderer()->drawLine(cx - size, cy, cx, cy - size, colour, 0);
	getRenderer()->drawLine(cx, cy - size, cx + size, cy, colour, 0);
	getRenderer()->drawLine(cx + size, cy, cx, cy + size, colour, 0);
	getRenderer()->drawLine(cx, cy + size, cx - size, cy, colour, 0);
}

void ButtonLayoutScreen::drawStickless(int startX, int startY, int buttonRadius, int buttonPadding)
{

	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	getRenderer()->drawEllipse(startX, startY, buttonRadius, buttonRadius, 1, pressedLeft());
	getRenderer()->drawEllipse(startX + buttonMargin, startY, buttonRadius, buttonRadius, 1, pressedDown());
	getRenderer()->drawEllipse(startX + (buttonMargin * 1.875), startY + (buttonMargin / 2), buttonRadius, buttonRadius, 1, pressedRight());
	getRenderer()->drawEllipse(startX + (buttonMargin * 2.25), startY + buttonMargin * 1.875, buttonRadius, buttonRadius, 1, pressedUp());
}

void ButtonLayoutScreen::drawWasdBox(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// WASD
	getRenderer()->drawEllipse(startX, startY + buttonMargin * 0.5, buttonRadius, buttonRadius, 1, pressedLeft());
	getRenderer()->drawEllipse(startX + buttonMargin, startY + buttonMargin * 0.875, buttonRadius, buttonRadius, 1, pressedDown());
	getRenderer()->drawEllipse(startX + buttonMargin * 1.5, startY - buttonMargin * 0.125, buttonRadius, buttonRadius, 1, pressedUp());
	getRenderer()->drawEllipse(startX + (buttonMargin * 2), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pressedRight());
}

void ButtonLayoutScreen::drawUDLR(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// UDLR
	getRenderer()->drawEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, pressedLeft());
	getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedUp());
	getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pressedDown());
	getRenderer()->drawEllipse(startX + (buttonMargin * 1.625), startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, pressedRight());
}

void ButtonLayoutScreen::drawArcadeStick(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// Stick
	getRenderer()->drawEllipse(startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

	if (pressedUp()) {
		if (pressedLeft()) {
			getRenderer()->drawEllipse(startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else {
			getRenderer()->drawEllipse(startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedDown()) {
		if (pressedLeft()) {
			getRenderer()->drawEllipse(startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else {
			getRenderer()->drawEllipse(startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedLeft()) {
		getRenderer()->drawEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else if (pressedRight()) {
		getRenderer()->drawEllipse(startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else {
		getRenderer()->drawEllipse(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	}
}

void ButtonLayoutScreen::drawVLXA(int startX, int startY, int buttonRadius, int buttonPadding)
{
    drawLever(startX, startY, buttonRadius, 1, 0, 0);

	//const int buttonMargin = buttonPadding + (buttonRadius * 2);
    //
	// Stick
	//getRenderer()->drawEllipse(startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);
    //
	//if (pressedUp()) {
	//	if (pressedLeft()) {
	//		getRenderer()->drawEllipse(startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
	//	} else if (pressedRight()) {
	//		getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
	//	} else {
	//		getRenderer()->drawEllipse(startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
	//	}
	//} else if (pressedDown()) {
	//	if (pressedLeft()) {
	//		getRenderer()->drawEllipse(startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
	//	} else if (pressedRight()) {
	//		getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
	//	} else {
	//		getRenderer()->drawEllipse(startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
	//	}
	//} else if (pressedLeft()) {
	//	getRenderer()->drawEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	//} else if (pressedRight()) {
	//	getRenderer()->drawEllipse(startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	//} else {
	//	getRenderer()->drawEllipse(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	//}
}

void ButtonLayoutScreen::drawFightboardMirrored(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);
    const int leftMargin = startX + buttonPadding + buttonRadius;

	getRenderer()->drawEllipse(leftMargin, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedL1());
	getRenderer()->drawEllipse(leftMargin + buttonMargin, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse(leftMargin + (buttonMargin*2), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(leftMargin + (buttonMargin*3), startY * 1.25, buttonRadius, buttonRadius, 1, pressedB3());

	getRenderer()->drawEllipse(leftMargin, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedL2());
	getRenderer()->drawEllipse(leftMargin + buttonMargin, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	getRenderer()->drawEllipse(leftMargin + (buttonMargin*2), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(leftMargin + (buttonMargin*3), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pressedB1());

    // Extra buttons
    getRenderer()->drawEllipse(startX + buttonMargin * 0.5, startY + (buttonMargin * 1.5), 3, 3, 1, pressedL3());
    getRenderer()->drawEllipse(startX + buttonMargin * 1.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pressedS1());
    getRenderer()->drawEllipse(startX + buttonMargin * 1.625, startY + (buttonMargin * 1.5), 3, 3, 1, pressedA1());
    getRenderer()->drawEllipse(startX + buttonMargin * 2.125+0.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pressedS2());
    getRenderer()->drawEllipse(startX + buttonMargin * 2.75, startY + (buttonMargin * 1.5), 3, 3, 1, pressedR3());
}

void ButtonLayoutScreen::drawTwinStickA(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// Stick
	getRenderer()->drawEllipse(startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

	if (pressedUp()) {
		if (pressedLeft()) {
			getRenderer()->drawEllipse(startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else {
			getRenderer()->drawEllipse(startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedDown()) {
		if (pressedLeft()) {
			getRenderer()->drawEllipse(startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else {
			getRenderer()->drawEllipse(startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedLeft()) {
		getRenderer()->drawEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else if (pressedRight()) {
		getRenderer()->drawEllipse(startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else {
		getRenderer()->drawEllipse(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	}
}

void ButtonLayoutScreen::drawTwinStickB(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// Stick
	getRenderer()->drawEllipse(startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

	if (pressedB4()) {
		if (pressedB3()) {
			getRenderer()->drawEllipse(startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedB2()) {
			getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else {
			getRenderer()->drawEllipse(startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedB1()) {
		if (pressedB3()) {
			getRenderer()->drawEllipse(startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedB2()) {
			getRenderer()->drawEllipse(startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else {
			getRenderer()->drawEllipse(startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedB3()) {
		getRenderer()->drawEllipse(startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else if (pressedB2()) {
		getRenderer()->drawEllipse(startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else {
		getRenderer()->drawEllipse(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	}
}

void ButtonLayoutScreen::drawMAMEA(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// MAME
	getRenderer()->drawRectangle(startX, startY + buttonMargin, startX + buttonSize, startY + buttonSize + buttonMargin, 1, pressedLeft());
	getRenderer()->drawRectangle(startX + buttonMargin, startY + buttonMargin, startX + buttonSize + buttonMargin, startY + buttonSize + buttonMargin, 1, pressedDown());
	getRenderer()->drawRectangle(startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pressedUp());
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin, 1, pressedRight());

}

void ButtonLayoutScreen::drawOpenCore0WASDA(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// Open_Core0 WASD
	getRenderer()->drawRectangle(startX, startY + buttonMargin, startX + buttonSize, startY + buttonSize + buttonMargin, 1, pressedLeft());
	getRenderer()->drawRectangle(startX + buttonMargin, startY + buttonMargin, startX + buttonSize + buttonMargin, startY + buttonSize + buttonMargin, 1, pressedDown());
	getRenderer()->drawRectangle(startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pressedUp());
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin, 1, pressedRight());

	// Aux buttons
    getRenderer()->drawEllipse(startX - 15 + buttonMargin * 0.5, startY - 25 + (buttonMargin * 1.5), 3, 3, 1, pressedS2());
    getRenderer()->drawEllipse(startX - 15 + buttonMargin * 1.25, startY - 25 + (buttonMargin * 1.5), 3, 3, 1, pressedS1());
    getRenderer()->drawEllipse(startX - 15 + buttonMargin * 2, startY -25 + (buttonMargin * 1.5), 3, 3, 1, pressedA1());
    getRenderer()->drawEllipse(startX - 15 + buttonMargin * 2.75, startY -25 + (buttonMargin * 1.5), 3, 3, 1, pressedA2());
    getRenderer()->drawEllipse(startX - 15 + buttonMargin * 3.5, startY -25 + (buttonMargin * 1.5), 3, 3, 1, pressedL3());
	getRenderer()->drawEllipse(startX - 15 + buttonMargin * 4.25, startY -25 + (buttonMargin * 1.5), 3, 3, 1, pressedR3());

}

void ButtonLayoutScreen::drawMAMEB(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// 6-button MAME Style
	getRenderer()->drawRectangle(startX, startY, startX + buttonSize, startY + buttonSize, 1, pressedB3());
	getRenderer()->drawRectangle(startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pressedB4());
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY, startX + buttonSize + buttonMargin * 2, startY + buttonSize, 1, pressedR1());

	getRenderer()->drawRectangle(startX, startY + buttonMargin, startX + buttonSize, startY + buttonMargin + buttonSize, 1, pressedB1());
	getRenderer()->drawRectangle(startX + buttonMargin, startY + buttonMargin, startX + buttonSize + buttonMargin, startY + buttonMargin + buttonSize, 1, pressedB2());
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonMargin + buttonSize, 1, pressedR2());

}

void ButtonLayoutScreen::drawMAME8B(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// 8-button MAME Style
	getRenderer()->drawRectangle(startX, startY, startX + buttonSize, startY + buttonSize, 1, pressedB3());
	getRenderer()->drawRectangle(startX + buttonMargin, startY - (buttonMargin / 3), startX + buttonSize + buttonMargin, startY - (buttonMargin / 3) + buttonSize, 1, pressedB4());
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY - (buttonMargin / 3), startX + buttonSize + buttonMargin * 2, startY - (buttonMargin / 3)+ buttonSize, 1, pressedR1());
	getRenderer()->drawRectangle(startX + buttonMargin * 3, startY, startX + buttonSize + buttonMargin * 3, startY + buttonSize, 1, pressedL1());

	getRenderer()->drawRectangle(startX, startY + buttonMargin, startX + buttonSize, startY + buttonMargin + buttonSize, 1, pressedB1());
	getRenderer()->drawRectangle(startX + buttonMargin, startY - (buttonMargin / 3) + buttonMargin, startX + buttonSize + buttonMargin, startY - (buttonMargin / 3) + buttonMargin + buttonSize, 1, pressedB2());
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY - (buttonMargin / 3) + buttonMargin, startX + buttonSize + buttonMargin * 2, startY - (buttonMargin / 3) + buttonMargin + buttonSize, 1, pressedR2());
	getRenderer()->drawRectangle(startX + buttonMargin * 3, startY + buttonMargin, startX + buttonSize + buttonMargin * 3, startY + buttonMargin + buttonSize, 1, pressedL2());

}

void ButtonLayoutScreen::drawOpenCore0WASDB(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// 8-button Open_Core0 WASD
	getRenderer()->drawRectangle(startX, startY, startX + buttonSize, startY + buttonSize, 1, pressedB3());
	getRenderer()->drawRectangle(startX + buttonMargin, startY - (buttonMargin / 3), startX + buttonSize + buttonMargin, startY - (buttonMargin / 3) + buttonSize, 1, pressedB4());
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY - (buttonMargin / 3), startX + buttonSize + buttonMargin * 2, startY - (buttonMargin / 3)+ buttonSize, 1, pressedR1());
	getRenderer()->drawRectangle(startX + buttonMargin * 3, startY, startX + buttonSize + buttonMargin * 3, startY + buttonSize, 1, pressedL1());

	getRenderer()->drawRectangle(startX, startY + buttonMargin, startX + buttonSize, startY + buttonMargin + buttonSize, 1, pressedB1());
	getRenderer()->drawRectangle(startX + buttonMargin, startY - (buttonMargin / 3) + buttonMargin, startX + buttonSize + buttonMargin, startY - (buttonMargin / 3) + buttonMargin + buttonSize, 1, pressedB2());
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY - (buttonMargin / 3) + buttonMargin, startX + buttonSize + buttonMargin * 2, startY - (buttonMargin / 3) + buttonMargin + buttonSize, 1, pressedR2());
	getRenderer()->drawRectangle(startX + buttonMargin * 3, startY + buttonMargin, startX + buttonSize + buttonMargin * 3, startY + buttonMargin + buttonSize, 1, pressedL2());

}

void ButtonLayoutScreen::drawKeyboardAngled(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// MixBox
	drawDiamond(startX, startY, buttonRadius, 1, pressedLeft());
	drawDiamond(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, 1, pressedDown());
	drawDiamond(startX + buttonMargin, startY, buttonRadius, 1, pressedUp());
	drawDiamond(startX + buttonMargin, startY + buttonMargin, buttonRadius, 1, pressedRight());
}

void ButtonLayoutScreen::drawVewlix(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button Vewlix
	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedL1());

	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	getRenderer()->drawEllipse(startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedL2());
}

void ButtonLayoutScreen::drawVLXB(int startX, int startY, int buttonRadius, int buttonPadding)
{
    drawButton(startX-2, startY-2, buttonRadius, 1, 0, GAMEPAD_MASK_B3);
    drawButton(startX+16, startY-7, buttonRadius, 1, 0, GAMEPAD_MASK_B4);
    drawButton(startX+32, startY-7, buttonRadius, 1, 0, GAMEPAD_MASK_R1);
    drawButton(startX+48, startY-7, buttonRadius, 1, 0, GAMEPAD_MASK_L1);

    drawButton(startX-5, startY+16, buttonRadius, 1, 0, GAMEPAD_MASK_B1);
    drawButton(startX+11, startY+9, buttonRadius, 1, 0, GAMEPAD_MASK_B2);
    drawButton(startX+27, startY+9, buttonRadius, 1, 0, GAMEPAD_MASK_R2);
    drawButton(startX+43, startY+9, buttonRadius, 1, 0, GAMEPAD_MASK_L2);
    
    drawButton(startX+69, startY+2, 6, 1, 0, GAMEPAD_MASK_S2);

    //X: 50, Y: 31, R: 7
    //X: 66, Y: 24, R: 7
    //X: 82, Y: 24, R: 7
    //X: 98, Y: 24, R: 7

    //X: 45, Y: 47, R: 7
    //X: 61, Y: 40, R: 7
    //X: 77, Y: 40, R: 7
    //X: 93, Y: 40, R: 7

    //X: 119, Y: 33, R: 1

	//const int buttonMargin = buttonPadding + (buttonRadius * 2);
    //
	//// 9-button Hori VLX
	//getRenderer()->drawEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pressedB3());
	//getRenderer()->drawEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	//getRenderer()->drawEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	//getRenderer()->drawEllipse(startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedL1());
    //
	//getRenderer()->drawEllipse(startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pressedB1());
	//getRenderer()->drawEllipse(startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	//getRenderer()->drawEllipse(startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	//getRenderer()->drawEllipse(startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedL2());
    //
	//getRenderer()->drawEllipse(startX + (buttonMargin * 7.4) - (buttonMargin / 3.5), startY + buttonMargin - (buttonMargin / 1.5), buttonRadius *.8, buttonRadius * .8, 1, pressedS2());
}

void ButtonLayoutScreen::drawFightboard(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	getRenderer()->drawEllipse((startX + buttonMargin * 3.625), startY * 1.25, buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse((startX + buttonMargin * 4.625), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse((startX + buttonMargin * 5.625), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse((startX + buttonMargin * 6.625), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedL1());

	getRenderer()->drawEllipse((startX + buttonMargin * 3.625), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse((startX + buttonMargin * 4.625), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse((startX + buttonMargin * 5.625), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	getRenderer()->drawEllipse((startX + buttonMargin * 6.625), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedL2());

    // Extra buttons
	getRenderer()->drawEllipse(startX + buttonMargin * 4.5, startY + (buttonMargin * 1.5), 3, 3, 1, pressedL3());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pressedS1());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.625, startY + (buttonMargin * 1.5), 3, 3, 1, pressedA1());
	getRenderer()->drawEllipse(startX + buttonMargin * 6.125+0.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pressedS2());
	getRenderer()->drawEllipse(startX + buttonMargin * 6.75, startY + (buttonMargin * 1.5), 3, 3, 1, pressedR3());
}

void ButtonLayoutScreen::drawVewlix7(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button Vewlix
	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedL1());

	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	//getRenderer()->drawEllipse(startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, gamepad->pressedL2());
}

void ButtonLayoutScreen::drawSega2p(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button Sega2P
	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin / 3), buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 5.75), startY, buttonRadius, buttonRadius, 1, pressedL1());

	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75), startY + buttonMargin + (buttonMargin / 3), buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	getRenderer()->drawEllipse(startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedL2());
}

void ButtonLayoutScreen::drawNoir8(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button Noir8
	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75), startY + (buttonMargin / 3.5), buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 5.75), startY, buttonRadius, buttonRadius, 1, pressedL1());

	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75), startY + buttonMargin + (buttonMargin / 3.5), buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	getRenderer()->drawEllipse(startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedL2());
}

void ButtonLayoutScreen::drawCapcom(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button Capcom
	getRenderer()->drawEllipse(startX + buttonMargin * 3.25, startY, buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse(startX + buttonMargin * 4.25, startY, buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.25, startY, buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse(startX + buttonMargin * 6.25, startY, buttonRadius, buttonRadius, 1, pressedL1());

	getRenderer()->drawEllipse(startX + buttonMargin * 3.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse(startX + buttonMargin * 4.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedR2());
	getRenderer()->drawEllipse(startX + buttonMargin * 6.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedL2());
}

void ButtonLayoutScreen::drawCapcom6(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 6-button Capcom
	getRenderer()->drawEllipse(startX + buttonMargin * 3.25, startY, buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse(startX + buttonMargin * 4.25, startY, buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.25, startY, buttonRadius, buttonRadius, 1, pressedR1());

	getRenderer()->drawEllipse(startX + buttonMargin * 3.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse(startX + buttonMargin * 4.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedR2());
}

void ButtonLayoutScreen::drawSticklessButtons(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button
	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75), startY, buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 5.75), startY, buttonRadius, buttonRadius, 1, pressedL1());

	getRenderer()->drawEllipse(startX + (buttonMargin * 2.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse(startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	getRenderer()->drawEllipse(startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedL2());
}

void ButtonLayoutScreen::drawWasdButtons(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button
	getRenderer()->drawEllipse(startX + buttonMargin * 3.625, startY, buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse(startX + buttonMargin * 4.625, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.625, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse(startX + buttonMargin * 6.625, startY, buttonRadius, buttonRadius, 1, pressedL1());

	getRenderer()->drawEllipse(startX + buttonMargin * 3.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse(startX + buttonMargin * 4.25, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.25, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	getRenderer()->drawEllipse(startX + buttonMargin * 6.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedL2());
}

void ButtonLayoutScreen::drawArcadeButtons(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button
	getRenderer()->drawEllipse(startX + buttonMargin * 3.125, startY, buttonRadius, buttonRadius, 1, pressedB3());
	getRenderer()->drawEllipse(startX + buttonMargin * 4.125, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB4());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.125, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR1());
	getRenderer()->drawEllipse(startX + buttonMargin * 6.125, startY, buttonRadius, buttonRadius, 1, pressedL1());

	getRenderer()->drawEllipse(startX + buttonMargin * 2.875, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedB1());
	getRenderer()->drawEllipse(startX + buttonMargin * 3.875, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedB2());
	getRenderer()->drawEllipse(startX + buttonMargin * 4.875, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedR2());
	getRenderer()->drawEllipse(startX + buttonMargin * 5.875, startY + buttonMargin, buttonRadius, buttonRadius, 1, pressedL2());
}

// I pulled this out of my PR, brought it back because of recent talks re: SOCD and rhythm games
// Enjoy!

void ButtonLayoutScreen::drawDancepadA(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	getRenderer()->drawRectangle(startX, startY + buttonMargin, startX + buttonSize, startY + buttonSize + buttonMargin, 1, pressedLeft());
	getRenderer()->drawRectangle(startX + buttonMargin, startY + buttonMargin * 2, startX + buttonSize + buttonMargin, startY + buttonSize + buttonMargin * 2, 1, pressedDown());
	getRenderer()->drawRectangle(startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pressedUp());
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin, 1, pressedRight());
}

void ButtonLayoutScreen::drawDancepadB(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	getRenderer()->drawRectangle(startX, startY, startX + buttonSize, startY + buttonSize, 1, pressedB2()); // Up/Left
	getRenderer()->drawRectangle(startX, startY + buttonMargin * 2, startX + buttonSize, startY + buttonSize + buttonMargin * 2, 1, pressedB4()); // Down/Left
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY, startX + buttonSize + buttonMargin * 2, startY + buttonSize, 1, pressedB1()); // Up/Right
	getRenderer()->drawRectangle(startX + buttonMargin * 2, startY + buttonMargin * 2, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin * 2, 1, pressedB3()); // Down/Right
}

void ButtonLayoutScreen::drawBlankA(int startX, int startY, int buttonSize, int buttonPadding)
{
}

void ButtonLayoutScreen::drawBlankB(int startX, int startY, int buttonSize, int buttonPadding)
{
}

void ButtonLayoutScreen::drawLever(uint16_t startX, uint16_t startY, uint16_t radius, uint16_t strokeColor, uint16_t fillColor, uint16_t inputType) {
    GPLever* lever = new GPLever();
    lever->setRenderer(getRenderer());
    
    lever->setPosition(startX, startY);
    lever->setStrokeColor(strokeColor);
    lever->setRadius(radius);
    lever->setInputType(inputType);

    addElement(lever);
}

void ButtonLayoutScreen::drawButton(uint16_t startX, uint16_t startY, uint16_t radius, uint16_t strokeColor, uint16_t fillColor, int16_t inputMask) {
    GPButton* button = new GPButton();
    button->setRenderer(getRenderer());

    button->setPosition(startX, startY);
    button->setStrokeColor(strokeColor);
    button->setRadius(radius);
    button->setInputMask(inputMask);

    addElement(button);
}

int8_t ButtonLayoutScreen::update() {
    GPWidget::update();
    bool configMode = getConfigMode();

    if (!hasInitialized) {
        hasInitialized = true;
        drawVLXA(14, 36, 10, 2);

        drawVLXB(50, 31, 7, 2);
    }

    if (!configMode) {
        return DisplayMode::BUTTONS;
    } else {
        uint16_t buttonState = _gamepadState.buttons;

        if (prevButtonState && !buttonState) {
            if (prevButtonState == GAMEPAD_MASK_B1) {
                prevButtonState = 0;
                return -1;
            }
        }

        prevButtonState = buttonState;
  
        return DisplayMode::BUTTONS;
    }
}
