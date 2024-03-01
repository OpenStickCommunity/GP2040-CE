//
// obd.inl
// Drawing code for OneBitDisplay library
//
const uint8_t *ucSmallFont;
const uint8_t *ucFont;
const uint8_t *ucBigFont;

void obdSetDCMode(OBDISP *pOBD, int iMode);
void InvertBytes(uint8_t *pData, uint8_t bLen);
void SPI_BitBang(OBDISP *pOBD, uint8_t *pData, int iLen, uint8_t iMOSIPin, uint8_t iSCKPin);

#define MAX_CACHE 192
static uint8_t u8Cache[MAX_CACHE]; // for faster character drawing
static volatile uint8_t u8End = 0;

static void obdCachedFlush(OBDISP *pOBD, int bRender)
{
	if (u8End > 0)
	{
		obdWriteDataBlock(pOBD, u8Cache, u8End, bRender);
		u8End = 0;
	}
} /* obdCachedFlush() */

static void obdCachedWrite(OBDISP *pOBD, uint8_t *pData, uint8_t u8Len, int bRender)
{
	if (u8End + u8Len > MAX_CACHE) // need to flush it
	{
		obdCachedFlush(pOBD, bRender); // write the old data
	}
	memcpy(&u8Cache[u8End], pData, u8Len);
	u8End += u8Len;

} /* obdCachedWrite() */

static void _I2CWrite(OBDISP *pOBD, unsigned char *pData, int iLen)
{
	if (pOBD->com_mode == COM_SPI) // we're writing to SPI, treat it differently
	{
		if (pOBD->iDCPin != 0xff)
		{
			gpio_put(pOBD->iDCPin, (pData[0] == 0) ? LOW : HIGH); // data versus command
			gpio_put(pOBD->iCSPin, LOW);
		}

		spi_write_blocking(pOBD->spi->getController(), &pData[1], iLen - 1);

		if (pOBD->type < SHARP_144x168)
			gpio_put(pOBD->iCSPin, HIGH);
	}
	else // must be I2C
	{
		if (pOBD->bWire && iLen > 32) // Hardware I2C has write length limits
		{
			iLen--;            // don't count the 0x40 byte the first time through
			while (iLen >= 31) // max 31 data byes + data introducer
			{
				pOBD->i2c->write(pOBD->oled_addr, pData, 32);
				iLen -= 31;
				pData += 31;
				pData[0] = 0x40;
			}
			if (iLen)
				iLen++; // If remaining data, add the last 0x40 byte
		}
		if (iLen) // if any data remaining
		{
			pOBD->i2c->write(pOBD->oled_addr, pData, iLen);
		}
	} // I2C
} /* _I2CWrite() */

// Send a single byte command to the OLED controller
void obdWriteCommand(OBDISP *pOBD, unsigned char c)
{
	unsigned char buf[4];

	if (pOBD->com_mode == COM_I2C)
	{                // I2C device
		buf[0] = 0x00; // command introducer
		buf[1] = c;
		_I2CWrite(pOBD, buf, 2);
	}
	else
	{ // must be SPI
		obdSetDCMode(pOBD, MODE_COMMAND);
		gpio_put(pOBD->iCSPin, LOW);

		if (pOBD->iMOSIPin == 0xff)
			spi_write_blocking(pOBD->spi->getController(), &c, 1);
		else
			SPI_BitBang(pOBD, &c, 1, pOBD->iMOSIPin, pOBD->iCLKPin);

		gpio_put(pOBD->iCSPin, HIGH);
		obdSetDCMode(pOBD, MODE_DATA);
	}
} /* obdWriteCommand() */

//
// Byte operands for compressing the data
// The first 2 bits are the type, followed by the counts
#define OP_MASK 0xc0
#define OP_SKIPCOPY 0x00
#define OP_COPYSKIP 0x40
#define OP_REPEATSKIP 0x80
#define OP_REPEAT 0xc0
//
// Write a block of flash memory to the display
//
void oledWriteFlashBlock(OBDISP *pOBD, uint8_t *s, int iLen)
{
	int j;
	int iWidthMask = pOBD->width - 1;
	int iSizeMask = ((pOBD->width * pOBD->height) / 8) - 1;
	int iWidthShift = (pOBD->width == 128) ? 7 : 6; // assume 128 or 64 wide
	uint8_t ucTemp[128];

	while (((pOBD->iScreenOffset & iWidthMask) + iLen) >= pOBD->width) // if it will hit the page end
	{
		j = pOBD->width - (pOBD->iScreenOffset & iWidthMask); // amount we can write in one shot
		memcpy(ucTemp, s, j);
		obdWriteDataBlock(pOBD, ucTemp, j, 1);
		s += j;
		iLen -= j;
		pOBD->iScreenOffset = (pOBD->iScreenOffset + j) & iSizeMask;
		obdSetPosition(pOBD, pOBD->iScreenOffset & iWidthMask, (pOBD->iScreenOffset >> iWidthShift), 1);
	} // while it needs some help
	memcpy(ucTemp, s, iLen);
	obdWriteDataBlock(pOBD, ucTemp, iLen, 1);
	pOBD->iScreenOffset = (pOBD->iScreenOffset + iLen) & iSizeMask;
} /* oledWriteFlashBlock() */

//
// Create a virtual display of any size
// The memory buffer must be provided at the time of creation
//
void obdCreateVirtualDisplay(OBDISP *pOBD, int width, int height, uint8_t *buffer)
{
	if (pOBD != NULL && buffer != NULL)
	{
		pOBD->width = width;
		pOBD->height = height;
		pOBD->type = LCD_VIRTUAL;
		pOBD->ucScreen = buffer;
		pOBD->iCursorX = pOBD->iCursorY = 0;
		pOBD->iScreenOffset = 0;
	}
} /* obdCreateVirtualDisplay() */
//
// Scroll the internal buffer by 1 scanline (up/down)
// width is in pixels, lines is group of 8 rows
//
int obdScrollBuffer(OBDISP *pOBD, int iStartCol, int iEndCol, int iStartRow, int iEndRow, int bUp)
{
	uint8_t b, *s;
	int col, row;
	int iPitch;

	if (iStartCol < 0 || iStartCol >= pOBD->width || iEndCol < 0 || iEndCol > pOBD->width || iStartCol > iEndCol) // invalid
		return -1;
	if (iStartRow < 0 || iStartRow >= (pOBD->height / 8) || iEndRow < 0 || iEndRow >= (pOBD->height / 8) || iStartRow > iEndRow)
		return -1;
	iPitch = pOBD->width;
	if (bUp)
	{
		for (row = iStartRow; row <= iEndRow; row++)
		{
			s = &pOBD->ucScreen[(row * iPitch) + iStartCol];
			for (col = iStartCol; col <= iEndCol; col++)
			{
				b = *s;
				b >>= 1; // scroll pixels 'up'
				if (row < iEndRow)
					b |= (s[iPitch] << 7); // capture pixel of row below, except for last row
				*s++ = b;
			} // for col
		}   // for row
	}     // up
	else  // down
	{
		for (row = iEndRow; row >= iStartRow; row--)
		{
			s = &pOBD->ucScreen[(row * iPitch) + iStartCol];
			for (col = iStartCol; col <= iEndCol; col++)
			{
				b = *s;
				b <<= 1; // scroll down
				if (row > iStartRow)
					b |= (s[-iPitch] >> 7); // capture pixel of row above
				*s++ = b;
			} // for col
		}   // for row
	}
	return 0;
} /* obdScrollBuffer() */
//
// Send commands to position the "cursor" (aka memory write address)
// to the given row and column
//
void obdSetPosition(OBDISP *pOBD, int x, int y, int bRender)
{
	unsigned char buf[4];
	int iPitch = pOBD->width;

	obdCachedFlush(pOBD, bRender); // flush any cached data first

	pOBD->iScreenOffset = (y * iPitch) + x;

	if (pOBD->type == LCD_VIRTUAL || pOBD->type >= SHARP_144x168)
		return; // nothing to do
	if (!bRender)
		return; // don't send the commands to the OLED if we're not rendering the graphics now
	if (pOBD->type == LCD_NOKIA5110)
	{
		obdWriteCommand(pOBD, 0x40 | y);
		obdWriteCommand(pOBD, 0x80 | x);
		return;
	}
	if (pOBD->type == OLED_64x32) // visible display starts at column 32, row 4
	{
		x += 32;             // display is centered in VRAM, so this is always true
		if (pOBD->flip == 0) // non-flipped display starts from line 4
			y += 4;
	}
	else if (pOBD->type == OLED_132x64) // SH1106 has 128 pixels centered in 132
	{
		x += 2;
	}
	else if (pOBD->type == OLED_96x16) // visible display starts at line 2
	{                                  // mapping is a bit strange on the 96x16 OLED
		if (pOBD->flip)
			x += 32;
		else
			y += 2;
	}
	else if (pOBD->type == OLED_72x40) // starts at x=28,y=3
	{
		x += 28;
		if (!pOBD->flip)
		{
			y += 3;
		}
	}
	if (pOBD->com_mode == COM_I2C)
	{                           // I2C device
		buf[0] = 0x00;            // command introducer
		buf[1] = 0xb0 | y;        // set page to Y
		buf[2] = x & 0xf;         // lower column address
		buf[3] = 0x10 | (x >> 4); // upper column addr
		_I2CWrite(pOBD, buf, 4);
	}
	else
	{                                          // SPI mode
		obdWriteCommand(pOBD, 0xb0 | y);         // set Y
		obdWriteCommand(pOBD, 0x10 | (x >> 4));  // set X MSB
		obdWriteCommand(pOBD, 0x00 | (x & 0xf)); // set X LSB
	}
} /* obdSetPosition() */

