/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#include "addons/i2cdisplay.h"
#include "GamepadState.h"
#include "enums.h"
#include "storagemanager.h"
#include "pico/stdlib.h"
#include "bitmaps.h"
#include "ps4_driver.h"
#include "version.h"
#include "config.pb.h"
#include "usb_driver.h"

bool I2CDisplayAddon::available() {
	const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
	return options.enabled && PeripheralManager::getInstance().isI2CEnabled(options.i2cBlock);
}

void I2CDisplayAddon::setup() {
	const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
	PeripheralI2C* i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);

	obdI2CInit(&obd,
	    options.size,
		options.i2cAddress,
		options.flip,
		options.invert,
		DISPLAY_USEWIRE,
		i2c,
		-1);

	const int detectedDisplay = initDisplay(0);
	if (isSH1106(detectedDisplay)) {
		// The display is actually a SH1106 that was misdetected as a SSD1306 by OneBitDisplay.
		// Reinitialize as SH1106.
		initDisplay(OLED_132x64);
	}

	obdSetContrast(&obd, 0xFF);
	obdSetBackBuffer(&obd, ucBackBuffer);
	clearScreen(1);
	gamepad = Storage::getInstance().GetGamepad();
	pGamepad = Storage::getInstance().GetProcessedGamepad();
	prevButtonState = 0;
	displaySaverTimer = options.displaySaverTimeout;
	displaySaverTimeout = displaySaverTimer;
	configMode = Storage::getInstance().GetConfigMode();
	turnOffWhenSuspended = options.turnOffWhenSuspended;

	const InputHistoryOptions& inputHistoryOptions = Storage::getInstance().getAddonOptions().inputHistoryOptions;
	isInputHistoryEnabled = inputHistoryOptions.enabled;
}

bool I2CDisplayAddon::isDisplayPowerOff()
{
	if (turnOffWhenSuspended && get_usb_suspended()) {
		if (displayIsPowerOn) setDisplayPower(0);
		return true;
	} else {
		if (!displayIsPowerOn) setDisplayPower(1);
	}

	if (!displaySaverTimeout) return false;

	float diffTime = getMillis() - prevMillis;
	displaySaverTimer -= diffTime;
	if (!!displaySaverTimeout && (gamepad->state.buttons || gamepad->state.dpad) ) {
		displaySaverTimer = displaySaverTimeout;
		setDisplayPower(1);
	} else if (!!displaySaverTimeout && displaySaverTimer <= 0) {
		setDisplayPower(0);
	}

	prevMillis = getMillis();

	return (!!displaySaverTimeout && displaySaverTimer <= 0);
}

void I2CDisplayAddon::setDisplayPower(uint8_t status)
{
	if (displayIsPowerOn != status) {
		displayIsPowerOn = status;
		obdPower(&obd, status);
	}
}

