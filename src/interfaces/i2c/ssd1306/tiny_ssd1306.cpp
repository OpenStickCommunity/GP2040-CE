#include "tiny_ssd1306.h"

void GPGFX_TinySSD1306::init(GPGFX_DisplayTypeOptions options) {
    _options.displayType = options.displayType;
    _options.i2c = options.i2c;
    _options.spi = options.spi;
    _options.size = options.size;
    _options.address = options.address;
    _options.orientation = options.orientation;
    _options.inverted = options.inverted;
    _options.font = options.font;

    _options.i2c->readRegister(_options.address, 0x00, &this->screenType, 1);
    this->screenType &= 0x0F;

    if (isSH1106(this->screenType)) {
        this->screenType = SCREEN_132x64;
    }

	uint8_t commands[] = {
		0x00,
		CommandOps::DISPLAY_OFF,
		CommandOps::SET_LOW_COLUMN,
		CommandOps::SET_HIGH_COLUMN,
		CommandOps::SET_START_LINE,

		CommandOps::MEMORY_MODE,
		0x00,

		CommandOps::SET_CONTRAST,
		0xFF,

		(!_options.inverted ? CommandOps::NORMAL_DISPLAY : CommandOps::INVERT_DISPLAY),

		CommandOps::SET_MULTIPLEX,
		63,

		CommandOps::SET_DISPLAY_OFFSET,
		0x00,

		CommandOps::SET_DISPLAY_CLOCK_DIVIDE,
		0x80,

		CommandOps::SET_PRECHARGE,
		0x22,

		CommandOps::SET_COM_PINS,
		0x12,

		CommandOps::SET_VCOM_DETECT,
		0x40,

		CommandOps::CHARGE_PUMP,
		0x14,

		((_options.orientation == 2) || (_options.orientation == 3) ? CommandOps::SEGMENT_REMAP_0 : CommandOps::SEGMENT_REMAP_127),
		((_options.orientation == 1) || (_options.orientation == 3) ? CommandOps::COM_SCAN_NORMAL : CommandOps::COM_SCAN_REVERSE),

		CommandOps::FULL_DISPLAY_ON_RESUME,
		CommandOps::DISPLAY_ON
	};

    sendCommands(commands, sizeof(commands));

    clear();
    drawBuffer(NULL);
}

bool GPGFX_TinySSD1306::isSH1106(int detectedDisplay) {

    this->setPower(false);

    const uint8_t RANDOM_DATA[] = { 0xbf, 0x88, 0x13, 0x41, 0x00 };
    uint8_t buffer[4];
    uint8_t i = 0;
    for (; i < sizeof(RANDOM_DATA); ++i) {
        buffer[0] = 0x80; // one command
        buffer[1] = 0xE0; // read-modify-write
        buffer[2] = 0xC0; // one data
        if (_options.i2c->write(_options.address, buffer, 3, false) == 0) {
            break;
        }

        // Read two bytes back, the first byte is a dummy read and the second byte is the byte was actually want.
        if (_options.i2c->read(_options.address, buffer, 2, false) == 0) {
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
        if (_options.i2c->write(_options.address, buffer, 4, false) == 0) {
            break;
        }
    }

    this->setPower(true);
    return i == sizeof(RANDOM_DATA);
}

void GPGFX_TinySSD1306::setPower(bool isPowered) {
	_isPowered = isPowered;
	sendCommand(_isPowered ? CommandOps::DISPLAY_ON : CommandOps::DISPLAY_OFF);
}

void GPGFX_TinySSD1306::clear() {
	memset(frameBuffer, 0, MAX_SCREEN_SIZE);
}

uint32_t GPGFX_TinySSD1306::getPixel(uint8_t x, uint8_t y) {
	uint16_t row, bitIndex;
    uint32_t result = 0;

	if ((x<MAX_SCREEN_WIDTH) and (y<MAX_SCREEN_HEIGHT))
	{
        if (this->screenType == ScreenAlternatives::SCREEN_132x64) {
            x+=2;
        }

		row=((y/8)*MAX_SCREEN_WIDTH)+x;
		bitIndex=y % 8;

        result = (frameBuffer[row] >> bitIndex) && 0x01;
	}

    return result;
}

void GPGFX_TinySSD1306::drawPixel(uint8_t x, uint8_t y, uint32_t color) {
	uint16_t row, bitIndex;

	if ((x<MAX_SCREEN_WIDTH) and (y<MAX_SCREEN_HEIGHT))
	{
        if (this->screenType == ScreenAlternatives::SCREEN_132x64) {
            x+=2;
        }

        if (x>=MAX_SCREEN_WIDTH) return;

		row=((y/8)*MAX_SCREEN_WIDTH)+x;
		bitIndex=y % 8;

        if (color == 1) {
		    frameBuffer[row] |= (color<<bitIndex);
        } else if (color == 0) {
            frameBuffer[row] &= ~(1<<bitIndex);
        } else {
            frameBuffer[row] ^= (1 << bitIndex);
        }
	}
}

void GPGFX_TinySSD1306::drawText(uint8_t x, uint8_t y, std::string text, uint8_t invert) {
	uint8_t spriteX, spriteY;
	uint8_t spriteByte;
	uint8_t spriteBit;
	uint8_t color;
	uint8_t currChar, glyphIndex;
	uint8_t charOffset = 0;
	const uint8_t* currGlyph;

    uint8_t maxTextSize = (MAX_SCREEN_WIDTH / _options.font.width);

	for (uint8_t charIndex = 0; charIndex < MIN(text.size(), maxTextSize); charIndex++) {
		currChar = text[charIndex];
		glyphIndex = currChar - GPGFX_FONT_CHAR_OFFSET;
		currGlyph = &_options.font.fontData[glyphIndex * ((_options.font.width - 1) * (_options.font.height/8))];

		for (spriteY = 0; spriteY < _options.font.height; spriteY++) {
			for (spriteX = 0; spriteX < _options.font.width-1; spriteX++) {
				spriteBit = spriteY % 8;
				spriteByte = currGlyph[spriteX];
				color = ((spriteByte >> spriteBit) & 0x01);
                if (invert) color = !color;
				drawPixel(((x*_options.font.width)+spriteX)+charOffset, (y*_options.font.height)+spriteY, color);
			}
		}

		charOffset += _options.font.width;
	}
}

void GPGFX_TinySSD1306::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color, uint8_t filled) {
    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int stepX = (x1 < x2) ? 1 : -1;
    int stepY = (y1 < y2) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        drawPixel(x1, y1, color);

        if (x1 == x2 && y1 == y2) break;

        int errDouble = 2 * err;
        if (errDouble > -dy) {
            err -= dy;
            x1 += stepX;
        }
        if (errDouble < dx) {
            err += dx;
            y1 += stepY;
        }
    }
}