//
// Write a block of pixel data to the OLED
// Length can be anything from 1 to 1024 (whole display)
//
void obdWriteDataBlock(OBDISP *pOBD, unsigned char *ucBuf, int iLen, int bRender)
{
	unsigned char ucTemp[196];
	int iPitch, iBufferSize;

	iPitch = pOBD->width;
	iBufferSize = iPitch * (pOBD->height / 8);

	// Keep a copy in local buffer
	if (pOBD->ucScreen && (iLen + pOBD->iScreenOffset) <= iBufferSize)
	{
		memcpy(&pOBD->ucScreen[pOBD->iScreenOffset], ucBuf, iLen);
		pOBD->iScreenOffset += iLen;
		// wrap around ?
		if (pOBD->iScreenOffset >= iBufferSize)
			pOBD->iScreenOffset -= iBufferSize;
	}
	if (pOBD->type == LCD_VIRTUAL || pOBD->type >= SHARP_144x168)
		return; // nothing else to do
						// Copying the data has the benefit in SPI mode of not letting
						// the original data get overwritten by the SPI.transfer() function
	if (bRender)
	{
		if (pOBD->com_mode == COM_SPI) // SPI/Bit Bang
		{
			gpio_put(pOBD->iCSPin, LOW);

			if (pOBD->iMOSIPin != 0xff) // Bit Bang
				SPI_BitBang(pOBD, ucBuf, iLen, pOBD->iMOSIPin, pOBD->iCLKPin);
			else
				spi_write_blocking(pOBD->spi->getController(), ucBuf, iLen);

			gpio_put(pOBD->iCSPin, HIGH);
		}
		else // I2C
		{
			ucTemp[0] = 0x40; // data command
			memcpy(&ucTemp[1], ucBuf, iLen);
			_I2CWrite(pOBD, ucTemp, iLen + 1);
		}
	}
} /* obdWriteDataBlock() */
//
// Write a repeating byte to the display
//
void obdRepeatByte(OBDISP *pOBD, uint8_t b, int iLen)
{
	int j;
	int iWidthMask = pOBD->width - 1;
	int iWidthShift = (pOBD->width == 128) ? 7 : 6; // assume 128 or 64 pixels wide
	int iSizeMask = ((pOBD->width * pOBD->height) / 8) - 1;
	uint8_t ucTemp[128];

	memset(ucTemp, b, (iLen > 128) ? 128 : iLen);
	while (((pOBD->iScreenOffset & iWidthMask) + iLen) >= pOBD->width) // if it will hit the page end
	{
		j = pOBD->width - (pOBD->iScreenOffset & iWidthMask); // amount we can write in one shot
		obdWriteDataBlock(pOBD, ucTemp, j, 1);
		iLen -= j;
		pOBD->iScreenOffset = (pOBD->iScreenOffset + j) & iSizeMask;
		obdSetPosition(pOBD, pOBD->iScreenOffset & iWidthMask, (pOBD->iScreenOffset >> iWidthShift), 1);
	} // while it needs some help
	obdWriteDataBlock(pOBD, ucTemp, iLen, 1);
	pOBD->iScreenOffset += iLen;
} /* obdRepeatByte() */

//
// Play a frame of animation data
// The animation data is assumed to be encoded for a full frame of the display
// Given the pointer to the start of the compressed data,
// it returns the pointer to the start of the next frame
// Frame rate control is up to the calling program to manage
// When it finishes the last frame, it will start again from the beginning
//
uint8_t *obdPlayAnimFrame(OBDISP *pOBD, uint8_t *pAnimation, uint8_t *pCurrent, int iLen)
{
	uint8_t *s;
	int i, j;
	unsigned char b, bCode;
	int iBufferSize = (pOBD->width * pOBD->height) / 8; // size in bytes of the display devce
	int iWidthMask, iWidthShift;

	iWidthMask = pOBD->width - 1;
	iWidthShift = (pOBD->width == 128) ? 7 : 6; // 128 or 64 pixels wide
	if (pCurrent == NULL || pCurrent > pAnimation + iLen)
		return NULL; // invalid starting point

	s = (uint8_t *)pCurrent; // start of animation data
	i = 0;
	obdSetPosition(pOBD, 0, 0, 1);
	while (i < iBufferSize) // run one frame
	{
		bCode = (char)*(s++);
		switch (bCode & OP_MASK) // different compression types
		{
		case OP_SKIPCOPY:           // skip/copy
			if (bCode == OP_SKIPCOPY) // big skip
			{
				b = (char)*(s++);
				i += b + 1;
				obdSetPosition(pOBD, i & iWidthMask, (i >> iWidthShift), 1);
			}
			else // skip/copy
			{
				if (bCode & 0x38)
				{
					i += ((bCode & 0x38) >> 3); // skip amount
					obdSetPosition(pOBD, i & iWidthMask, (i >> iWidthShift), 1);
				}
				if (bCode & 7)
				{
					oledWriteFlashBlock(pOBD, s, bCode & 7);
					s += (bCode & 7);
					i += bCode & 7;
				}
			}
			break;
		case OP_COPYSKIP:           // copy/skip
			if (bCode == OP_COPYSKIP) // big copy
			{
				b = (char)*(s++);
				j = b + 1;
				oledWriteFlashBlock(pOBD, s, j);
				s += j;
				i += j;
			}
			else
			{
				j = ((bCode & 0x38) >> 3);
				if (j)
				{
					oledWriteFlashBlock(pOBD, s, j);
					s += j;
					i += j;
				}
				if (bCode & 7)
				{
					i += (bCode & 7); // skip
					obdSetPosition(pOBD, i & iWidthMask, (i >> iWidthShift), 1);
				}
			}
			break;
		case OP_REPEATSKIP:        // repeat/skip
			j = (bCode & 0x38) >> 3; // repeat count
			b = (char)*(s++);
			obdRepeatByte(pOBD, b, j);
			i += j;
			if (bCode & 7)
			{
				i += (bCode & 7); // skip amount
				obdSetPosition(pOBD, i & iWidthMask, (i >> iWidthShift), 1);
			}
			break;

		case OP_REPEAT:
			j = (bCode & 0x3f) + 1;
			b = (char)*(s++);
			obdRepeatByte(pOBD, b, j);
			i += j;
			break;
		}                         // switch on code type
	}                           // while rendering a frame
	if (s >= pAnimation + iLen) // we've hit the end, restart from the beginning
		s = pAnimation;
	return s; // return pointer to start of next frame
} /* obdPlayAnimFrame() */
//
// Draw a sprite of any size in any position
// If it goes beyond the left/right or top/bottom edges
// it's trimmed to show the valid parts
// This function requires a back buffer to be defined
// The priority color (0 or 1) determines which color is painted
// when a 1 is encountered in the source image.
//
void obdDrawSprite(OBDISP *pOBD, uint8_t *pSprite, int cx, int cy, int iPitch, int x, int y, uint8_t iPriority)
{
	int tx, ty, dx, dy, iStartX;
	uint8_t *s, *d, uc, pix, ucSrcMask, ucDstMask;
	int iLocalPitch;

	iLocalPitch = pOBD->width;

	if (x + cx < 0 || y + cy < 0 || x >= pOBD->width || y >= pOBD->height || pOBD->ucScreen == NULL)
		return;  // no backbuffer or out of bounds
	dy = y;    // destination y
	if (y < 0) // skip the invisible parts
	{
		cy += y;
		y = -y;
		pSprite += (y * iPitch);
		dy = 0;
	}
	if (y + cy > pOBD->height)
		cy = pOBD->height - y;
	iStartX = 0;
	dx = x;
	if (x < 0)
	{
		cx += x;
		x = -x;
		iStartX = x;
		dx = 0;
	}
	if (x + cx > pOBD->width)
		cx = pOBD->width - x;
	for (ty = 0; ty < cy; ty++)
	{
		s = &pSprite[iStartX >> 3];
		d = &pOBD->ucScreen[(dy >> 3) * iLocalPitch + dx];
		ucSrcMask = 0x80 >> (iStartX & 7);
		pix = *s++;
		ucDstMask = 1 << (dy & 7);
		if (iPriority) // priority color is 1
		{
			for (tx = 0; tx < cx; tx++)
			{
				uc = d[0];
				if (pix & ucSrcMask) // set pixel in source, set it in dest
					d[0] = (uc | ucDstMask);
				d++; // next pixel column
				ucSrcMask >>= 1;
				if (ucSrcMask == 0) // read next byte
				{
					ucSrcMask = 0x80;
					pix = *s++;
				}
			} // for tx
		}   // priorty color 1
		else
		{
			for (tx = 0; tx < cx; tx++)
			{
				uc = d[0];
				if (pix & ucSrcMask) // clr pixel in source, clr it in dest
					d[0] = (uc & ~ucDstMask);
				d++; // next pixel column
				ucSrcMask >>= 1;
				if (ucSrcMask == 0) // read next byte
				{
					ucSrcMask = 0x80;
					pix = *s++;
				}
			} // for tx
		}   // priority color 0
		dy++;
		pSprite += iPitch;
	} // for ty
} /* obdDrawSprite() */
//
// Draw a 16x16 tile in any of 4 rotated positions
// Assumes input image is laid out like "normal" graphics with
// the MSB on the left and 2 bytes per line
// On AVR, the source image is assumed to be in FLASH memory
// The function can draw the tile on byte boundaries, so the x value
// can be from 0 to width-16 and y can be from 0 to (height/8)-2
//
void obdDrawTile(OBDISP *pOBD, const uint8_t *pTile, int x, int y, int iRotation, int bInvert, int bRender)
{
	uint8_t ucTemp[32]; // prepare LCD data here
	uint8_t i, j, k, iOffset, ucMask, uc, ucPixels;
	uint8_t bFlipX = 0, bFlipY = 0;

	if (x < 0 || y < 0 || y > (pOBD->height / 8) - 2 || x > pOBD->width - 16)
		return; // out of bounds
	if (pTile == NULL)
		return; // bad pointer; really? :(
	if (iRotation == ANGLE_180 || iRotation == ANGLE_270 || iRotation == ANGLE_FLIPX)
		bFlipX = 1;
	if (iRotation == ANGLE_180 || iRotation == ANGLE_270 || iRotation == ANGLE_FLIPY)
		bFlipY = 1;

	memset(ucTemp, 0, sizeof(ucTemp)); // we only set white pixels, so start from black
	if (iRotation == ANGLE_0 || iRotation == ANGLE_180 || iRotation == ANGLE_FLIPX || iRotation == ANGLE_FLIPY)
	{
		for (j = 0; j < 16; j++) // y
		{
			for (i = 0; i < 16; i += 8) // x
			{
				ucPixels = *((uint8_t *)pTile++);
				ucMask = 0x80; // MSB is the first source pixel
				for (k = 0; k < 8; k++)
				{
					if (ucPixels & ucMask) // translate the pixel
					{
						if (bFlipY)
							uc = 0x80 >> (j & 7);
						else
							uc = 1 << (j & 7);
						iOffset = i + k;
						if (bFlipX)
							iOffset = 15 - iOffset;
						iOffset += (j & 8) << 1; // top/bottom half of output
						if (bFlipY)
							iOffset ^= 16;
						ucTemp[iOffset] |= uc;
					}
					ucMask >>= 1;
				} // for k
			}   // for i
		}     // for j
	}
	else // rotated 90/270
	{
		for (j = 0; j < 16; j++) // y
		{
			for (i = 0; i < 16; i += 8) // x
			{
				ucPixels = *((uint8_t *)pTile++);
				ucMask = 0x80; // MSB is the first source pixel
				for (k = 0; k < 8; k++)
				{
					if (ucPixels & ucMask) // translate the pixel
					{
						if (bFlipY)
							uc = 0x80 >> k;
						else
							uc = 1 << k;
						iOffset = 15 - j;
						if (bFlipX)
							iOffset = 15 - iOffset;
						iOffset += i << 1; // top/bottom half of output
						if (bFlipY)
							iOffset ^= 16;
						ucTemp[iOffset] |= uc;
					}
					ucMask >>= 1;
				} // for k
			}   // for i
		}     // for j
	}
	if (bInvert)
		InvertBytes(ucTemp, 32);
	// Send the data to the display
	obdSetPosition(pOBD, x, y, bRender);
	obdWriteDataBlock(pOBD, ucTemp, 16, bRender); // top half
	obdSetPosition(pOBD, x, y + 1, bRender);
	obdWriteDataBlock(pOBD, &ucTemp[16], 16, bRender); // bottom half
} /* obdDrawTile() */

