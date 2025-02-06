#ifndef _GPGFX_H_
#define _GPGFX_H_

#include <string>
#include "pico/stdlib.h"

#include "GPGFX_core.h"

class GPGFX {
    public:
        GPGFX();

        void init(GPGFX_DisplayTypeOptions options);

        GPGFX_DisplayTypeOptions getAvailableDisplay(GPGFX_DisplayType displayType);

        GPGFX_DisplayBase* getDriver() { return displayDriver; }

        // drawing methods
        void clearScreen();
        void render();

        uint32_t getPixel(uint16_t x, uint16_t y);
        void drawPixel(uint16_t x, uint16_t y, uint32_t color);
        void drawText(uint16_t x, uint16_t y, std::string text, uint8_t invert = 0);
        void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color, uint8_t filled);
        void drawArc(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled, double startAngle, double endAngle, uint8_t closed);
        void drawEllipse(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled);
        void drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t filled, double rotationAngle = 0);
        void drawPolygon(uint16_t x, uint16_t y, uint16_t radius, uint16_t sides, uint32_t color, uint8_t filled, double rotation = 0);
        void drawSprite(uint8_t* spriteData, uint16_t width, uint16_t height, uint16_t pitch, uint16_t x, uint16_t y, uint8_t priority, double scale = 1.0);
    private:
        GPGFX_DisplayBase* displayDriver = nullptr;

        bool detectDisplay(GPGFX_DisplayTypeOptions* display, GPGFX_DisplayType displayType);
};

#endif
