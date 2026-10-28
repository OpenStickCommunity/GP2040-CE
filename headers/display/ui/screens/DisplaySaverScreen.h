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
        DisplaySaverMode displaySaverMode{};

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
        static const uint8_t PIPE_CELL = 7;
        static const uint8_t PIPE_COLS = SCREEN_WIDTH / PIPE_CELL;
        static const uint8_t PIPE_ROWS = SCREEN_HEIGHT / PIPE_CELL;
        static const uint8_t PIPE_HEADS = 2;
        struct PipeHead {
            uint8_t x;
            uint8_t y;
            uint8_t dir;
            uint8_t length;
            bool alive;
        };
        uint8_t pipeCells[PIPE_ROWS][PIPE_COLS] = {};
        PipeHead pipeHeads[PIPE_HEADS] = {};
        uint16_t pipeFilled = 0;
        uint32_t pipeLastStep = 0;
        uint32_t pipeFullSince = 0;
        void initPipeScene();
        bool isPipeCellFree(int16_t x, int16_t y);
        bool spawnPipe(PipeHead& head);
        void stepPipe(PipeHead& head);
        void fillPipeRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
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