// Set (or clear) an individual pixel
// The local copy of the frame buffer is used to avoid
// reading data from the display controller
int obdSetPixel(OBDISP *pOBD, int x, int y, unsigned char ucColor, int bRender)
{
	int i;
	unsigned char uc, ucOld;
	int iPitch, iSize;

	iPitch = pOBD->width;
	iSize = iPitch * (pOBD->height / 8);

	i = ((y >> 3) * iPitch) + x;
	if (i < 0 || i > iSize - 1) // off the screen
		return -1;
	obdSetPosition(pOBD, x, y >> 3, bRender);

	if (pOBD->ucScreen)
		uc = ucOld = pOBD->ucScreen[i];
	else if (pOBD->type == OLED_132x64 || pOBD->type == OLED_128x128) // SH1106/SH1107 can read data
	{
		uint8_t ucTemp[3];
		ucTemp[0] = 0x80; // one command
		ucTemp[1] = 0xE0; // read_modify_write
		ucTemp[2] = 0xC0; // one data
		_I2CWrite(pOBD, ucTemp, 3);

		// read a dummy byte followed by the data byte we want
		pOBD->i2c->read(pOBD->oled_addr, ucTemp, 2);
		uc = ucOld = ucTemp[1]; // first byte is garbage
	}
	else
		uc = ucOld = 0;

	uc &= ~(0x1 << (y & 7));
	if (ucColor)
	{
		uc |= (0x1 << (y & 7));
	}
	if (uc != ucOld) // pixel changed
	{
		//    obdSetPosition(x, y>>3);
		if (pOBD->ucScreen)
		{
			obdWriteDataBlock(pOBD, &uc, 1, bRender);
			pOBD->ucScreen[i] = uc;
		}
		else if (pOBD->type == OLED_132x64 || pOBD->type == OLED_128x128) // end the read_modify_write operation
		{
			uint8_t ucTemp[4];
			ucTemp[0] = 0xc0; // one data
			ucTemp[1] = uc;   // actual data
			ucTemp[2] = 0x80; // one command
			ucTemp[3] = 0xEE; // end read_modify_write operation
			_I2CWrite(pOBD, ucTemp, 4);
		}
	}
	return 0;
} /* obdSetPixel() */

//
// Invert font data
//
void InvertBytes(uint8_t *pData, uint8_t bLen)
{
	uint8_t i;
	for (i = 0; i < bLen; i++)
	{
		*pData = ~(*pData);
		pData++;
	}
} /* InvertBytes() */

//
// Load a 1-bpp Windows bitmap
// Pass the pointer to the beginning of the BMP file
// First pass version assumes a full screen bitmap
//
int obdLoadBMP(OBDISP *pOBD, uint8_t *pBMP, int dx, int dy, int bInvert)
{
	int16_t i16, cx, cy;
	int iOffBits; // offset to bitmap data
	int iPitch, iDestPitch;
	uint8_t x, y, b, *s, *d;
	uint8_t dst_mask, src_mask;
	uint8_t bFlipped = false;

	iDestPitch = pOBD->width;
	i16 = *((uint16_t *)pBMP);
	if (i16 != 0x4d42) // must start with 'BM'
		return -1;       // not a BMP file
	cx = *((uint16_t *)(pBMP + 18));
	if (cx + dx > pOBD->width) // must fit on the display
		return -1;
	cy = *((uint16_t *)(pBMP + 22));
	if (cy < 0)
		cy = -cy;
	else
		bFlipped = true;
	if (cy + dy > pOBD->height) // must fit on the display
		return -1;
	i16 = *((uint16_t *)(pBMP + 28));
	if (i16 != 1) // must be 1 bit per pixel
		return -1;
	iOffBits = *((uint16_t *)(pBMP + 10));
	iPitch = (((cx + 7) >> 3) + 3) & 0xfffc; // must be DWORD aligned
	if (bFlipped)
	{
		iOffBits += ((cy - 1) * iPitch); // start from bottom
		iPitch = -iPitch;
	}

	for (y = 0; y < cy; y++)
	{
		dst_mask = 1 << ((y + dy) & 7);
		if (pOBD->ucScreen)
			d = &pOBD->ucScreen[(((y + dy) >> 3) * iDestPitch) + dx];
		else
		{
			d = u8Cache;
			if ((y & 7) == 0)
				memset(u8Cache, 0, sizeof(u8Cache));
		}
		s = &pBMP[iOffBits + (y * iPitch)];
		src_mask = 0;
		for (x = 0; x < cx; x++)
		{
			if (src_mask == 0) // need to load the next byte
			{
				b = *((uint8_t *)s++);
				src_mask = 0x80; // MSB on left
			}
			if (b & src_mask)
			{
				if (bInvert)
					d[0] &= ~dst_mask;
				else
					d[0] |= dst_mask;
			}
			else
			{
				if (bInvert)
					d[0] |= dst_mask;
				else
					d[0] &= ~dst_mask;
			}
			d++;
			src_mask >>= 1;
		}                                                            // for x
		if (pOBD->ucScreen == NULL && ((y & 7) == 7 || y == cy - 1)) // dump to LCD
		{
			obdSetPosition(pOBD, dx, (y + dy) >> 3, 1);
			obdWriteDataBlock(pOBD, u8Cache, cx, 1);
		}
	} // for y
	return 0;
} /* obdLoadBMP() */
//
// Set the current cursor position
// The column represents the pixel column (0-127)
// The row represents the text row (0-7)
//
void obdSetCursor(OBDISP *pOBD, int x, int y)
{
	pOBD->iCursorX = x;
	pOBD->iCursorY = y;
} /* obdSetCursor() */
//
// Turn text wrap on or off for the oldWriteString() function
//
void obdSetTextWrap(OBDISP *pOBD, int bWrap)
{
	pOBD->wrap = bWrap;
} /* obdSetTextWrap() */
//
// Draw a string with a fractional scale in both dimensions
// the scale is a 16-bit integer with and 8-bit fraction and 8-bit mantissa
// To draw at 1x scale, set the scale factor to 256. To draw at 2x, use 512
// The output must be drawn into a memory buffer, not directly to the display
//
int obdScaledString(OBDISP *pOBD, int x, int y, char *szMsg, int iSize, int bInvert, int iXScale, int iYScale, int iRotation)
{
	uint32_t row, col, dx, dy;
	uint32_t sx, sy;
	uint8_t c, uc, color, *d;
	const uint8_t *s;
	uint8_t ucTemp[16];
	int tx, ty, bit, iFontOff;
	int iPitch;
	int iFontWidth;

	if (iXScale == 0 || iYScale == 0 || szMsg == NULL || pOBD == NULL || pOBD->ucScreen == NULL || x < 0 || y < 0 || x >= pOBD->width - 1 || y >= pOBD->height - 1)
		return -1; // invalid display structure
	if (iSize != FONT_8x8 && iSize != FONT_6x8)
		return -1; // only on the small fonts (for now)
	iFontWidth = (iSize == FONT_6x8) ? 6 : 8;
	s = (iSize == FONT_6x8) ? ucSmallFont : ucFont;
	iPitch = pOBD->width;
	dx = (iFontWidth * iXScale) >> 8; // width of each character
	dy = (8 * iYScale) >> 8;          // height of each character
	sx = 65536 / iXScale;             // turn the scale into an accumulator value
	sy = 65536 / iYScale;
	while (*szMsg)
	{
		c = *szMsg++; // debug - start with normal font
		iFontOff = (int)(c - 32) * (iFontWidth - 1);
		// we can't directly use the pointer to FLASH memory, so copy to a local buffer
		ucTemp[0] = 0; // first column is blank
		memcpy(&ucTemp[1], &s[iFontOff], iFontWidth - 1);
		if (bInvert)
			InvertBytes(ucTemp, iFontWidth);
		col = 0;
		for (tx = 0; tx < (int)dx; tx++)
		{
			row = 0;
			uc = ucTemp[col >> 8];
			for (ty = 0; ty < (int)dy; ty++)
			{
				int nx = 0, ny = 0;
				bit = row >> 8;
				color = (uc & (1 << bit)); // set or clear the pixel
				switch (iRotation)
				{
				case ROT_0:
					nx = x + tx;
					ny = y + ty;
					break;
				case ROT_90:
					nx = x - ty;
					ny = y + tx;
					break;
				case ROT_180:
					nx = x - tx;
					ny = y - ty;
					break;
				case ROT_270:
					nx = x + ty;
					ny = y - tx;
					break;
				} // switch on rotation direction
				// plot the pixel if it's within the image boundaries
				if (nx >= 0 && ny >= 0 && nx < pOBD->width && ny < pOBD->height)
				{
					d = &pOBD->ucScreen[(ny >> 3) * iPitch + nx];
					if (color)
						d[0] |= (1 << (ny & 7));
					else
						d[0] &= ~(1 << (ny & 7));
				}
				row += sy; // add fractional increment to source row of character
			}            // for ty
			col += sx;   // add fractional increment to source column
		}              // for tx
		// update the 'cursor' position
		switch (iRotation)
		{
		case ROT_0:
			x += dx;
			break;
		case ROT_90:
			y += dx;
			break;
		case ROT_180:
			x -= dx;
			break;
		case ROT_270:
			y -= dx;
			break;
		} // switch on rotation
	}   // while (*szMsg)
	return 0;
} /* obdScaledString() */

