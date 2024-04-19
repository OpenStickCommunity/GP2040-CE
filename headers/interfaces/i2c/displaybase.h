#ifndef _DISPLAYBASE_H_
#define _DISPLAYBASE_H_

#include <string>
#include <string.h>
#include "pico/stdlib.h"
#include "GPGFX_types.h"

class GPGFX_DisplayBase {
    public:
        virtual void init(GPGFX_DisplayTypeOptions options) {}

        virtual void setPower(bool isPowered) {}

        virtual void clear() {}

        virtual void drawPixel(uint8_t x, uint8_t y, uint32_t color) {}

        virtual void drawText(uint8_t x, uint8_t y, std::string text, uint8_t invert = 0) {}

        virtual void drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color, uint8_t filled) {}

        virtual void drawArc(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled, double startAngle, double endAngle, uint8_t closed) {}

        virtual void drawEllipse(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled) {}

        virtual void drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t filled) {}

        virtual void drawPolygon(uint16_t x, uint16_t y, uint16_t radius, uint16_t sides, uint32_t color, uint8_t filled, double rotation = 0) {}

        virtual void drawSprite(uint8_t* spriteData, uint16_t width, uint16_t height, uint16_t pitch, uint16_t x, uint16_t y, uint8_t priority) {}

        virtual void drawBuffer(uint8_t *pBuffer) {}

        void setMetrics(GPGFX_DisplayMetrics* metrics) { this->_metrics = metrics; }
        GPGFX_DisplayMetrics* getMetrics() { return this->_metrics; }
    protected:
        GPGFX_DisplayMetrics* _metrics;
};

#endif
