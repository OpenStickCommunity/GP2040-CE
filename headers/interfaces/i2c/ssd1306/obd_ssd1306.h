#ifndef _GPGFX_OBD_SSD1306_H_
#define _GPGFX_OBD_SSD1306_H_

#include "GPGFX.h"
#include "GPGFX_types.h"
#include "displaybase.h"
#include "OneBitDisplay.h"

class GPGFX_OBD_SSD1306 : public GPGFX_DisplayBase {
    public:
        void init(GPGFX_DisplayTypeOptions options);

        void setPower(bool isPowered);

        void clear();

        void drawPixel(uint8_t x, uint8_t y, uint32_t color);

        void drawText(uint8_t x, uint8_t y, std::string text, uint8_t invert = 0);

        void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color, uint8_t filled);

        void drawEllipse(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled);

        void drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t filled);

        void drawSprite(uint8_t* spriteData, uint16_t width, uint16_t height, uint16_t pitch, uint16_t x, uint16_t y, uint8_t priority);

        void drawBuffer(uint8_t *pBuffer);
    private:
        OBDISP obd;
        GPGFX_DisplayTypeOptions _options;

	    int initDisplay(int typeOverride);
        bool isSH1106(int detectedDisplay);
        void clearScreen(int render);

	    uint8_t ucBackBuffer[1024];
};

#endif