//
// Draw a string of normal (8x8), small (6x8) or large (16x32) characters
// At the given col+row
//
int obdWriteString(OBDISP *pOBD, int iScroll, int x, int y, char *szMsg, int iSize, int bInvert, int bRender)
{
	int i, iFontOff, iLen, iFontSkip;
	unsigned char c, *s, ucTemp[40];

	if (x == -1 || y == -1) // use the cursor position
	{
		x = pOBD->iCursorX;
		y = pOBD->iCursorY;
	}
	else
	{
		pOBD->iCursorX = x;
		pOBD->iCursorY = y; // set the new cursor position
	}
	if (pOBD->iCursorX >= pOBD->width || pOBD->iCursorY >= pOBD->height / 8)
		return -1; // can't draw off the display

	obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY, bRender);
	if (iSize == FONT_8x8) // 8x8 font
	{
		i = 0;
		iFontSkip = iScroll & 7; // number of columns to initially skip
		while (pOBD->iCursorX < pOBD->width && szMsg[i] != 0 && pOBD->iCursorY < pOBD->height / 8)
		{
			if (iScroll < 8) // only display visible characters
			{
				c = (unsigned char)szMsg[i];
				iFontOff = (int)(c - 32) * 7;
				// we can't directly use the pointer to FLASH memory, so copy to a local buffer
				ucTemp[0] = 0; // first column is blank
				memcpy(&ucTemp[1], &ucFont[iFontOff], 7);
				if (bInvert)
					InvertBytes(ucTemp, 8);
				iLen = 8 - iFontSkip;
				if (pOBD->iCursorX + iLen > pOBD->width) // clip right edge
					iLen = pOBD->width - pOBD->iCursorX;
				obdCachedWrite(pOBD, &ucTemp[iFontSkip], iLen, bRender);
				//             obdWriteDataBlock(pOBD, &ucTemp[iFontSkip], iLen, bRender); // write character pattern
				pOBD->iCursorX += iLen;
				if (pOBD->iCursorX >= pOBD->width - 7 && pOBD->wrap) // word wrap enabled?
				{
					pOBD->iCursorX = 0; // start at the beginning of the next line
					pOBD->iCursorY++;
					obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY, bRender);
				}
				iFontSkip = 0;
			}
			iScroll -= 8;
			i++;
		}                              // while
		obdCachedFlush(pOBD, bRender); // write any remaining data
		return 0;
	} // 8x8
	else if (iSize == FONT_16x32)
	{
		i = 0;
		iFontSkip = iScroll & 15; // number of columns to initially skip
		while (pOBD->iCursorX < pOBD->width && pOBD->iCursorY < (pOBD->height / 8) - 3 && szMsg[i] != 0)
		{
			if (iScroll < 16) // if characters are visible
			{
				s = (unsigned char *)&ucBigFont[(unsigned char)(szMsg[i] - 32) * 64];
				iLen = 16 - iFontSkip;
				if (pOBD->iCursorX + iLen > pOBD->width) // clip right edge
					iLen = pOBD->width - pOBD->iCursorX;
				// we can't directly use the pointer to FLASH memory, so copy to a local buffer
				obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY, bRender);
				memcpy(ucTemp, s, 16);
				if (bInvert)
					InvertBytes(ucTemp, 16);
				obdWriteDataBlock(pOBD, &ucTemp[iFontSkip], iLen, bRender); // write character pattern
				obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY + 1, bRender);
				memcpy(ucTemp, s + 16, 16);
				if (bInvert)
					InvertBytes(ucTemp, 16);
				obdWriteDataBlock(pOBD, &ucTemp[iFontSkip], iLen, bRender); // write character pattern
																																		//              if (pOBD->iCursorY <= 5)
				{
					obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY + 2, bRender);
					memcpy(ucTemp, s + 32, 16);
					if (bInvert)
						InvertBytes(ucTemp, 16);
					obdWriteDataBlock(pOBD, &ucTemp[iFontSkip], iLen, bRender); // write character pattern
				}
				//              if (pOBD->iCursorY <= 4)
				{
					obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY + 3, bRender);
					memcpy(ucTemp, s + 48, 16);
					if (bInvert)
						InvertBytes(ucTemp, 16);
					obdWriteDataBlock(pOBD, &ucTemp[iFontSkip], iLen, bRender); // write character pattern
				}
				pOBD->iCursorX += iLen;
				if (pOBD->iCursorX >= pOBD->width - 15 && pOBD->wrap) // word wrap enabled?
				{
					pOBD->iCursorX = 0; // start at the beginning of the next line
					pOBD->iCursorY += 4;
				}
				iFontSkip = 0;
			} // if character visible from scrolling
			iScroll -= 16;
			i++;
		} // while
		return 0;
	}                             // 16x32
	else if (iSize == FONT_16x16) // 8x8 stretched to 16x16
	{
		i = 0;
		iFontSkip = iScroll & 15; // number of columns to initially skip
		while (pOBD->iCursorX < pOBD->width && pOBD->iCursorY < (pOBD->height / 8) - 1 && szMsg[i] != 0)
		{
			// stretch the 'normal' font instead of using the big font
			if (iScroll < 16) // if characters are visible
			{
				int tx, ty;
				c = szMsg[i] - 32;
				unsigned char uc1, uc2, ucMask, *pDest;
				s = (unsigned char *)&ucFont[(int)c * 7];
				ucTemp[0] = 0; // first column is blank
				memcpy(&ucTemp[1], s, 7);
				if (bInvert)
					InvertBytes(ucTemp, 8);
				// Stretch the font to double width + double height
				memset(&ucTemp[8], 0, 32); // write 32 new bytes
				for (tx = 0; tx < 8; tx++)
				{
					ucMask = 3;
					pDest = &ucTemp[8 + tx * 2];
					uc1 = uc2 = 0;
					c = ucTemp[tx];
					for (ty = 0; ty < 4; ty++)
					{
						if (c & (1 << ty)) // a bit is set
							uc1 |= ucMask;
						if (c & (1 << (ty + 4)))
							uc2 |= ucMask;
						ucMask <<= 2;
					}
					pDest[0] = uc1;
					pDest[1] = uc1; // double width
					pDest[16] = uc2;
					pDest[17] = uc2;
				}
				iLen = 16 - iFontSkip;
				if (pOBD->iCursorX + iLen > pOBD->width) // clip right edge
					iLen = pOBD->width - pOBD->iCursorX;
				obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY, bRender);
				obdWriteDataBlock(pOBD, &ucTemp[8 + iFontSkip], iLen, bRender);
				obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY + 1, bRender);
				obdWriteDataBlock(pOBD, &ucTemp[24 + iFontSkip], iLen, bRender);
				pOBD->iCursorX += iLen;
				if (pOBD->iCursorX >= pOBD->width - 15 && pOBD->wrap) // word wrap enabled?
				{
					pOBD->iCursorX = 0; // start at the beginning of the next line
					pOBD->iCursorY += 2;
					obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY, bRender);
				}
				iFontSkip = 0;
			} // if characters are visible
			iScroll -= 16;
			i++;
		} // while
		return 0;
	}                             // 16x16
	else if (iSize == FONT_12x16) // 6x8 stretched to 12x16
	{
		i = 0;
		iFontSkip = iScroll % 12; // number of columns to initially skip
		while (pOBD->iCursorX < pOBD->width && pOBD->iCursorY < (pOBD->height / 8) - 1 && szMsg[i] != 0)
		{
			// stretch the 'normal' font instead of using the big font
			if (iScroll < 12) // if characters are visible
			{
				int tx, ty;
				c = szMsg[i] - 32;
				unsigned char uc1, uc2, ucMask, *pDest;
				s = (unsigned char *)&ucSmallFont[(int)c * 5];
				ucTemp[0] = 0; // first column is blank
				memcpy(&ucTemp[1], s, 6);
				if (bInvert)
					InvertBytes(ucTemp, 6);
				// Stretch the font to double width + double height
				memset(&ucTemp[6], 0, 24); // write 24 new bytes
				for (tx = 0; tx < 6; tx++)
				{
					ucMask = 3;
					pDest = &ucTemp[6 + tx * 2];
					uc1 = uc2 = 0;
					c = ucTemp[tx];
					for (ty = 0; ty < 4; ty++)
					{
						if (c & (1 << ty)) // a bit is set
							uc1 |= ucMask;
						if (c & (1 << (ty + 4)))
							uc2 |= ucMask;
						ucMask <<= 2;
					}
					pDest[0] = uc1;
					pDest[1] = uc1; // double width
					pDest[12] = uc2;
					pDest[13] = uc2;
				}
				// smooth the diagonal lines
				for (tx = 0; tx < 5; tx++)
				{
					uint8_t c0, c1, ucMask2;
					c0 = ucTemp[tx];
					c1 = ucTemp[tx + 1];
					pDest = &ucTemp[6 + tx * 2];
					ucMask = 1;
					ucMask2 = 2;
					for (ty = 0; ty < 7; ty++)
					{
						if (((c0 & ucMask) && !(c1 & ucMask) && !(c0 & ucMask2) && (c1 & ucMask2)) || (!(c0 & ucMask) && (c1 & ucMask) && (c0 & ucMask2) && !(c1 & ucMask2)))
						{
							if (ty < 3) // top half
							{
								if (bInvert)
								{
									pDest[1] &= ~(1 << ((ty * 2) + 1));
									pDest[2] &= ~(1 << ((ty * 2) + 1));
									pDest[1] &= ~(1 << ((ty + 1) * 2));
									pDest[2] &= ~(1 << ((ty + 1) * 2));
								}
								else
								{
									pDest[1] |= (1 << ((ty * 2) + 1));
									pDest[2] |= (1 << ((ty * 2) + 1));
									pDest[1] |= (1 << ((ty + 1) * 2));
									pDest[2] |= (1 << ((ty + 1) * 2));
								}
							}
							else if (ty == 3) // on the border
							{
								if (bInvert)
								{
									pDest[1] &= ~0x80;
									pDest[2] &= ~0x80;
									pDest[13] &= ~1;
									pDest[14] &= ~1;
								}
								else
								{
									pDest[1] |= 0x80;
									pDest[2] |= 0x80;
									pDest[13] |= 1;
									pDest[14] |= 1;
								}
							}
							else // bottom half
							{
								if (bInvert)
								{
									pDest[13] &= ~(1 << (2 * (ty - 4) + 1));
									pDest[14] &= ~(1 << (2 * (ty - 4) + 1));
									pDest[13] &= ~(1 << ((ty - 3) * 2));
									pDest[14] &= ~(1 << ((ty - 3) * 2));
								}
								else
								{
									pDest[13] |= (1 << (2 * (ty - 4) + 1));
									pDest[14] |= (1 << (2 * (ty - 4) + 1));
									pDest[13] |= (1 << ((ty - 3) * 2));
									pDest[14] |= (1 << ((ty - 3) * 2));
								}
							}
						}
						else if (!(c0 & ucMask) && (c1 & ucMask) && (c0 & ucMask2) && !(c1 & ucMask2))
						{
							if (ty < 4) // top half
							{
								if (bInvert)
								{
									pDest[1] &= ~(1 << ((ty * 2) + 1));
									pDest[2] &= ~(1 << ((ty + 1) * 2));
								}
								else
								{
									pDest[1] |= (1 << ((ty * 2) + 1));
									pDest[2] |= (1 << ((ty + 1) * 2));
								}
							}
							else
							{
								if (bInvert)
								{
									pDest[13] &= ~(1 << (2 * (ty - 4) + 1));
									pDest[14] &= ~(1 << ((ty - 3) * 2));
								}
								else
								{
									pDest[13] |= (1 << (2 * (ty - 4) + 1));
									pDest[14] |= (1 << ((ty - 3) * 2));
								}
							}
						}
						ucMask <<= 1;
						ucMask2 <<= 1;
					}
				}
				iLen = 12 - iFontSkip;
				if (pOBD->iCursorX + iLen > pOBD->width) // clip right edge
					iLen = pOBD->width - pOBD->iCursorX;
				obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY, bRender);
				obdWriteDataBlock(pOBD, &ucTemp[6 + iFontSkip], iLen, bRender);
				obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY + 1, bRender);
				obdWriteDataBlock(pOBD, &ucTemp[18 + iFontSkip], iLen, bRender);
				pOBD->iCursorX += iLen;
				if (pOBD->iCursorX >= pOBD->width - 11 && pOBD->wrap) // word wrap enabled?
				{
					pOBD->iCursorX = 0; // start at the beginning of the next line
					pOBD->iCursorY += 2;
					obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY, bRender);
				}
				iFontSkip = 0;
			} // if characters are visible
			iScroll -= 12;
			i++;
		} // while
		return 0;
	} // 12x16
	else if (iSize == FONT_6x8)
	{
		i = 0;
		iFontSkip = iScroll % 6;
		while (pOBD->iCursorX < pOBD->width && pOBD->iCursorY < (pOBD->height / 8) && szMsg[i] != 0)
		{
			if (iScroll < 6) // if characters are visible
			{
				c = szMsg[i] - 32;
				// we can't directly use the pointer to FLASH memory, so copy to a local buffer
				ucTemp[0] = 0; // first column is blank
				memcpy(&ucTemp[1], &ucSmallFont[(int)c * 5], 5);
				if (bInvert)
					InvertBytes(ucTemp, 6);
				iLen = 6 - iFontSkip;
				if (pOBD->iCursorX + iLen > pOBD->width) // clip right edge
					iLen = pOBD->width - pOBD->iCursorX;
				obdCachedWrite(pOBD, &ucTemp[iFontSkip], iLen, bRender);
				//               obdWriteDataBlock(pOBD, &ucTemp[iFontSkip], iLen, bRender); // write character pattern
				pOBD->iCursorX += iLen;
				iFontSkip = 0;
				if (pOBD->iCursorX >= pOBD->width - 5 && pOBD->wrap) // word wrap enabled?
				{
					pOBD->iCursorX = 0; // start at the beginning of the next line
					pOBD->iCursorY++;
					obdSetPosition(pOBD, pOBD->iCursorX, pOBD->iCursorY, bRender);
				}
			} // if characters are visible
			iScroll -= 6;
			i++;
		}
		obdCachedFlush(pOBD, bRender); // write any remaining data
		return 0;
	}          // 6x8
	return -1; // invalid size
} /* obdWriteString() */
//
// Get the width of text in a custom font
//
void obdGetStringBox(GFXfont *pFont, char *szMsg, int *width, int *top, int *bottom)
{
	int cx = 0;
	int c, i = 0;
	GFXglyph *pGlyph;
	int miny, maxy;

	if (width == NULL || top == NULL || bottom == NULL || pFont == NULL || szMsg == NULL)
		return; // bad pointers
	miny = 100;
	maxy = 0;
	while (szMsg[i])
	{
		c = szMsg[i++];
		if (c < pFont->first || c > pFont->last) // undefined character
			continue;                              // skip it
		c -= pFont->first;                       // first char of font defined
		pGlyph = &pFont->glyph[c];
		cx += pGlyph->xAdvance;
		if (pGlyph->yOffset < miny)
			miny = pGlyph->yOffset;
		if (pGlyph->height + pGlyph->yOffset > maxy)
			maxy = pGlyph->height + pGlyph->yOffset;
	}
	*width = cx;
	*top = miny;
	*bottom = maxy;
} /* obdGetStringBox() */

