/*
 * SPDX-License-Identifier: MIT
 * SPDX-FileCopyrightText: Copyright (c) 2021 Jason Skuby (mytechtoybox.com)
 */

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include <string>
#include <hardware/i2c.h>
#include "OneBitDisplay.h"
#include "BoardConfig.h"
#include "gpaddon.h"
#include "gamepad.h"
#include "storagemanager.h"

#ifndef HAS_I2C_DISPLAY
#define HAS_I2C_DISPLAY -1
#endif

#ifndef DISPLAY_I2C_ADDR
#define DISPLAY_I2C_ADDR 0x3C
#endif

#ifndef I2C_SDA_PIN
#define I2C_SDA_PIN -1
#endif

#ifndef I2C_SCL_PIN
#define I2C_SCL_PIN -1
#endif

#ifndef I2C_BLOCK
#define I2C_BLOCK i2c0
#endif

#ifndef I2C_SPEED
#define I2C_SPEED 800000
#endif

#ifndef DISPLAY_SIZE
#define DISPLAY_SIZE OLED_128x64
#endif

#ifndef DISPLAY_FLIP
#define DISPLAY_FLIP 0
#endif

#ifndef DISPLAY_INVERT
#define DISPLAY_INVERT 0
#endif

#ifndef DISPLAY_USEWIRE
#define DISPLAY_USEWIRE 1
#endif

#ifndef DISPLAY_SAVER_TIMEOUT
#define DISPLAY_SAVER_TIMEOUT 0
#endif

#ifndef SPLASH_DURATION
#define SPLASH_DURATION 7500 // Duration in milliseconds
#endif

// i2c Display Module
#define I2CDisplayName "I2CDisplay"

// i2C OLED Display
class I2CDisplayAddon : public GPAddon
{
public:
	virtual bool available();
	virtual void setup();
	virtual void preprocess() {}
	virtual void process();
	virtual std::string name() { return I2CDisplayName; }
private:
	int initDisplay(int typeOverride);
	bool isSH1106(int detectedDisplay);
	void clearScreen(int render); // DisplayModule
	void drawStickless(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawWasdBox(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawArcadeStick(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawStatusBar(Gamepad*);
	void drawText(int startX, int startY, std::string text);
	void initMenu(char**);
	//Adding my stuff here, remember to sort before PR
	void drawDiamond(int cx, int cy, int size, uint8_t colour, uint8_t filled);
	void drawUDLR(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawMAMEA(int startX, int startY, int buttonSize, int buttonPadding);
	void drawMAMEB(int startX, int startY, int buttonSize, int buttonPadding);
	void drawKeyboardAngled(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawVewlix(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawVewlix7(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawSega2p(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawNoir8(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawCapcom(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawCapcom6(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawSticklessButtons(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawWasdButtons(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawArcadeButtons(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawSplashScreen(int splashMode, uint8_t* splashChoice, int splashSpeed);
	void drawDancepadA(int startX, int startY, int buttonSize, int buttonPadding);
	void drawDancepadB(int startX, int startY, int buttonSize, int buttonPadding);
	void drawTwinStickA(int startX, int startY, int buttonSize, int buttonPadding);
	void drawTwinStickB(int startX, int startY, int buttonSize, int buttonPadding);
	void drawBlankA(int startX, int startY, int buttonSize, int buttonPadding);
	void drawBlankB(int startX, int startY, int buttonSize, int buttonPadding);
	void drawVLXA(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawVLXB(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawButtonLayoutLeft(ButtonLayoutCustomOptions options);
	void drawButtonLayoutRight(ButtonLayoutCustomOptions options);
	void drawFightboard(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawFightboardMirrored(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawFightboardStick(int startX, int startY, int buttonRadius, int buttonPadding);
	void drawFightboardStickMirrored(int startX, int startY, int buttonRadius, int buttonPadding);
	bool pressedUp();
	bool pressedDown();
	bool pressedLeft();
	bool pressedRight();
	const BoardOptions& getBoardOptions();
	bool isDisplayPowerOff();
	void setDisplayPower(uint8_t status);
	uint32_t displaySaverTimeout = 0;
	int32_t displaySaverTimer;
	uint8_t displayIsPowerOn = 1;
	uint32_t prevMillis;
	uint8_t ucBackBuffer[1024];
	OBDISP obd;
	std::string statusBar;
	Gamepad* gamepad;
	Gamepad* pGamepad;
	bool configMode;

	enum DisplayMode {
		CONFIG_INSTRUCTION,
		BUTTONS,
		SPLASH
	};

	DisplayMode getDisplayMode();
	DisplayMode prevDisplayMode;
	uint16_t prevButtonState;
};

#endif