void GPGFX_TinySSD1306::drawArc(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled, double startAngle, double endAngle, uint8_t closed) {
    // Convert degrees to radians
    startAngle = startAngle * M_PI / 180.0;
    endAngle = endAngle * M_PI / 180.0;

    // Angle step based on the resolution you want
    double angleStep = 0.01; // Adjust as needed for smoother arcs

    for (double angle = startAngle; angle < endAngle; angle += angleStep) {
        int xPos = x + static_cast<int>(radiusX * cos(angle));
        int yPos = y + static_cast<int>(radiusY * sin(angle));
        drawPixel(xPos, yPos, color);
    }

    // Draw the last point
    int xPos = x + static_cast<int>(radiusX * cos(endAngle));
    int yPos = y + static_cast<int>(radiusY * sin(endAngle));
    drawPixel(xPos, yPos, color);

    if (closed) {
        drawLine(x, y, (x + static_cast<int>(radiusX * cos(startAngle))), (y + static_cast<int>(radiusY * sin(startAngle))), color, filled);
        drawLine(x, y, (x + static_cast<int>(radiusX * cos(endAngle))), (y + static_cast<int>(radiusY * sin(endAngle))), color, filled);
    }

    // If filled is true, fill the arc
    if (filled) {
        // Draw lines to fill the arc
        for (double angle = startAngle; angle <= endAngle; angle += angleStep) {
            int xPosStart = x;
            int yPosStart = y;
            int xPosEnd = x + static_cast<int>(radiusX * cos(angle));
            int yPosEnd = y + static_cast<int>(radiusY * sin(angle));
            // Draw line from center to arc point
            // You may replace this with your actual line drawing function
            //drawPixel(xPosStart, yPosStart, color);
            //drawPixel(xPosEnd, yPosEnd, color);
            drawLine(xPosStart, yPosStart, xPosEnd, yPosEnd, color, filled);
        }
    }
}

