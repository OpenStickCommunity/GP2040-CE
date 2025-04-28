#include "obd_ssd1306.h"

void GPGFX_OBD_SSD1306::init(GPGFX_DisplayTypeOptions options) {
    _options.displayType = options.displayType;
    _options.i2c = options.i2c;
    _options.spi = options.spi;
    _options.size = options.size;
    _options.address = options.address;
    _options.orientation = options.orientation;
    _options.inverted = options.inverted;
    _options.font = options.font;

    obdI2CInit(&obd,
        _options.size,
        _options.address,
        _options.orientation,
        _options.inverted,
        1,
        _options.i2c,
        -1
    );

	obdSetFonts(GP_Font_Standard, GP_Font_Basic, GP_Font_Big);

	const int detectedDisplay = initDisplay(0);
	if (isSH1106(detectedDisplay)) {
		// The display is actually a SH1106 that was misdetected as a SSD1306 by OneBitDisplay.
		// Reinitialize as SH1106.
		initDisplay(OLED_132x64);
	}

	obdSetContrast(&obd, 0xFF);
	obdSetBackBuffer(&obd, ucBackBuffer); // TODO: replace with global back buffer
	clearScreen(1);
}

void GPGFX_OBD_SSD1306::setPower(bool isPowered) {
    obdPower(&obd, isPowered);
}

void GPGFX_OBD_SSD1306::clear() {
    clearScreen(0);
}

void GPGFX_OBD_SSD1306::drawPixel(uint8_t x, uint8_t y, uint32_t color) {
	obdSetPixel(&obd, x, y, color, 1);
}

uint32_t GPGFX_OBD_SSD1306::getPixel(uint8_t x, uint8_t y) {
	return 0;
}

void GPGFX_OBD_SSD1306::drawText(uint8_t x, uint8_t y, std::string text, uint8_t invert) {
    obdWriteString(&obd, 0, x, y, (char*)text.c_str(), FONT_6x8, 0, 1);
}

void GPGFX_OBD_SSD1306::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color, uint8_t filled) {
    obdDrawLine(&obd, x1, y1, x2, y2, color, filled);
}

void GPGFX_OBD_SSD1306::drawEllipse(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled) {
    obdPreciseEllipse(&obd, x, y, radiusX, radiusY, color, filled);
}

void GPGFX_OBD_SSD1306::drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t filled, double rotationAngle) {
    obdRectangle(&obd, x, y, width, height, color, filled);
}

void GPGFX_OBD_SSD1306::drawSprite(uint8_t* spriteData, uint16_t width, uint16_t height, uint16_t pitch, uint16_t x, uint16_t y, uint8_t priority, double scale) {
    obdDrawSprite(&obd, spriteData, width, height, pitch, x, y, priority);
}

void GPGFX_OBD_SSD1306::drawBuffer(uint8_t *pBuffer) {
    obdDumpBuffer(&obd, NULL);
}

//------------------------------------------------------------

int GPGFX_OBD_SSD1306::initDisplay(int typeOverride) {
	return obdI2CInit(&obd,
	    typeOverride > 0 ? typeOverride : _options.size,
		_options.address,
		_options.orientation,
		_options.inverted,
		1,
		_options.i2c,
		-1
    );
}

bool GPGFX_OBD_SSD1306::isSH1106(int detectedDisplay) {
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

void GPGFX_OBD_SSD1306::clearScreen(int render) {
	obdFill(&obd, 0, render);
}
