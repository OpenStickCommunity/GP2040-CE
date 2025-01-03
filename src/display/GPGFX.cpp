#include "GPGFX.h"

#include <cstring>
#include <vector>

#include "peripheralmanager.h"
#include "obd_ssd1306.h"
#include "tiny_ssd1306.h"

std::map<GPGFX_DisplayType, std::map<GPGFX_DisplaySize, GPGFX_DisplayMetrics>> GPGFX_DisplayModes = {
    {
        {DISPLAY_TYPE_SSD1306},
        {
            {SIZE_128x32,{128,32,1}},
            {SIZE_128x64,{128,64,1}},
        },
    },
};

GPGFX::GPGFX() {
    
}

void GPGFX::init(GPGFX_DisplayTypeOptions options) {
    switch (options.displayType) {
        case GPGFX_DisplayType::DISPLAY_TYPE_SSD1306:
            this->displayDriver = new GPGFX_TinySSD1306();
            break;
        default:
            options.displayType = GPGFX_DisplayType::DISPLAY_TYPE_NONE;
    }

    if (options.displayType != GPGFX_DisplayType::DISPLAY_TYPE_NONE) {
        this->displayDriver->setMetrics(&GPGFX_DisplayModes[options.displayType][(GPGFX_DisplaySize)options.size]);
        this->displayDriver->init(options);
    }
}

GPGFX_DisplayTypeOptions GPGFX::getAvailableDisplay(GPGFX_DisplayType displayType) {
    GPGFX_DisplayTypeOptions display;

    display.displayType = displayType;

    if (display.displayType == GPGFX_DisplayType::DISPLAY_TYPE_NONE) {
        // autoscan for device
        for (uint16_t i = GPGFX_DisplayType::DISPLAY_TYPE_NONE; i < GPGFX_DisplayType::DISPLAY_TYPE_COUNT; i++) {
            if (detectDisplay(&display, (GPGFX_DisplayType)i)) break;
        }
    } else {
        if (!detectDisplay(&display, display.displayType)) {
            display.displayType = GPGFX_DisplayType::DISPLAY_TYPE_NONE;
        }
    }
    return display;
}

bool GPGFX::detectDisplay(GPGFX_DisplayTypeOptions* display, GPGFX_DisplayType displayType) {
    GPGFX_DisplayBase* driver = nullptr;

    if (displayType == GPGFX_DisplayType::DISPLAY_TYPE_SSD1306) {
        driver = new GPGFX_TinySSD1306();
    } else {
        driver = nullptr;
    }

    if (driver != nullptr) {
        if (driver->isI2C()) {
            PeripheralI2CScanResult result = PeripheralManager::getInstance().scanForI2CDevice(driver->getDeviceAddresses());
            if (result.address > -1) {
                display->displayType = displayType;
                display->address = result.address;
                display->i2c = PeripheralManager::getInstance().getI2C(result.block);
                display->i2c->setExclusiveUse(result.address);
                return true;
            }
        }
        if (driver->isSPI()) {
            // NYI: check if SPI display exists
        }
        delete driver;
    }

    return false;
}

void GPGFX::clearScreen() {
    this->displayDriver->clear();
}

void GPGFX::render() {
    this->displayDriver->drawBuffer(NULL);
}

uint32_t GPGFX::getPixel(uint16_t x, uint16_t y) {
    return this->displayDriver->getPixel(x, y);
}

void GPGFX::drawPixel(uint16_t x, uint16_t y, uint32_t color) {
    this->displayDriver->drawPixel(x, y, color);
}

void GPGFX::drawText(uint16_t x, uint16_t y, std::string text, uint8_t invert) {
    this->displayDriver->drawText(x, y, text, invert);
}

void GPGFX::drawArc(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled, double startAngle, double endAngle, uint8_t closed) {
    this->displayDriver->drawArc(x, y, radiusX, radiusY, color, filled, startAngle, endAngle, closed);
}

void GPGFX::drawEllipse(uint16_t x, uint16_t y, uint32_t radiusX, uint32_t radiusY, uint32_t color, uint8_t filled) {
    this->displayDriver->drawEllipse(x, y, radiusX, radiusY, color, filled);
}

void GPGFX::drawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color, uint8_t filled) {
    this->displayDriver->drawLine(x1, y1, x2, y2, color, filled);
}

void GPGFX::drawRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint32_t color, uint8_t filled, double rotationAngle) {
    this->displayDriver->drawRectangle(x, y, width, height, color, filled, rotationAngle);
}

void GPGFX::drawPolygon(uint16_t x, uint16_t y, uint16_t radius, uint16_t sides, uint32_t color, uint8_t filled, double rotation) {
    this->displayDriver->drawPolygon(x, y, radius, sides, color, filled, rotation);
}

void GPGFX::drawSprite(uint8_t* spriteData, uint16_t width, uint16_t height, uint16_t pitch, uint16_t x, uint16_t y, uint8_t priority, double scale) {
    this->displayDriver->drawSprite(spriteData, width, height, pitch, x, y, priority, scale);
}