void I2CDisplayAddon::process() {
	if (!configMode && isDisplayPowerOff()) return;

	clearScreen(0);

	switch (getDisplayMode()) {
		case I2CDisplayAddon::DisplayMode::CONFIG_INSTRUCTION:
			drawStatusBar(gamepad);
			drawText(0, 2, "[Web Config Mode]");
			drawText(0, 3, std::string("GP2040-CE : ") + std::string(GP2040VERSION));
			drawText(0, 4, "[http://192.168.7.1]");
			drawText(0, 5, "Preview:");
			drawText(5, 6, "B1 > Button");
			drawText(5, 7, "B2 > Splash");
			break;
		case I2CDisplayAddon::DisplayMode::SPLASH:
			if (getDisplayOptions().splashMode == static_cast<SplashMode>(SPLASH_MODE_NONE)) {
				drawText(0, 4, " Splash NOT enabled.");
				break;
			}
			drawSplashScreen(getDisplayOptions().splashMode, (uint8_t*) Storage::getInstance().getDisplayOptions().splashImage.bytes, 90);
			break;
		case I2CDisplayAddon::DisplayMode::BUTTONS:
			drawStatusBar(gamepad);
			const DisplayOptions& options = getDisplayOptions();
			ButtonLayoutCustomOptions buttonLayoutCustomOptions = options.buttonLayoutCustomOptions;

			switch (options.buttonLayout) {
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
					drawVLXA(7, (isInputHistoryEnabled ? 22 : 28), 7, 2);
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

			switch (options.buttonLayoutRight) {
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
					drawVLXB(6, (isInputHistoryEnabled ? 22 : 28), 7, 2);
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

			if(isInputHistoryEnabled && inputHistoryAddon != nullptr) {
				inputHistoryAddon->drawHistory(&obd);
			}

			break;
	}

	obdDumpBuffer(&obd, NULL);
}

I2CDisplayAddon::DisplayMode I2CDisplayAddon::getDisplayMode() {
	if (configMode) {
		gamepad->read();
		uint16_t buttonState = gamepad->state.buttons;
		if (prevButtonState && !buttonState) { // has button been pressed (held and released)?
			switch (prevButtonState) {
				case (GAMEPAD_MASK_B1):
					prevDisplayMode =
						prevDisplayMode == I2CDisplayAddon::DisplayMode::BUTTONS ?
							I2CDisplayAddon::DisplayMode::CONFIG_INSTRUCTION : I2CDisplayAddon::DisplayMode::BUTTONS;
						break;
				case (GAMEPAD_MASK_B2):
					prevDisplayMode =
						prevDisplayMode == I2CDisplayAddon::DisplayMode::SPLASH ?
							I2CDisplayAddon::DisplayMode::CONFIG_INSTRUCTION : I2CDisplayAddon::DisplayMode::SPLASH;
					break;
				default:
					prevDisplayMode = I2CDisplayAddon::DisplayMode::CONFIG_INSTRUCTION;
			}
		}
		prevButtonState = buttonState;
		return prevDisplayMode;
	} else {
		if (Storage::getInstance().getDisplayOptions().splashMode != static_cast<SplashMode>(SPLASH_MODE_NONE)) {
			uint32_t splashDuration = getDisplayOptions().splashDuration;
			if (splashDuration == 0 || getMillis() < splashDuration) {
				return I2CDisplayAddon::DisplayMode::SPLASH;
			}
		}
	}

	return I2CDisplayAddon::DisplayMode::BUTTONS;
}

const DisplayOptions& I2CDisplayAddon::getDisplayOptions() {
	bool configMode = Storage::getInstance().GetConfigMode();
	return configMode ? Storage::getInstance().getPreviewDisplayOptions() : Storage::getInstance().getDisplayOptions();
}

int I2CDisplayAddon::initDisplay(int typeOverride) {
	const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
	PeripheralI2C* i2c = PeripheralManager::getInstance().getI2C(options.i2cBlock);
	return obdI2CInit(&obd,
	    typeOverride > 0 ? typeOverride : options.size,
		options.i2cAddress,
		options.flip,
		options.invert,
		DISPLAY_USEWIRE,
		i2c,
		-1);
}

bool I2CDisplayAddon::isSH1106(int detectedDisplay) {
	// Only attempt detection if we think we are using a SSD1306 or if auto-detection failed.
	if (detectedDisplay != OLED_SSD1306_3C &&
		detectedDisplay != OLED_SSD1306_3D &&
		detectedDisplay != OLED_NOT_FOUND) {
		return false;
	}

	// To detect an SH1106 we make use of the fact that SH1106 supports read-modify-write operations over I2C, whereas
	// SSD1306 does not.
	// We perform a number of read-modify-write operations and check whether the data we read back matches the data we
	// previously wrote. If it does we can be reasonably confident that we are using a SH1106.

	// We turn the display off for the remainder of this function, we do not want users to observe the random data we
	// are writing.
	obdPower(&obd, false);

	const uint8_t RANDOM_DATA[] = { 0xbf, 0x88, 0x13, 0x41, 0x00 };
	uint8_t buffer[4];
	uint8_t i = 0;
	for (; i < sizeof(RANDOM_DATA); ++i) {
		buffer[0] = 0x80; // one command
		buffer[1] = 0xE0; // read-modify-write
		buffer[2] = 0xC0; // one data
		if (obd.i2c->write(obd.oled_addr, buffer, 3) == 0) {
			break;
		}

		// Read two bytes back, the first byte is a dummy read and the second byte is the byte was actually want.
		if (obd.i2c->read(obd.oled_addr, buffer, 2) == 0) {
			break;
		}

		// Check whether the byte we read is the byte we previously wrote.
		if (i > 0 && buffer[1] != RANDOM_DATA[i - 1]) {
			break;
		}

		// Write the current byte, we will attempt to read it in the next loop iteration.
		buffer[0] = 0xc0; // one data
		buffer[1] = RANDOM_DATA[i]; // data byte
		buffer[2] = 0x80; // one command
		buffer[3] = 0xEE; // end read-modify-write
		if (obd.i2c->write(obd.oled_addr, buffer, 4) == 0) {
			break;
		}
	}

	obdPower(&obd, true);
	return i == sizeof(RANDOM_DATA);
}

void I2CDisplayAddon::clearScreen(int render) {
	obdFill(&obd, 0, render);
}

void I2CDisplayAddon::drawButtonLayoutLeft(ButtonLayoutParamsLeft& options)
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

void I2CDisplayAddon::drawButtonLayoutRight(ButtonLayoutParamsRight& options)
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

void I2CDisplayAddon::drawDiamond(int cx, int cy, int size, uint8_t colour, uint8_t filled)
{
	if (filled) {
		int i;
		for (i = 0; i < size; i++) {
			obdDrawLine(&obd, cx - i, cy - size + i, cx + i, cy - size + i, colour, 0);
			obdDrawLine(&obd, cx - i, cy + size - i, cx + i, cy + size - i, colour, 0);
		}
		obdDrawLine(&obd, cx - size, cy, cx + size, cy, colour, 0); // Fill in the middle
	}
	obdDrawLine(&obd, cx - size, cy, cx, cy - size, colour, 0);
	obdDrawLine(&obd, cx, cy - size, cx + size, cy, colour, 0);
	obdDrawLine(&obd, cx + size, cy, cx, cy + size, colour, 0);
	obdDrawLine(&obd, cx, cy + size, cx - size, cy, colour, 0);
}

void I2CDisplayAddon::drawStickless(int startX, int startY, int buttonRadius, int buttonPadding)
{

	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	obdPreciseEllipse(&obd, startX, startY, buttonRadius, buttonRadius, 1, pressedLeft());
	obdPreciseEllipse(&obd, startX + buttonMargin, startY, buttonRadius, buttonRadius, 1, pressedDown());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 1.875), startY + (buttonMargin / 2), buttonRadius, buttonRadius, 1, pressedRight());
	obdPreciseEllipse(&obd, startX + (buttonMargin * (isInputHistoryEnabled ? 1.875 : 2.25)), startY + buttonMargin * (isInputHistoryEnabled ? 1.5 : 1.875), buttonRadius, buttonRadius, 1, pressedUp());
}

