#ifndef _GPSPRITE_H_
#define _GPSPRITE_H_

#include "GPWidget.h"

class GPSprite : public GPWidget {
    public:
        void draw();
        GPSprite* setSizeX(uint16_t sizeX) { this->_sizeX = sizeX; return this; }
        GPSprite* setSizeY(uint16_t sizeY) { this->_sizeY = sizeY; return this; }
    private:
        uint16_t _sizeX = 0;
        uint16_t _sizeY = 0;

        const unsigned char *spriteData;
};

#endif