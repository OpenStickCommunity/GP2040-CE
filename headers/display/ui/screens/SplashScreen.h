#ifndef _SPLASHSCREEN_H_
#define _SPLASHSCREEN_H_

#include "GPGFX_UI_widgets.h"

#define SPLASH_SCREEN_WIDTH 128
#define SPLASH_SCREEN_HEIGHT 64
#define SPLASH_TEXT_CHAR_WIDTH 6
#define SPLASH_TEXT_LINE_HEIGHT 8
#define SPLASH_TEXT_LINE_GAP 4

class SplashScreen : public GPScreen {
    public:
        SplashScreen() {}
        SplashScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual ~SplashScreen(){}
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();
    protected:
        virtual void drawScreen();
        void drawSplashText();
        bool hasSecondStage();
        void showSecondStage();
        uint16_t prevButtonState = 0;
        uint32_t splashStartTime = 0;
        bool configMode = false;
        bool showingText = false;
        bool onSecondStage = false;
};

#endif