void I2CDisplayAddon::drawWasdBox(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// WASD
	obdPreciseEllipse(&obd, startX, startY + buttonMargin * 0.5, buttonRadius, buttonRadius, 1, pressedLeft());
	obdPreciseEllipse(&obd, startX + buttonMargin, startY + buttonMargin * 0.875, buttonRadius, buttonRadius, 1, pressedDown());
	obdPreciseEllipse(&obd, startX + buttonMargin * 1.5, startY - buttonMargin * 0.125, buttonRadius, buttonRadius, 1, pressedUp());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 2), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pressedRight());
}

void I2CDisplayAddon::drawUDLR(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// UDLR
	obdPreciseEllipse(&obd, startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, pressedLeft());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pressedUp());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pressedDown());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 1.625), startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, pressedRight());
}

void I2CDisplayAddon::drawArcadeStick(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// Stick
	obdPreciseEllipse(&obd, startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

	if (pressedUp()) {
		if (pressedLeft()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedDown()) {
		if (pressedLeft()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else {
			obdPreciseEllipse(&obd, startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedLeft()) {
		obdPreciseEllipse(&obd, startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else if (pressedRight()) {
		obdPreciseEllipse(&obd, startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else {
		obdPreciseEllipse(&obd, startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	}
}

void I2CDisplayAddon::drawVLXA(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// Stick
	obdPreciseEllipse(&obd, startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

	if (pressedUp()) {
		if (pressedLeft()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedDown()) {
		if (pressedLeft()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else {
			obdPreciseEllipse(&obd, startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedLeft()) {
		obdPreciseEllipse(&obd, startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else if (pressedRight()) {
		obdPreciseEllipse(&obd, startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else {
		obdPreciseEllipse(&obd, startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	}
}

void I2CDisplayAddon::drawFightboardMirrored(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);
    const int leftMargin = startX + buttonPadding + buttonRadius;

	obdPreciseEllipse(&obd, leftMargin, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());
	obdPreciseEllipse(&obd, leftMargin + buttonMargin, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, leftMargin + (buttonMargin*2), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, leftMargin + (buttonMargin*3), startY * 1.25, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());

	obdPreciseEllipse(&obd, leftMargin, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
	obdPreciseEllipse(&obd, leftMargin + buttonMargin, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, leftMargin + (buttonMargin*2), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, leftMargin + (buttonMargin*3), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());

    // Extra buttons
    obdPreciseEllipse(&obd, startX + buttonMargin * 0.5, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedL3());
    obdPreciseEllipse(&obd, startX + buttonMargin * 1.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS1());
    obdPreciseEllipse(&obd, startX + buttonMargin * 1.625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedA1());
    obdPreciseEllipse(&obd, startX + buttonMargin * 2.125+0.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS2());
    obdPreciseEllipse(&obd, startX + buttonMargin * 2.75, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedR3());
}

void I2CDisplayAddon::drawTwinStickA(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// Stick
	obdPreciseEllipse(&obd, startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

	if (pressedUp()) {
		if (pressedLeft()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedDown()) {
		if (pressedLeft()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else if (pressedRight()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else {
			obdPreciseEllipse(&obd, startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pressedLeft()) {
		obdPreciseEllipse(&obd, startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else if (pressedRight()) {
		obdPreciseEllipse(&obd, startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else {
		obdPreciseEllipse(&obd, startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	}
}

void I2CDisplayAddon::drawTwinStickB(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// Stick
	obdPreciseEllipse(&obd, startX + (buttonMargin / 2), startY + (buttonMargin / 2), buttonRadius * 1.25, buttonRadius * 1.25, 1, 0);

	if (pGamepad->pressedB4()) {
		if (pGamepad->pressedB3()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 5), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else if (pGamepad->pressedB2()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY + (buttonMargin / 5), buttonRadius, buttonRadius, 1, 1);
		} else {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 2), startY, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pGamepad->pressedB1()) {
		if (pGamepad->pressedB3()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin / 5), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else if (pGamepad->pressedB2()) {
			obdPreciseEllipse(&obd, startX + (buttonMargin * 0.875), startY + (buttonMargin * 0.875), buttonRadius, buttonRadius, 1, 1);
		} else {
			obdPreciseEllipse(&obd, startX + buttonMargin / 2, startY + buttonMargin, buttonRadius, buttonRadius, 1, 1);
		}
	} else if (pGamepad->pressedB3()) {
		obdPreciseEllipse(&obd, startX, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else if (pGamepad->pressedB2()) {
		obdPreciseEllipse(&obd, startX + buttonMargin, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	} else {
		obdPreciseEllipse(&obd, startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, buttonRadius, 1, 1);
	}
}

void I2CDisplayAddon::drawMAMEA(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// MAME
	obdRectangle(&obd, startX, startY + buttonMargin, startX + buttonSize, startY + buttonSize + buttonMargin, 1, pressedLeft());
	obdRectangle(&obd, startX + buttonMargin, startY + buttonMargin, startX + buttonSize + buttonMargin, startY + buttonSize + buttonMargin, 1, pressedDown());
	obdRectangle(&obd, startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pressedUp());
	obdRectangle(&obd, startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin, 1, pressedRight());

}

void I2CDisplayAddon::drawOpenCore0WASDA(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// Open_Core0 WASD
	obdRectangle(&obd, startX, startY + buttonMargin, startX + buttonSize, startY + buttonSize + buttonMargin, 1, pressedLeft());
	obdRectangle(&obd, startX + buttonMargin, startY + buttonMargin, startX + buttonSize + buttonMargin, startY + buttonSize + buttonMargin, 1, pressedDown());
	obdRectangle(&obd, startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pressedUp());
	obdRectangle(&obd, startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin, 1, pressedRight());

	// Aux buttons
    obdPreciseEllipse(&obd, startX - 15 + buttonMargin * 0.5, startY - 25 + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS2());
    obdPreciseEllipse(&obd, startX - 15 + buttonMargin * 1.25, startY - 25 + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS1());
    obdPreciseEllipse(&obd, startX - 15 + buttonMargin * 2, startY -25 + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedA1());
    obdPreciseEllipse(&obd, startX - 15 + buttonMargin * 2.75, startY -25 + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedA2());
    obdPreciseEllipse(&obd, startX - 15 + buttonMargin * 3.5, startY -25 + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedL3());
	obdPreciseEllipse(&obd, startX - 15 + buttonMargin * 4.25, startY -25 + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedR3());

}

void I2CDisplayAddon::drawMAMEB(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// 6-button MAME Style
	obdRectangle(&obd, startX, startY, startX + buttonSize, startY + buttonSize, 1, pGamepad->pressedB3());
	obdRectangle(&obd, startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pGamepad->pressedB4());
	obdRectangle(&obd, startX + buttonMargin * 2, startY, startX + buttonSize + buttonMargin * 2, startY + buttonSize, 1, pGamepad->pressedR1());

	obdRectangle(&obd, startX, startY + buttonMargin, startX + buttonSize, startY + buttonMargin + buttonSize, 1, pGamepad->pressedB1());
	obdRectangle(&obd, startX + buttonMargin, startY + buttonMargin, startX + buttonSize + buttonMargin, startY + buttonMargin + buttonSize, 1, pGamepad->pressedB2());
	obdRectangle(&obd, startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonMargin + buttonSize, 1, pGamepad->pressedR2());

}

void I2CDisplayAddon::drawMAME8B(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// 8-button MAME Style
	obdRectangle(&obd, startX, startY, startX + buttonSize, startY + buttonSize, 1, pGamepad->pressedB3());
	obdRectangle(&obd, startX + buttonMargin, startY - (buttonMargin / 3), startX + buttonSize + buttonMargin, startY - (buttonMargin / 3) + buttonSize, 1, pGamepad->pressedB4());
	obdRectangle(&obd, startX + buttonMargin * 2, startY - (buttonMargin / 3), startX + buttonSize + buttonMargin * 2, startY - (buttonMargin / 3)+ buttonSize, 1, pGamepad->pressedR1());
	obdRectangle(&obd, startX + buttonMargin * 3, startY, startX + buttonSize + buttonMargin * 3, startY + buttonSize, 1, pGamepad->pressedL1());

	obdRectangle(&obd, startX, startY + buttonMargin, startX + buttonSize, startY + buttonMargin + buttonSize, 1, pGamepad->pressedB1());
	obdRectangle(&obd, startX + buttonMargin, startY - (buttonMargin / 3) + buttonMargin, startX + buttonSize + buttonMargin, startY - (buttonMargin / 3) + buttonMargin + buttonSize, 1, pGamepad->pressedB2());
	obdRectangle(&obd, startX + buttonMargin * 2, startY - (buttonMargin / 3) + buttonMargin, startX + buttonSize + buttonMargin * 2, startY - (buttonMargin / 3) + buttonMargin + buttonSize, 1, pGamepad->pressedR2());
	obdRectangle(&obd, startX + buttonMargin * 3, startY + buttonMargin, startX + buttonSize + buttonMargin * 3, startY + buttonMargin + buttonSize, 1, pGamepad->pressedL2());

}

void I2CDisplayAddon::drawOpenCore0WASDB(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	// 8-button Open_Core0 WASD
	obdRectangle(&obd, startX, startY, startX + buttonSize, startY + buttonSize, 1, pGamepad->pressedB3());
	obdRectangle(&obd, startX + buttonMargin, startY - (buttonMargin / 3), startX + buttonSize + buttonMargin, startY - (buttonMargin / 3) + buttonSize, 1, pGamepad->pressedB4());
	obdRectangle(&obd, startX + buttonMargin * 2, startY - (buttonMargin / 3), startX + buttonSize + buttonMargin * 2, startY - (buttonMargin / 3)+ buttonSize, 1, pGamepad->pressedR1());
	obdRectangle(&obd, startX + buttonMargin * 3, startY, startX + buttonSize + buttonMargin * 3, startY + buttonSize, 1, pGamepad->pressedL1());

	obdRectangle(&obd, startX, startY + buttonMargin, startX + buttonSize, startY + buttonMargin + buttonSize, 1, pGamepad->pressedB1());
	obdRectangle(&obd, startX + buttonMargin, startY - (buttonMargin / 3) + buttonMargin, startX + buttonSize + buttonMargin, startY - (buttonMargin / 3) + buttonMargin + buttonSize, 1, pGamepad->pressedB2());
	obdRectangle(&obd, startX + buttonMargin * 2, startY - (buttonMargin / 3) + buttonMargin, startX + buttonSize + buttonMargin * 2, startY - (buttonMargin / 3) + buttonMargin + buttonSize, 1, pGamepad->pressedR2());
	obdRectangle(&obd, startX + buttonMargin * 3, startY + buttonMargin, startX + buttonSize + buttonMargin * 3, startY + buttonMargin + buttonSize, 1, pGamepad->pressedL2());

}

void I2CDisplayAddon::drawKeyboardAngled(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// MixBox
	drawDiamond(startX, startY, buttonRadius, 1, pressedLeft());
	drawDiamond(startX + buttonMargin / 2, startY + buttonMargin / 2, buttonRadius, 1, pressedDown());
	drawDiamond(startX + buttonMargin, startY, buttonRadius, 1, pressedUp());
	drawDiamond(startX + buttonMargin, startY + buttonMargin, buttonRadius, 1, pressedRight());
}

void I2CDisplayAddon::drawVewlix(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// inner circle for turbo enabled
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_2) obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius - 3, buttonRadius - 3, 1, false);  //P1 B3
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_3) obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false);  //P2 B4
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_5) obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false);  //P3 R1
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_4) obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false); //P4 L1

	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_0) obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius - 3, buttonRadius - 3, 1, false);  //K1 B1
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_1) obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false);  //K2 B2
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_7) obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false); //K3 R2
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_6) obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false); //K4 L2

	// 8-button Vewlix
	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void I2CDisplayAddon::drawVLXB(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 9-button Hori VLX
	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL2());

	obdPreciseEllipse(&obd, startX + (buttonMargin * 7.4) - (buttonMargin / 3.5), startY + buttonMargin - (buttonMargin / 1.5), buttonRadius *.8, buttonRadius * .8, 1, pGamepad->pressedS2());
}

void I2CDisplayAddon::drawFightboard(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	obdPreciseEllipse(&obd, (startX + buttonMargin * 3.625), startY * 1.25, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, (startX + buttonMargin * 4.625), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, (startX + buttonMargin * 5.625), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, (startX + buttonMargin * 6.625), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, (startX + buttonMargin * 3.625), startY + buttonMargin * 1.25, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, (startX + buttonMargin * 4.625), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, (startX + buttonMargin * 5.625), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, (startX + buttonMargin * 6.625), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL2());

    // Extra buttons
	obdPreciseEllipse(&obd, startX + buttonMargin * 4.5, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedL3());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS1());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedA1());
	obdPreciseEllipse(&obd, startX + buttonMargin * 6.125+0.0625, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedS2());
	obdPreciseEllipse(&obd, startX + buttonMargin * 6.75, startY + (buttonMargin * 1.5), 3, 3, 1, pGamepad->pressedR3());
}

void I2CDisplayAddon::drawVewlix7(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button Vewlix
	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75) - (buttonMargin / 3), startY + buttonMargin + (buttonMargin * 0.2), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	//obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75) - (buttonMargin / 3), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, gamepad->pressedL2());
}

void I2CDisplayAddon::drawSega2p(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button Sega2P
	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + (buttonMargin / 3), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + buttonMargin + (buttonMargin / 3), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void I2CDisplayAddon::drawNoir8(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// inner circle for turbo enabled
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_2) obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + (buttonMargin / 3.5), buttonRadius - 3, buttonRadius - 3, 1, false);  //P1 B3
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_3) obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false);  //P2 B4
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_5) obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false);  //P3 R1
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_4) obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY, buttonRadius - 3, buttonRadius - 3, 1, false); //P4 L1

	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_0) obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + buttonMargin + (buttonMargin / 3.5), buttonRadius - 3, buttonRadius - 3, 1, false);  //K1 B1
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_1) obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false);  //K2 B2
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_7) obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false); //K3 R2
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_6) obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius - 3, buttonRadius - 3, 1, false); //K4 L2

	// 8-button Noir8
	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + (buttonMargin / 3.5), buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + buttonMargin + (buttonMargin / 3.5), buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void I2CDisplayAddon::drawCapcom(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button Capcom
	obdPreciseEllipse(&obd, startX + buttonMargin * 3.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + buttonMargin * 4.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, startX + buttonMargin * 6.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, startX + buttonMargin * 3.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + buttonMargin * 4.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, startX + buttonMargin * 6.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void I2CDisplayAddon::drawCapcom6(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 6-button Capcom
	obdPreciseEllipse(&obd, startX + buttonMargin * 3.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + buttonMargin * 4.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.25, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedR1());

	obdPreciseEllipse(&obd, startX + buttonMargin * 3.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + buttonMargin * 4.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
}

void I2CDisplayAddon::drawSticklessButtons(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// inner circle for turbo enabled
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_0) obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + buttonMargin, buttonRadius - 3, buttonRadius - 3, 1, false);  //K1 
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_1) obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false);  //K2
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_2) obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY, buttonRadius - 3, buttonRadius - 3, 1, false);  //P1
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_3) obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false);  //P2

	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_4) obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY, buttonRadius - 3, buttonRadius - 3, 1, false); //P4
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_5) obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false);  //P3
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_6) obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius - 3, buttonRadius - 3, 1, false); //K4
	if(gamepad->turboState.buttons & GAMEPAD_BUTTON_7) obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius - 3, buttonRadius - 3, 1, false); //K3

	// 8-button
	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, startX + (buttonMargin * 2.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 3.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 4.75), startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, startX + (buttonMargin * 5.75), startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void I2CDisplayAddon::drawWasdButtons(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button
	obdPreciseEllipse(&obd, startX + buttonMargin * 3.625, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + buttonMargin * 4.625, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.625, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, startX + buttonMargin * 6.625, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, startX + buttonMargin * 3.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + buttonMargin * 4.25, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.25, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, startX + buttonMargin * 6.25, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

void I2CDisplayAddon::drawArcadeButtons(int startX, int startY, int buttonRadius, int buttonPadding)
{
	const int buttonMargin = buttonPadding + (buttonRadius * 2);

	// 8-button
	obdPreciseEllipse(&obd, startX + buttonMargin * 3.125, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedB3());
	obdPreciseEllipse(&obd, startX + buttonMargin * 4.125, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB4());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.125, startY - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR1());
	obdPreciseEllipse(&obd, startX + buttonMargin * 6.125, startY, buttonRadius, buttonRadius, 1, pGamepad->pressedL1());

	obdPreciseEllipse(&obd, startX + buttonMargin * 2.875, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedB1());
	obdPreciseEllipse(&obd, startX + buttonMargin * 3.875, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedB2());
	obdPreciseEllipse(&obd, startX + buttonMargin * 4.875, startY + buttonMargin - (buttonMargin / 4), buttonRadius, buttonRadius, 1, pGamepad->pressedR2());
	obdPreciseEllipse(&obd, startX + buttonMargin * 5.875, startY + buttonMargin, buttonRadius, buttonRadius, 1, pGamepad->pressedL2());
}

// I pulled this out of my PR, brought it back because of recent talks re: SOCD and rhythm games
// Enjoy!

void I2CDisplayAddon::drawDancepadA(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	obdRectangle(&obd, startX, startY + buttonMargin, startX + buttonSize, startY + buttonSize + buttonMargin, 1, pressedLeft());
	obdRectangle(&obd, startX + buttonMargin, startY + buttonMargin * 2, startX + buttonSize + buttonMargin, startY + buttonSize + buttonMargin * 2, 1, pressedDown());
	obdRectangle(&obd, startX + buttonMargin, startY, startX + buttonSize + buttonMargin, startY + buttonSize, 1, pressedUp());
	obdRectangle(&obd, startX + buttonMargin * 2, startY + buttonMargin, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin, 1, pressedRight());
}

void I2CDisplayAddon::drawDancepadB(int startX, int startY, int buttonSize, int buttonPadding)
{
	const int buttonMargin = buttonPadding + buttonSize;

	obdRectangle(&obd, startX, startY, startX + buttonSize, startY + buttonSize, 1, pGamepad->pressedB2()); // Up/Left
	obdRectangle(&obd, startX, startY + buttonMargin * 2, startX + buttonSize, startY + buttonSize + buttonMargin * 2, 1, pGamepad->pressedB4()); // Down/Left
	obdRectangle(&obd, startX + buttonMargin * 2, startY, startX + buttonSize + buttonMargin * 2, startY + buttonSize, 1, pGamepad->pressedB1()); // Up/Right
	obdRectangle(&obd, startX + buttonMargin * 2, startY + buttonMargin * 2, startX + buttonSize + buttonMargin * 2, startY + buttonSize + buttonMargin * 2, 1, pGamepad->pressedB3()); // Down/Right
}

void I2CDisplayAddon::drawBlankA(int startX, int startY, int buttonSize, int buttonPadding)
{
}

void I2CDisplayAddon::drawBlankB(int startX, int startY, int buttonSize, int buttonPadding)
{
}

void I2CDisplayAddon::drawSplashScreen(int splashMode, uint8_t * splashChoice, int splashSpeed)
{
    int mils = getMillis();
    switch (splashMode)
	{
		case SPLASH_MODE_STATIC: // Default, display static or custom image
			obdDrawSprite(&obd, splashChoice, 128, 64, 16, 0, 0, 1);
			break;
		case SPLASH_MODE_CLOSEIN: // Close-in. Animate the GP2040 logo
			obdDrawSprite(&obd, (uint8_t *)bootLogoTop, 43, 39, 6, 43, std::min<int>((mils / splashSpeed) - 39, 0), 1);
			obdDrawSprite(&obd, (uint8_t *)bootLogoBottom, 80, 21, 10, 24, std::max<int>(64 - (mils / (splashSpeed * 2)), 44), 1);
			break;
        case SPLASH_MODE_CLOSEINCUSTOM: // Close-in on custom image or delayed close-in if custom image does not exist
            obdDrawSprite(&obd, splashChoice, 128, 64, 16, 0, 0, 1);
            if (mils > 2500) {
                int milss = mils - 2500;
                obdRectangle(&obd, 0, 0, 127, 1 + (milss / splashSpeed), 0, 1);
                obdRectangle(&obd, 0, 63, 127, 62 - (milss / (splashSpeed * 2)), 0, 1);
                obdDrawSprite(&obd, (uint8_t *)bootLogoTop, 43, 39, 6, 43, std::min<int>((milss / splashSpeed) - 39, 0), 1);
                obdDrawSprite(&obd, (uint8_t *)bootLogoBottom, 80, 21, 10, 24, std::max<int>(64 - (milss / (splashSpeed * 2)), 44), 1);
            }
            break;
	}
}

void I2CDisplayAddon::drawText(int x, int y, std::string text) {
	obdWriteString(&obd, 0, x, y, (char*)text.c_str(), FONT_6x8, 0, 0);
}

void I2CDisplayAddon::drawStatusBar(Gamepad * gamepad)
{
	const TurboOptions& turboOptions = Storage::getInstance().getAddonOptions().turboOptions;

	// Limit to 21 chars with 6x8 font for now
	statusBar.clear();

	switch (gamepad->getOptions().inputMode)
	{
		case INPUT_MODE_HID:    statusBar += "DINPUT"; break;
		case INPUT_MODE_SWITCH: statusBar += "SWITCH"; break;
		case INPUT_MODE_XINPUT: statusBar += "XINPUT"; break;
		case INPUT_MODE_MDMINI: statusBar += "GEN/MD"; break;
		case INPUT_MODE_NEOGEO: statusBar += "NGMINI"; break;
		case INPUT_MODE_PCEMINI: statusBar += "PCE/TG"; break;
		case INPUT_MODE_EGRET: statusBar += "EGRET"; break;
		case INPUT_MODE_ASTRO: statusBar += "ASTRO"; break;
		case INPUT_MODE_PSCLASSIC: statusBar += "PSC"; break;
		case INPUT_MODE_XBOXORIGINAL: statusBar += "OGXBOX"; break;
		case INPUT_MODE_PS4:
			if ( PS4Data::getInstance().ps4ControllerType == PS4ControllerType::PS4_CONTROLLER ) {
				if (PS4Data::getInstance().authsent == true )
					statusBar += "PS4:AS";
				else
					statusBar += "PS4   ";
			} else if ( PS4Data::getInstance().ps4ControllerType == PS4ControllerType::PS4_ARCADESTICK ) {
				if (PS4Data::getInstance().authsent == true )
					statusBar += "PS5:AS";
				else
					statusBar += "PS5   ";
			}
			break;
		case INPUT_MODE_XBONE:    statusBar += "XBONE"; break;
		case INPUT_MODE_KEYBOARD: statusBar += "HID-KB"; break;
		case INPUT_MODE_CONFIG: statusBar += "CONFIG"; break;
	}

	if ( turboOptions.enabled && isValidPin(turboOptions.buttonPin) ) {
		statusBar += " T";
		if ( turboOptions.shotCount < 10 ) // padding
			statusBar += "0";
		statusBar += std::to_string(turboOptions.shotCount);
	} else {
		statusBar += "    "; // no turbo, don't show Txx setting
	}
	switch (gamepad->getOptions().dpadMode)
	{

		case DPAD_MODE_DIGITAL:      statusBar += " D"; break;
		case DPAD_MODE_LEFT_ANALOG:  statusBar += " L"; break;
		case DPAD_MODE_RIGHT_ANALOG: statusBar += " R"; break;
	}

	switch (Gamepad::resolveSOCDMode(gamepad->getOptions()))
	{
		case SOCD_MODE_NEUTRAL:               statusBar += " SOCD-N"; break;
		case SOCD_MODE_UP_PRIORITY:           statusBar += " SOCD-U"; break;
		case SOCD_MODE_SECOND_INPUT_PRIORITY: statusBar += " SOCD-L"; break;
		case SOCD_MODE_FIRST_INPUT_PRIORITY:  statusBar += " SOCD-F"; break;
		case SOCD_MODE_BYPASS:                statusBar += " SOCD-X"; break;
	}
	if (Storage::getInstance().getAddonOptions().macroOptions.enabled)
		statusBar += " M";
	drawText(0, 0, statusBar);
}

bool I2CDisplayAddon::pressedUp()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return pGamepad->pressedUp();
		case DPAD_MODE_LEFT_ANALOG:  return pGamepad->state.ly == GAMEPAD_JOYSTICK_MIN;
		case DPAD_MODE_RIGHT_ANALOG: return pGamepad->state.ry == GAMEPAD_JOYSTICK_MIN;
	}

	return false;
}