//
// Draw a string of characters in a custom font
// A back buffer must be defined
//
int obdWriteStringCustom(OBDISP *pOBD, GFXfont *pFont, int x, int y, char *szMsg, uint8_t ucColor)
{
	int i, end_y, dx, dy, tx, ty, c, iBitOff;
	uint8_t *s, *d, bits, ucMask, ucClr, uc;
	GFXfont font;
	GFXglyph glyph, *pGlyph;
	int iPitch;

	if (pOBD == NULL || pFont == NULL || pOBD->ucScreen == NULL || x < 0)
		return -1;
	iPitch = pOBD->width;
	// in case of running on AVR, get copy of data from FLASH
	memcpy(&font, pFont, sizeof(font));
	pGlyph = &glyph;

	i = 0;
	while (szMsg[i] && x < pOBD->width)
	{
		c = szMsg[i++];
		if (c < font.first || c > font.last) // undefined character
			continue;                          // skip it
		c -= font.first;                     // first char of font defined
		memcpy(&glyph, &font.glyph[c], sizeof(glyph));
		dx = x + pGlyph->xOffset; // offset from character UL to start drawing
		dy = y + pGlyph->yOffset;
		s = font.bitmap + pGlyph->bitmapOffset; // start of bitmap data
		// Bitmap drawing loop. Image is MSB first and each pixel is packed next
		// to the next (continuing on to the next character line)
		iBitOff = 0;   // bitmap offset (in bits)
		bits = uc = 0; // bits left in this font byte
		end_y = dy + pGlyph->height;
		if (dy < 0)
		{ // skip these lines
			iBitOff += (pGlyph->width * (-dy));
			dy = 0;
		}
		for (ty = dy; ty < end_y && ty < pOBD->height; ty++)
		{
			ucMask = 1 << (ty & 7); // destination bit number for this line
			ucClr = (ucColor) ? ucMask : 0;
			d = &pOBD->ucScreen[(ty >> 3) * iPitch + dx]; // internal buffer dest
			for (tx = 0; tx < pGlyph->width; tx++)
			{
				if (uc == 0)
				{                           // need to read more font data
					tx += bits;               // skip any remaining 0 bits
					uc = s[iBitOff >> 3];     // get more font bitmap data
					bits = 8 - (iBitOff & 7); // we might not be on a byte boundary
					iBitOff += bits;          // because of a clipped line
					uc <<= (8 - bits);
					if (tx >= pGlyph->width)
					{
						while (tx >= pGlyph->width)
						{ // rolls into next line(s)
							tx -= pGlyph->width;
							ty++;
						}
						if (ty >= end_y || ty >= pOBD->height)
						{ // we're past the end
							tx = pGlyph->width;
							continue; // exit this character cleanly
						}
						// need to recalculate mask and offset in case Y changed
						ucMask = 1 << (ty & 7); // destination bit number for this line
						ucClr = (ucColor) ? ucMask : 0;
						d = &pOBD->ucScreen[(ty >> 3) * iPitch + dx]; // internal buffer dest
					}
				} // if we ran out of bits
				if (uc & 0x80)
				{ // set pixel
					if (ucClr)
						d[tx] |= ucMask;
					else
						d[tx] &= ~ucMask;
				}
				bits--; // next bit
				uc <<= 1;
			}                    // for x
		}                      // for y
		x += pGlyph->xAdvance; // width of this character
	}                        // while drawing characters
	return 0;
} /* obdWriteStringCustom() */