void GPGFX_TinySSD1306::drawEllipse(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled) {
    //printf("Ellipse %d, %d, %d, %d, %d, %d\n", x, y, radiusX, radiusY, color, filled);
	long x1 = -radiusX, y1 = 0;
	long e2 = radiusY, dx = (1 + 2 * x1) * e2 * e2;
	long dy = x1 * x1, err = dx + dy;
	long diff = 0;

	while (x1 <= 0) {
		drawPixel(x - x1, y + y1, color);
		drawPixel(x + x1, y + y1, color);
		drawPixel(x + x1, y - y1, color);
		drawPixel(x - x1, y - y1, color);

		if (filled)
		{
			for (int i = 0; i < ((x - x1) - (x + x1)) / 2; i++) {
				drawPixel(x - i, y + y1, color);
				drawPixel(x + i, y + y1, color);
				drawPixel(x + i, y - y1, color);
				drawPixel(x - i, y - y1, color);
			}
		}

		e2 = 2 * err;

		if (e2 >= dx) {
			x1++;
			err += dx += 2 * (long)radiusY * radiusY;
		}

		if (e2 <= dy) {
			y1++;
			err += dy += 2 * (long)radiusX * radiusX;
		}
	};

	while (y1++ < radiusY) {
		drawPixel(x, y + y1, color);
		drawPixel(x, y - y1, color);
	}
}

void GPGFX_TinySSD1306::drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t filled, double rotationAngle) {
    // Calculate center point of the rectangle
    double centerX = (x + width) / 2.0;
    double centerY = (y + height) / 2.0;

    // Calculate half width and half height for easier calculations
    double halfWidth = (width - x) / 2.0;
    double halfHeight = (height - y) / 2.0;

    // Convert rotation angle to radians
    double angleRad = rotationAngle * M_PI / 180.0;

    // Pre-calculate sine and cosine of the rotation angle
    double cosA = cos(angleRad);
    double sinA = sin(angleRad);

    // Calculate rotated coordinates for each corner of the rectangle
    double x0 = centerX + cosA * (-halfWidth) - sinA * (-halfHeight);
    double y0 = centerY + sinA * (-halfWidth) + cosA * (-halfHeight);

    double x1 = centerX + cosA * (halfWidth) - sinA * (-halfHeight);
    double y1 = centerY + sinA * (halfWidth) + cosA * (-halfHeight);

    double x2 = centerX + cosA * (halfWidth) - sinA * (halfHeight);
    double y2 = centerY + sinA * (halfWidth) + cosA * (halfHeight);

    double x3 = centerX + cosA * (-halfWidth) - sinA * (halfHeight);
    double y3 = centerY + sinA * (-halfWidth) + cosA * (halfHeight);

    // Round coordinates to nearest integer
    uint16_t x0_rounded = (uint16_t)round(x0);
    uint16_t y0_rounded = (uint16_t)round(y0);
    uint16_t x1_rounded = (uint16_t)round(x1);
    uint16_t y1_rounded = (uint16_t)round(y1);
    uint16_t x2_rounded = (uint16_t)round(x2);
    uint16_t y2_rounded = (uint16_t)round(y2);
    uint16_t x3_rounded = (uint16_t)round(x3);
    uint16_t y3_rounded = (uint16_t)round(y3);

    // Draw lines between rotated coordinates
    drawLine(x0_rounded, y0_rounded, x1_rounded, y1_rounded, color, filled);
    drawLine(x1_rounded, y1_rounded, x2_rounded, y2_rounded, color, filled);
    drawLine(x2_rounded, y2_rounded, x3_rounded, y3_rounded, color, filled);
    drawLine(x3_rounded, y3_rounded, x0_rounded, y0_rounded, color, filled);

	if (filled) {
        // Calculate the number of lines needed for the filling
        uint16_t numLines = (uint16_t)round(sqrt(halfWidth * halfWidth + halfHeight * halfHeight) * 2);

        for (uint16_t i = 0; i <= numLines; i++) {
            double t = (double)i / numLines;
            double xStart = (1 - t) * x0 + t * x3;
            double yStart = (1 - t) * y0 + t * y3;
            double xEnd = (1 - t) * x1 + t * x2;
            double yEnd = (1 - t) * y1 + t * y2;

            drawLine((uint16_t)round(xStart), (uint16_t)round(yStart), (uint16_t)round(xEnd), (uint16_t)round(yEnd), color, filled);
        }
	}
}

