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

void GPGFX_TinySSD1306::setPower(bool isPowered) {
	_isPowered = isPowered;
}

void GPGFX_TinySSD1306::clear() {
	memset(frameBuffer, 0, MAX_SCREEN_SIZE);
}

void GPGFX_TinySSD1306::drawPixel(uint8_t x, uint8_t y, uint32_t color) {
	uint16_t row, bitIndex;

	if ((x<MAX_SCREEN_WIDTH) and (y<MAX_SCREEN_HEIGHT))
	{
		row=((y/8)*128)+x;
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

	for (uint8_t charIndex = 0; charIndex < text.size(); charIndex++) {
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

void GPGFX_TinySSD1306::drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t filled) {
    //printf("Rect %d, %d, %d, %d, %d, %d\n", x, y, width, height, color, filled);
	drawLine(x, y, x, height, color, filled);
	drawLine(x, y, width, y, color, filled);
	drawLine(width, height, x, height, color, filled);
	drawLine(width, height, width, y, color, filled);

	if (filled) {
		for (uint8_t i = 1; i < (height-y); i++) {
			drawLine(x, y+i, width, y+i, color, filled);
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

void GPGFX_TinySSD1306::drawSprite(uint8_t* image, uint16_t width, uint16_t height, uint16_t pitch, uint16_t x, uint16_t y, uint8_t priority) {
	uint8_t spriteByte;
	uint8_t spriteBit;
	uint8_t spriteX, spriteY;
	uint8_t color;

	for (spriteY = 0; spriteY < height; spriteY++) {
		for (spriteX = 0; spriteX < width; spriteX++) {
			spriteBit = spriteX % 8;
			//spriteByte = image[(spriteY * (width / 8)) + (spriteX / 8)];
			spriteByte = image[(spriteY * ((width + 7) / 8)) + (spriteX / 8)];
			color = ((spriteByte >> (7 - spriteBit)) & 0x01);

			drawPixel(x+spriteX, y+spriteY, color);
		}
	}
}

void GPGFX_TinySSD1306::drawBuffer(uint8_t* pBuffer) {
	uint16_t bufferSize = MAX_SCREEN_SIZE;
	uint8_t buffer[bufferSize+1] = {SET_START_LINE};

	int result = -1;
	
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