//
// Render a sprite/rectangle of pixels from a provided buffer to the display.
// The row values refer to byte rows, not pixel rows due to the memory
// layout of OLEDs and LCDs.
// returns 0 for success, -1 for invalid parameter
//
int obdDrawGFX(OBDISP *pOBD, uint8_t *pBuffer, int iSrcCol, int iSrcRow, int iDestCol, int iDestRow, int iWidth, int iHeight, int iSrcPitch)
{
	int y;

	if (iSrcCol < 0 || iSrcCol >= pOBD->width || iSrcRow < 0 || iSrcRow > (pOBD->height / 8) - 1 || iDestCol < 0 || iDestCol >= pOBD->width || iDestRow < 0 || iDestRow >= (pOBD->height >> 3) || iSrcPitch <= 0)
		return -1; // invalid

	for (y = iSrcRow; y < iSrcRow + iHeight; y++)
	{
		uint8_t *s = &pBuffer[(y * iSrcPitch) + iSrcCol];
		obdSetPosition(pOBD, iDestCol, iDestRow, 1);
		obdWriteDataBlock(pOBD, s, iWidth, 1);
		pBuffer += iSrcPitch;
		iDestRow++;
	} // for y
	return 0;
} /* obdDrawGFX() */
//
// Fill the frame buffer with a byte pattern
// e.g. all off (0x00) or all on (0xff)
//
void obdFill(OBDISP *pOBD, unsigned char ucData, int bRender)
{
	uint8_t y;
	uint8_t iLines;

	pOBD->iCursorX = pOBD->iCursorY = 0;
	if (pOBD->type == LCD_VIRTUAL || pOBD->type >= SHARP_144x168) // pure memory, handle it differently
	{
		if (pOBD->ucScreen)
			memset(pOBD->ucScreen, ucData, pOBD->width * (pOBD->height / 8));
		return;
	}
	iLines = pOBD->height >> 3;
	memset(u8Cache, ucData, pOBD->width);

	for (y = 0; y < iLines; y++)
	{
		obdSetPosition(pOBD, 0, y, bRender); // set to (0,Y)
		obdWriteDataBlock(pOBD, u8Cache, pOBD->width, bRender);
	} // for y
	if (pOBD->ucScreen)
		memset(pOBD->ucScreen, ucData, (pOBD->width * pOBD->height) / 8);
} /* obdFill() */

//
// Provide or revoke a back buffer for your OLED graphics
// This allows you to manage the RAM used by ss_oled on tiny
// embedded platforms like the ATmega series
// Pass NULL to revoke the buffer. Make sure you provide a buffer
// large enough for your display (e.g. 128x64 needs 1K - 1024 bytes)
//
void obdSetBackBuffer(OBDISP *pOBD, uint8_t *pBuffer)
{
	pOBD->ucScreen = pBuffer;
} /* obdSetBackBuffer() */

void obdDrawLine(OBDISP *pOBD, int x1, int y1, int x2, int y2, uint8_t ucColor, int bRender)
{
	int temp;
	int dx = x2 - x1;
	int dy = y2 - y1;
	int error;
	uint8_t *p, *pStart, mask, bOld, bNew;
	int xinc, yinc;
	int y, x;
	int iPitch = pOBD->width;

	if (x1 < 0 || x2 < 0 || y1 < 0 || y2 < 0 || x1 >= pOBD->width || x2 >= pOBD->width || y1 >= pOBD->height || y2 >= pOBD->height)
		return;

	if (abs(dx) > abs(dy))
	{
		// X major case
		if (x2 < x1)
		{
			dx = -dx;
			temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}

		y = y1;
		dy = (y2 - y1);
		error = dx >> 1;
		yinc = 1;
		if (dy < 0)
		{
			dy = -dy;
			yinc = -1;
		}
		p = pStart = &pOBD->ucScreen[x1 + ((y >> 3) * iPitch)]; // point to current spot in back buffer
		mask = 1 << (y & 7);                                    // current bit offset
		for (x = x1; x1 <= x2; x1++)
		{
			if (ucColor)
				*p++ |= mask; // set pixel and increment x pointer
			else
				*p++ &= ~mask;
			error -= dy;
			if (error < 0)
			{
				error += dx;
				if (yinc > 0)
					mask <<= 1;
				else
					mask >>= 1;
				if (mask == 0) // we've moved outside the current row, write the data we changed
				{
					obdSetPosition(pOBD, x, y >> 3, bRender);
					obdWriteDataBlock(pOBD, pStart, (int)(p - pStart), bRender); // write the row we changed
					x = x1 + 1;                                                  // we've already written the byte at x1
					y1 = y + yinc;
					p += (yinc > 0) ? iPitch : -iPitch;
					pStart = p;
					mask = 1 << (y1 & 7);
				}
				y += yinc;
			}
		}                // for x1
		if (p != pStart) // some data needs to be written
		{
			obdSetPosition(pOBD, x, y >> 3, bRender);
			obdWriteDataBlock(pOBD, pStart, (int)(p - pStart), bRender);
		}
	}
	else
	{
		// Y major case
		if (y1 > y2)
		{
			dy = -dy;
			temp = x1;
			x1 = x2;
			x2 = temp;
			temp = y1;
			y1 = y2;
			y2 = temp;
		}

		p = &pOBD->ucScreen[x1 + ((y1 >> 3) * iPitch)]; // point to current spot in back buffer
		bOld = bNew = p[0];                             // current data at that address
		mask = 1 << (y1 & 7);                           // current bit offset
		dx = (x2 - x1);
		error = dy >> 1;
		xinc = 1;
		if (dx < 0)
		{
			dx = -dx;
			xinc = -1;
		}
		for (x = x1; y1 <= y2; y1++)
		{
			if (ucColor)
				bNew |= mask; // set the pixel
			else
				bNew &= ~mask;
			error -= dx;
			mask <<= 1;    // y1++
			if (mask == 0) // we're done with this byte, write it if necessary
			{
				if (bOld != bNew)
				{
					p[0] = bNew; // save to RAM
					obdSetPosition(pOBD, x, y1 >> 3, bRender);
					obdWriteDataBlock(pOBD, &bNew, 1, bRender);
				}
				p += iPitch; // next line
				bOld = bNew = p[0];
				mask = 1; // start at LSB again
			}
			if (error < 0)
			{
				error += dy;
				if (bOld != bNew) // write the last byte we modified if it changed
				{
					p[0] = bNew; // save to RAM
					obdSetPosition(pOBD, x, y1 >> 3, bRender);
					obdWriteDataBlock(pOBD, &bNew, 1, bRender);
				}
				p += xinc;
				x += xinc;
				bOld = bNew = p[0];
			}
		}                 // for y
		if (bOld != bNew) // write the last byte we modified if it changed
		{
			p[0] = bNew; // save to RAM
			obdSetPosition(pOBD, x, y2 >> 3, bRender);
			obdWriteDataBlock(pOBD, &bNew, 1, bRender);
		}
	} // y major case
} /* obdDrawLine() */

