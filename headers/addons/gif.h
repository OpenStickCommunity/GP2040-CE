#ifndef GIF_H_
#define GIF_H_

#define GIF_ENABLED 0
#define GIF_STARTING_GIF_ADDRESS 0
#define GIF_NUMBER_OF_FRAMES 0

#define GIF_EEPROM_DISTANCE 0xA000 // Leave room of 40kb for the EEPROM to grow
#define GIF_CODE_DISTANCE 0x20000 // Leave room of 128kb for the code to grow 

#include "gpaddon.h"
#include "config.pb.h"

#define GifName "Gif"

class GifAddon: public GPAddon
{
    public:
        virtual bool available();
        virtual void setup();
        virtual void preprocess() {};
        virtual void process() {};
        virtual std::string name() { return GifName; };
        static bool setupForWrite(size_t numberOfBytes);
        static bool writePage(std::string& base64, size_t pageIndex);
        virtual void checkBeforeDraw(int32_t currentMsTime);
        SplashFrame * currentFrame = nullptr;
    private:
        void setCurrentFrameFor(uint32_t index);
        uint currentFrameIndex = 0;
        int32_t lastDrawTimestamp = 0;
};

#endif
