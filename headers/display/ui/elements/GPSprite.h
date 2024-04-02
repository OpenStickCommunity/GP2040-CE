#ifndef _GPSPRITE_H_
#define _GPSPRITE_H_

#include "GPWidget.h"

class GPSprite : public GPWidget {
    public:
        void draw();
        GPSprite* setSize(uint16_t sizeX, uint16_t sizeY) { this->_sizeX = sizeX; this->_sizeY = sizeY; return this; }
    private:
        uint16_t _sizeX = 0;
        uint16_t _sizeY = 0;

        const unsigned char *spriteData;
};

#endif