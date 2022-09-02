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

// i2c Display Module
#define I2CDisplayName "I2CDisplay"

// i2C OLED Display
class I2CDisplayAddon : public GPAddon
{
public:
	virtual bool available();  // GPAddon
	virtual void setup();
	virtual void process();
	virtual std::string name() { return I2CDisplayName; }
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
	void drawSplashScreen(int splashMode, int splashSpeed);
	void drawDancepadA(int startX, int startY, int buttonSize, int buttonPadding);
	void drawDancepadB(int startX, int startY, int buttonSize, int buttonPadding);
	uint8_t ucBackBuffer[1024];
	OBDISP obd;
	std::string statusBar;
	Gamepad* gamepad;
	Gamepad* pGamepad;
};

#endif
