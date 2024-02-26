#ifndef _GPGFX_TINYSSD1306_H_
#define _GPGFX_TINYSSD1306_H_

#include "GPGFX_types.h"
#include "displaybase.h"
#include "math.h"

class GPGFX_TinySSD1306 : public GPGFX_DisplayBase {
    public:
        void init(GPGFX_DisplayTypeOptions options);

        void setPower(bool isPowered);

        void clear();

        void drawPixel(uint8_t x, uint8_t y, uint32_t color);

        void drawText(uint8_t x, uint8_t y, std::string text, uint8_t invert = 0);

        void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color, uint8_t filled);

        void drawArc(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled, double startAngle, double endAngle, uint8_t closed);

        void drawEllipse(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled);

        void drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t filled);

        void drawPolygon(uint16_t x, uint16_t y, uint16_t radius, uint16_t sides, uint32_t color, uint8_t filled, double rotation = 0);

        void drawSprite(uint8_t* spriteData, uint16_t width, uint16_t height, uint16_t pitch, uint16_t x, uint16_t y, uint8_t priority);

        void drawBuffer(uint8_t *pBuffer);
    private:
        typedef enum {
            SET_LOW_COLUMN = 0x00,
            SET_HIGH_COLUMN = 0x10,
            MEMORY_MODE = 0x20,
            COLUMN_ADDRESS = 0x21,
            PAGE_ADDRESS = 0x22,
            DEACTIVATE_SCROLL = 0x2E,
            ACTIVATE_SCROLL = 0x2F,
            SET_START_LINE = 0x40,
            SET_CONTRAST = 0x81,
            CHARGE_PUMP = 0x8D,
            SEGMENT_REMAP_0 = 0xA0,
            SEGMENT_REMAP_127 = 0xA1,
            FULL_DISPLAY_ON_RESUME = 0xA4,
            FULL_DISPLAY_ON = 0xA5,
            NORMAL_DISPLAY = 0xA6,
            INVERT_DISPLAY = 0xA7,
            SET_MULTIPLEX = 0xA8,
            DISPLAY_OFF = 0xAE,
            DISPLAY_ON = 0xAF,
            COM_SCAN_NORMAL = 0xC0,
            COM_SCAN_REVERSE = 0xC8,
            SET_DISPLAY_OFFSET = 0xD3,
            SET_DISPLAY_CLOCK_DIVIDE = 0xD5,
            SET_PRECHARGE = 0xD9,
            SET_COM_PINS = 0xDA,
            SET_VCOM_DETECT = 0xDB,
        } CommandOps;

        bool _isPowered = false;

        static const uint16_t MAX_SCREEN_WIDTH = 128;
        static const uint16_t MAX_SCREEN_HEIGHT = 64;
        static const uint16_t MAX_SCREEN_SIZE = (MAX_SCREEN_WIDTH * MAX_SCREEN_HEIGHT / 8);

        GPGFX_DisplayTypeOptions _options;

        void sendCommand(uint8_t command);
        void sendCommands(uint8_t* commands, uint16_t length);

        uint8_t frameBuffer[MAX_SCREEN_SIZE];
        uint8_t framePage = 0;
};

#endif