//
// For drawing ellipses, a circle is drawn and the x and y pixels are scaled by a 16-bit integer fraction
// This function draws a single pixel and scales its position based on the x/y fraction of the ellipse
//
static void DrawScaledPixel(OBDISP *pOBD, int iCX, int iCY, int x, int y, int32_t iXFrac, int32_t iYFrac, uint8_t ucColor)
{
	uint8_t *d, ucMask;
	int iPitch;

	iPitch = pOBD->width;
	if (iXFrac != 0x10000)
		x = ((x * iXFrac) >> 16);
	if (iYFrac != 0x10000)
		y = ((y * iYFrac) >> 16);
	x += iCX;
	y += iCY;
	if (x < 0 || x >= pOBD->width || y < 0 || y >= pOBD->height)
		return; // off the screen
	d = &pOBD->ucScreen[((y >> 3) * iPitch) + x];
	ucMask = 1 << (y & 7);
	if (ucColor)
		*d |= ucMask;
	else
		*d &= ~ucMask;
} /* DrawScaledPixel() */
//
// For drawing filled ellipses
//
static void DrawScaledLine(OBDISP *pOBD, int iCX, int iCY, int x, int y, int32_t iXFrac, int32_t iYFrac, uint8_t ucColor)
{
	int iLen, x2;
	uint8_t *d, ucMask;
	int iPitch;

	iPitch = pOBD->width;
	if (iXFrac != 0x10000)
		x = ((x * iXFrac) >> 16);
	if (iYFrac != 0x10000)
		y = ((y * iYFrac) >> 16);
	iLen = x * 2;
	x = iCX - x;
	y += iCY;
	x2 = x + iLen;
	if (y < 0 || y >= pOBD->height)
		return; // completely off the screen
	if (x < 0)
		x = 0;
	if (x2 >= pOBD->width)
		x2 = pOBD->width - 1;
	iLen = x2 - x + 1; // new length
	d = &pOBD->ucScreen[((y >> 3) * iPitch) + x];
	ucMask = 1 << (y & 7);
	if (ucColor) // white
	{
		for (; iLen > 0; iLen--)
			*d++ |= ucMask;
	}
	else // black
	{
		for (; iLen > 0; iLen--)
			*d++ &= ~ucMask;
	}
} /* DrawScaledLine() */
//
// Draw the 8 pixels around the Bresenham circle
// (scaled to make an ellipse)
//
static void BresenhamCircle(OBDISP *pOBD, int iCX, int iCY, int x, int y, int32_t iXFrac, int32_t iYFrac, uint8_t ucColor, uint8_t bFill)
{
	if (bFill) // draw a filled ellipse
	{
		// for a filled ellipse, draw 4 lines instead of 8 pixels
		DrawScaledLine(pOBD, iCX, iCY, x, y, iXFrac, iYFrac, ucColor);
		DrawScaledLine(pOBD, iCX, iCY, x, -y, iXFrac, iYFrac, ucColor);
		DrawScaledLine(pOBD, iCX, iCY, y, x, iXFrac, iYFrac, ucColor);
		DrawScaledLine(pOBD, iCX, iCY, y, -x, iXFrac, iYFrac, ucColor);
	}
	else // draw 8 pixels around the edges
	{
		DrawScaledPixel(pOBD, iCX, iCY, x, y, iXFrac, iYFrac, ucColor);
		DrawScaledPixel(pOBD, iCX, iCY, -x, y, iXFrac, iYFrac, ucColor);
		DrawScaledPixel(pOBD, iCX, iCY, x, -y, iXFrac, iYFrac, ucColor);
		DrawScaledPixel(pOBD, iCX, iCY, -x, -y, iXFrac, iYFrac, ucColor);
		DrawScaledPixel(pOBD, iCX, iCY, y, x, iXFrac, iYFrac, ucColor);
		DrawScaledPixel(pOBD, iCX, iCY, -y, x, iXFrac, iYFrac, ucColor);
		DrawScaledPixel(pOBD, iCX, iCY, y, -x, iXFrac, iYFrac, ucColor);
		DrawScaledPixel(pOBD, iCX, iCY, -y, -x, iXFrac, iYFrac, ucColor);
	}
} /* BresenhamCircle() */

//
// Draw an outline or filled ellipse
//
void obdEllipse(OBDISP *pOBD, int iCenterX, int iCenterY, int32_t iRadiusX, int32_t iRadiusY, uint8_t ucColor, uint8_t bFilled)
{
	int32_t iXFrac, iYFrac;
	int iRadius, iDelta, x, y;

	if (pOBD == NULL || pOBD->ucScreen == NULL)
		return; // must have back buffer defined
	if (iRadiusX <= 0 || iRadiusY <= 0)
		return; // invalid radii

	if (iRadiusX > iRadiusY) // use X as the primary radius
	{
		iRadius = iRadiusX;
		iXFrac = 65536;
		iYFrac = (iRadiusY * 65536) / iRadiusX;
	}
	else
	{
		iRadius = iRadiusY;
		iXFrac = (iRadiusX * 65536) / iRadiusY;
		iYFrac = 65536;
	}
	iDelta = 3 - (2 * iRadius);
	x = 0;
	y = iRadius;
	while (x <= y)
	{
		BresenhamCircle(pOBD, iCenterX, iCenterY, x, y, iXFrac, iYFrac, ucColor, bFilled);
		x++;
		if (iDelta < 0)
		{
			iDelta += (4 * x) + 6;
		}
		else
		{
			iDelta += 4 * (x - y) + 10;
			y--;
		}
	}
} /* obdEllipse() */

//
// Draw an outline or filled ellipse with more precision
//
void obdPreciseEllipse(OBDISP *pOBD, int x, int y, int32_t iRadiusX, int32_t iRadiusY, uint8_t ucColor, uint8_t bFilled)
{
	long x1 = -iRadiusX, y1 = 0;
	long e2 = iRadiusY, dx = (1 + 2 * x1) * e2 * e2;
	long dy = x1 * x1, err = dx + dy;
	long diff = 0;

	do {
		obdSetPixel(pOBD, x - x1, y + y1, ucColor, 0);
		obdSetPixel(pOBD, x - x1, y + y1, ucColor, 0);
		obdSetPixel(pOBD, x + x1, y + y1, ucColor, 0);
		obdSetPixel(pOBD, x + x1, y - y1, ucColor, 0);
		obdSetPixel(pOBD, x - x1, y - y1, ucColor, 0);

		if (bFilled)
		{
			for (int i = 0; i < ((x - x1) - (x + x1)) / 2; i++) {
				obdSetPixel(pOBD, x - i, y + y1, ucColor, 0);
				obdSetPixel(pOBD, x + i, y + y1, ucColor, 0);
				obdSetPixel(pOBD, x + i, y - y1, ucColor, 0);
				obdSetPixel(pOBD, x - i, y - y1, ucColor, 0);
			}
		}

		e2 = 2 * err;

		if (e2 >= dx) {
			x1++;
			err += dx += 2 * (long)iRadiusY * iRadiusY;
		}

		if (e2 <= dy) {
			y1++;
			err += dy += 2 * (long)iRadiusX * iRadiusX;
		}
	} while (x1 <= 0);

	while (y1++ < iRadiusY) {
		obdSetPixel(pOBD, x, y + y1, ucColor, 0);
		obdSetPixel(pOBD, x, y - y1, ucColor, 0);
	}
} /* obdPreciseEllipse() */

