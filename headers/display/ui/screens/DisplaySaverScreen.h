#ifndef _DISPLAYSAVERSCREEN_H_
#define _DISPLAYSAVERSCREEN_H_

#include <vector>
#include "GPGFX_UI_widgets.h"

const uint8_t SCREEN_WIDTH = 128;
const uint8_t SCREEN_HEIGHT = 64;

class DisplaySaverScreen : public GPScreen {
    public:
        DisplaySaverScreen() {}
        DisplaySaverScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual ~DisplaySaverScreen() {}
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();
    protected:
        virtual void drawScreen();
        uint16_t prevButtonState = 0;
        DisplaySaverMode displaySaverMode;

        // snow screen
        uint8_t snowflakeSpeeds[SCREEN_WIDTH][SCREEN_HEIGHT] = {};
        int8_t snowflakeDrift[SCREEN_WIDTH][SCREEN_HEIGHT] = {};
        void initSnowScene();
        void drawSnowScene();

        // bounce
        uint16_t bounceSpriteX = 0;
        uint16_t bounceSpriteY = 0;
        uint16_t bounceSpriteWidth = 128;
        uint16_t bounceSpriteHeight = 35;
        double bounceSpriteVelocityX = 1;
        double bounceSpriteVelocityY = 1;
        double bounceScale = 0.5;
        void drawBounceScene();

        // pipes
        void drawPipeScene();

        // toaster
        struct ToastParams {
            double scale;
            int16_t x;
            int16_t y;
            int16_t dx;
            int16_t dy;
        };

        std::vector<ToastParams> toasters;
        uint16_t numberOfToasters = 10;
        uint16_t toasterSpriteWidth = 128;
        uint16_t toasterSpriteHeight = 35;
        void initToasters();
        void drawToasterScene();
};

#endif