void GPGFX_TinySSD1306::drawPolygon(uint16_t x, uint16_t y, uint16_t radius, uint16_t sides, uint32_t color, uint8_t filled, double rotation) {
    // Calculate the angle increment between each vertex
    double angleIncrement = 2 * M_PI / sides;

    // Calculate vertices
    uint16_t xVertices[sides];
    uint16_t yVertices[sides];
    for (int i = 0; i < sides; i++) {
        double angle = i * angleIncrement + rotation;
        xVertices[i] = x + round(radius * cos(angle));
        yVertices[i] = y + round(radius * sin(angle));
    }

    // Draw lines between vertices
    for (int i = 0; i < sides - 1; i++) {
        drawLine(xVertices[i], yVertices[i], xVertices[i + 1], yVertices[i + 1], color, false);
    }
    drawLine(xVertices[sides - 1], yVertices[sides - 1], xVertices[0], yVertices[0], color, false);

    if (filled) {
        // Find the minimum and maximum y coordinates to scan
        uint16_t minY = yVertices[0], maxY = yVertices[0];
        for (int i = 1; i < sides; i++) {
            if (yVertices[i] < minY) minY = yVertices[i];
            if (yVertices[i] > maxY) maxY = yVertices[i];
        }

        // Scan horizontally and draw lines between intersections
        for (int scanY = minY + 1; scanY < maxY; scanY++) {
            int intersections = 0;
            double intersectPoints[sides];

            for (int i = 0; i < sides; i++) {
                int next = (i + 1) % sides;
                if ((yVertices[i] < scanY && yVertices[next] >= scanY) || (yVertices[next] < scanY && yVertices[i] >= scanY)) {
                    intersectPoints[intersections++] = xVertices[i] + (scanY - yVertices[i]) * (xVertices[next] - xVertices[i]) / (yVertices[next] - yVertices[i]);
                }
            }

            // Sort the intersection points by x coordinate
            for (int i = 0; i < intersections - 1; i++) {
                for (int j = 0; j < intersections - i - 1; j++) {
                    if (intersectPoints[j] > intersectPoints[j + 1]) {
                        double temp = intersectPoints[j];
                        intersectPoints[j] = intersectPoints[j + 1];
                        intersectPoints[j + 1] = temp;
                    }
                }
            }

            // Draw lines between pairs of intersection points
            for (int i = 0; i < intersections; i += 2) {
                drawLine(intersectPoints[i], scanY, intersectPoints[i + 1], scanY, color, false);
            }
        }
    }
}

void GPGFX_TinySSD1306::drawSprite(uint8_t* image, uint16_t width, uint16_t height, uint16_t pitch, uint16_t x, uint16_t y, uint8_t priority, double scale) {
	uint8_t spriteByte;
	uint8_t spriteBit;
	uint8_t spriteX, spriteY;
	uint8_t color;

	for (uint16_t scaledY = 0; scaledY < height * scale; ++scaledY) {
		for (uint16_t scaledX = 0; scaledX < width * scale; ++scaledX) {
			spriteX = scaledX / scale;
			spriteY = scaledY / scale;
			
			spriteBit = spriteX % 8;
			spriteByte = image[(spriteY * ((width + 7) / 8)) + (spriteX / 8)];
			color = ((spriteByte >> (7 - spriteBit)) & 0x01);
			
			drawPixel(x + scaledX, y + scaledY, color);
		}
	}
}

void GPGFX_TinySSD1306::drawBuffer(uint8_t* pBuffer) {
	uint16_t bufferSize = MAX_SCREEN_SIZE;
	uint8_t buffer[bufferSize+1] = {SET_START_LINE};

	int result = -1;
	
    if (this->screenType == ScreenAlternatives::SCREEN_132x64) {
        uint16_t x = 0;
        uint16_t y = 0;
        for (y = 0; y < (MAX_SCREEN_HEIGHT/8); y++) {
            sendCommand(0xB0 + y);
            sendCommand(x & 0x0F);
            sendCommand(0x10 | (x >> 4));
        
            if (pBuffer == NULL) {
                memcpy(&buffer[1],&frameBuffer[y*MAX_SCREEN_WIDTH],MAX_SCREEN_WIDTH);
            } else {
                memcpy(&buffer[1],&pBuffer[y*MAX_SCREEN_WIDTH],MAX_SCREEN_WIDTH);
            }
        
            result = _options.i2c->write(_options.address, buffer, MAX_SCREEN_WIDTH+3, false);
        }
    } else {
        sendCommand(CommandOps::PAGE_ADDRESS);
        sendCommand(0x00);
        sendCommand(0x07);
        sendCommand(CommandOps::COLUMN_ADDRESS);
        sendCommand(0x00);
        sendCommand(0x7F);

        if (pBuffer == NULL) {
            memcpy(&buffer[1],frameBuffer,bufferSize);
        } else {
            memcpy(&buffer[1],pBuffer,bufferSize);
        }
        result = _options.i2c->write(_options.address, buffer, sizeof(buffer), false);
    }

	if (framePage < MAX_SCREEN_HEIGHT/8) {
		framePage++;
	} else {
		framePage = 0;
	}
}

void GPGFX_TinySSD1306::sendCommand(uint8_t command){ 
	uint8_t commandData[] = {0x00, command};
	sendCommands(commandData, 2);
}

void GPGFX_TinySSD1306::sendCommands(uint8_t* commands, uint16_t length){ 
	int result = _options.i2c->write(_options.address, commands, length, false);
}