//
// Draw an outline or filled rectangle
//
void obdRectangle(OBDISP *pOBD, int x1, int y1, int x2, int y2, uint8_t ucColor, uint8_t bFilled)
{
	uint8_t *d, ucMask, ucMask2;
	int tmp, iOff;
	int iPitch;

	if (pOBD == NULL || pOBD->ucScreen == NULL)
		return; // only works with a back buffer
	if (x1 < 0 || y1 < 0 || x2 < 0 || y2 < 0 ||
			x1 >= pOBD->width || y1 >= pOBD->height || x2 >= pOBD->width || y2 >= pOBD->height)
		return; // invalid coordinates
	iPitch = pOBD->width;
	// Make sure that X1/Y1 is above and to the left of X2/Y2
	// swap coordinates as needed to make this true
	if (x2 < x1)
	{
		tmp = x1;
		x1 = x2;
		x2 = tmp;
	}
	if (y2 < y1)
	{
		tmp = y1;
		y1 = y2;
		y2 = tmp;
	}
	if (bFilled)
	{
		int x, y, iMiddle;
		iMiddle = (y2 >> 3) - (y1 >> 3);
		ucMask = 0xff << (y1 & 7);
		if (iMiddle == 0) // top and bottom lines are in the same row
			ucMask &= (0xff >> (7 - (y2 & 7)));
		d = &pOBD->ucScreen[(y1 >> 3) * iPitch + x1];
		// Draw top
		for (x = x1; x <= x2; x++)
		{
			if (ucColor)
				*d |= ucMask;
			else
				*d &= ~ucMask;
			d++;
		}
		if (iMiddle > 1) // need to draw middle part
		{
			ucMask = (ucColor) ? 0xff : 0x00;
			for (y = 1; y < iMiddle; y++)
			{
				d = &pOBD->ucScreen[(y1 >> 3) * iPitch + x1 + (y * iPitch)];
				for (x = x1; x <= x2; x++)
					*d++ = ucMask;
			}
		}
		if (iMiddle >= 1) // need to draw bottom part
		{
			ucMask = 0xff >> (7 - (y2 & 7));
			d = &pOBD->ucScreen[(y2 >> 3) * iPitch + x1];
			for (x = x1; x <= x2; x++)
			{
				if (ucColor)
					*d++ |= ucMask;
				else
					*d++ &= ~ucMask;
			}
		}
	}
	else // outline
	{
		// see if top and bottom lines are within the same byte rows
		d = &pOBD->ucScreen[(y1 >> 3) * iPitch + x1];
		if ((y1 >> 3) == (y2 >> 3))
		{
			ucMask2 = 0xff << (y1 & 7); // L/R end masks
			ucMask = 1 << (y1 & 7);
			ucMask |= 1 << (y2 & 7);
			ucMask2 &= (0xff >> (7 - (y2 & 7)));
			if (ucColor)
			{
				*d++ |= ucMask2; // start
				x1++;
				for (; x1 < x2; x1++)
					*d++ |= ucMask;
				if (x1 <= x2)
					*d++ |= ucMask2; // right edge
			}
			else
			{
				*d++ &= ~ucMask2;
				x1++;
				for (; x1 < x2; x1++)
					*d++ &= ~ucMask;
				if (x1 <= x2)
					*d++ &= ~ucMask2; // right edge
			}
		}
		else
		{
			int y;
			// L/R sides
			iOff = (x2 - x1);
			ucMask = 1 << (y1 & 7);
			for (y = y1; y <= y2; y++)
			{
				if (ucColor)
				{
					*d |= ucMask;
					d[iOff] |= ucMask;
				}
				else
				{
					*d &= ~ucMask;
					d[iOff] &= ~ucMask;
				}
				ucMask <<= 1;
				if (ucMask == 0)
				{
					ucMask = 1;
					d += iPitch;
				}
			}
			// T/B sides
			ucMask = 1 << (y1 & 7);
			ucMask2 = 1 << (y2 & 7);
			x1++;
			d = &pOBD->ucScreen[(y1 >> 3) * iPitch + x1];
			iOff = (y2 >> 3) - (y1 >> 3);
			iOff *= iPitch;
			for (; x1 < x2; x1++)
			{
				if (ucColor)
				{
					*d |= ucMask;
					d[iOff] |= ucMask2;
				}
				else
				{
					*d &= ~ucMask;
					d[iOff] &= ~ucMask2;
				}
				d++;
			}
		}
	} // outline
} /* obdRectangle() */
//
// Copy the current bitmap buffer from its native form (LSB_FIRST, VERTICAL_BYTES) to the requested form
// returns 0 for success, -1 for error
// Constants to be combined for the iFlags parameter:
// Output format options -
// OBD_LSB_FIRST     0x001
// OBD_MSB_FIRST     0x002
// OBD_VERT_BYTES    0x004
// OBD_HORZ_BYTES    0x008
// Orientation Options -
// OBD_ROTATE_90     0x010
// OBD_FLIP_VERT     0x020
// OBD_FLIP_HORZ     0x040
// Polarity Options -
// OBD_INVERT        0x080
int obdCopy(OBDISP *pOBD, int iFlags, uint8_t *pDestination)
{
	int i, x, y, iPitch, iSize;
	int xStart, xEnd, yStart, yEnd, yDst, xDst, dx, dy;
	uint8_t ucSrcMask, ucDstMask, *s, *d;

	iPitch = pOBD->width;

	if (pDestination == NULL || pOBD == NULL || pOBD->ucScreen == NULL)
		return -1;
	// calculate output buffer size
	if (iFlags & OBD_HORZ_BYTES)
	{
		if (iFlags & OBD_ROTATE_90)
			iSize = ((pOBD->height + 7) >> 3) * pOBD->width;
		else
			iSize = ((pOBD->width + 7) >> 3) * pOBD->height;
	}
	else
	{
		if (iFlags & OBD_ROTATE_90)
			iSize = (pOBD->height * ((pOBD->width + 7) >> 3));
		else
			iSize = (pOBD->width * ((pOBD->height + 7) >> 3));
	}
	memset(pDestination, 0, iSize); // start with 0 in dest
	// Prepare vars for walking through the source image
	if (iFlags & OBD_ROTATE_90)
	{
		if (iFlags & OBD_FLIP_HORZ)
		{
			dy = 1;
			yStart = 0;
			yEnd = pOBD->height;
		}
		else
		{
			dy = -1;
			yStart = pOBD->height - 1;
			yEnd = -1;
		}
		if (iFlags & OBD_FLIP_VERT)
		{
			dx = -1;
			xStart = pOBD->width - 1;
			xEnd = -1;
		}
		else
		{
			dx = 1;
			xStart = 0;
			xEnd = pOBD->width;
		}
	}
	else
	{ // not rotated
		if (iFlags & OBD_FLIP_HORZ)
		{
			dx = -1;
			xStart = pOBD->width - 1;
			xEnd = -1;
		}
		else
		{
			dx = 1;
			xStart = 0;
			xEnd = pOBD->width;
		}
		if (iFlags & OBD_FLIP_VERT)
		{
			dy = -1;
			yStart = pOBD->height - 1;
			yEnd = -1;
		}
		else
		{
			dy = 1;
			yStart = 0;
			yEnd = pOBD->height;
		}
	}
	// Due to the possible number of permutations, the different loops
	// are more generic and handle flips/bit-direction with a more general
	// approach which moves individual pixels even when a more efficient
	// method is possible. More cycles, but able to do EVERYTHING
	// Separate output by byte orientation
	// Vertical bytes here
	if (iFlags & OBD_VERT_BYTES)
	{
		if (iFlags & OBD_ROTATE_90)
		{
			xDst = 0;
			for (y = yStart; y != yEnd; y += dy, xDst++)
			{
				ucSrcMask = (1 << (y & 7));
				yDst = 0;
				s = &pOBD->ucScreen[(y >> 3) * iPitch];
				d = &pDestination[xDst];
				for (x = xStart; x != xEnd; x += dx, yDst++)
				{
					if (s[x] & ucSrcMask)
					{ // set pixel, copy to dest
						if (iFlags & OBD_LSB_FIRST)
							d[(yDst >> 3) * pOBD->height] |= (1 << (yDst & 7));
						else
							d[(yDst >> 3) * pOBD->height] |= (0x80 >> (yDst & 7));
					}
				} // for x
			}   // for y
		}     // rotate 90
		else  // normally oriented
		{
			yDst = 0;
			for (y = yStart; y != yEnd; y += dy, yDst++)
			{
				ucSrcMask = (1 << (y & 7));
				if (iFlags & OBD_LSB_FIRST)
					ucDstMask = (1 << (y & 7));
				else
					ucDstMask = (0x80 >> (y & 7));
				xDst = 0;
				s = &pOBD->ucScreen[(y >> 3) * iPitch];
				d = &pDestination[(yDst >> 3) * iPitch];
				for (x = xStart; x != xEnd; x += dx, xDst++)
				{
					if (s[x] & ucSrcMask) // set pixel, copy to dest
						d[xDst] |= ucDstMask;
				} // for x
			}   // for y
		}     // normal orientation
	}       // vertical bytes
	else    // Horizontal bytes here
	{
		if (iFlags & OBD_ROTATE_90)
		{
			int iDstPitch = (pOBD->height + 7) >> 3; // dest bytes per line
			xDst = 0;
			for (y = yStart; y != yEnd; y += dy, xDst++)
			{
				ucSrcMask = (1 << (y & 7));
				yDst = 0;
				s = &pOBD->ucScreen[(y >> 3) * iPitch];
				d = &pDestination[xDst >> 3];
				ucDstMask = (iFlags & OBD_LSB_FIRST) ? (1 << (xDst & 7)) : (0x80 >> (xDst & 7));
				for (x = xStart; x != xEnd; x += dx, yDst++)
				{
					if (s[x] & ucSrcMask) // set pixel, copy to dest
						d[yDst * iDstPitch] |= ucDstMask;
				} // for x
			}   // for y
		}     // rotate 90
		else  // normally oriented
		{
			int iDstPitch = (pOBD->width + 7) >> 3; // dest bytes per line
			yDst = 0;
			for (y = yStart; y != yEnd; y += dy, yDst++)
			{
				ucSrcMask = (1 << (y & 7));
				xDst = 0;
				s = &pOBD->ucScreen[(y >> 3) * iPitch];
				d = &pDestination[yDst * iDstPitch];
				ucDstMask = (iFlags & OBD_LSB_FIRST) ? 0x1 : 0x80;
				for (x = xStart; x != xEnd; x += dx, xDst++)
				{
					if (s[x] & ucSrcMask) // set pixel, copy to dest
						d[(xDst >> 3)] |= ucDstMask;
					if (iFlags & OBD_LSB_FIRST)
						ucDstMask <<= 1;
					else
						ucDstMask >>= 1;
					if (ucDstMask == 0)
						ucDstMask = (iFlags & OBD_LSB_FIRST) ? 0x1 : 0x80;
				} // for x
			}   // for y
		}     // normal orientation
	}
	// Invert all pixels?
	if (iFlags & OBD_INVERT)
	{
		for (i = 0; i < iSize; i++)
			pDestination[i] = 255 - pDestination[i];
	}
	return 0;
} /* obdCopy() */

void obdSetFonts(const uint8_t *smallFont, const uint8_t *font, const uint8_t *bigFont) {
	ucSmallFont = smallFont;
	ucFont = font;
	ucBigFont = bigFont;
}