bool I2CDisplayAddon::pressedDown()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return pGamepad->pressedDown();
		case DPAD_MODE_LEFT_ANALOG:  return pGamepad->state.ly == GAMEPAD_JOYSTICK_MAX;
		case DPAD_MODE_RIGHT_ANALOG: return pGamepad->state.ry == GAMEPAD_JOYSTICK_MAX;
	}

	return false;
}

bool I2CDisplayAddon::pressedLeft()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return pGamepad->pressedLeft();
		case DPAD_MODE_LEFT_ANALOG:  return pGamepad->state.lx == GAMEPAD_JOYSTICK_MIN;
		case DPAD_MODE_RIGHT_ANALOG: return pGamepad->state.rx == GAMEPAD_JOYSTICK_MIN;
	}

	return false;
}

bool I2CDisplayAddon::pressedRight()
{
	switch (gamepad->getOptions().dpadMode)
	{
		case DPAD_MODE_DIGITAL:      return pGamepad->pressedRight();
		case DPAD_MODE_LEFT_ANALOG:  return pGamepad->state.lx == GAMEPAD_JOYSTICK_MAX;
		case DPAD_MODE_RIGHT_ANALOG: return pGamepad->state.rx == GAMEPAD_JOYSTICK_MAX;
	}

	return false;
}

void I2CDisplayAddon::attachInputHistoryAddon(InputHistoryAddon* pInputHistoryAddon) {
	inputHistoryAddon = pInputHistoryAddon;
}
