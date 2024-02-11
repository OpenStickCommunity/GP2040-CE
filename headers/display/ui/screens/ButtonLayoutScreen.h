#ifndef _BUTTONLAYOUTSCREEN_H_
#define _BUTTONLAYOUTSCREEN_H_

#include "GPGFX_UI_widgets.h"
#include "GPGFX_UI_layouts.h"

class ButtonLayoutScreen : public GPScreen {
    public:
        ButtonLayoutScreen() {}
        ButtonLayoutScreen(GPGFX* renderer) { setRenderer(renderer); }
        int8_t update();
    protected:
        void drawScreen();
    private:
        // new layout methods
        void drawLever(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, uint16_t inputType);
        GPButton* drawButton(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, int16_t inputMask = -1);
        void drawElement(GPButtonLayout element);

        // old layout methods
        void drawStickless(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawWasdBox(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawArcadeStick(int startX, int startY, int buttonRadius, int buttonPadding);
        //Adding my stuff here, remember to sort before PR
        void drawDiamond(int x, int y, int size, uint8_t color, uint8_t filled);
        void drawUDLR(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawMAMEA(int startX, int startY, int buttonSize, int buttonPadding);
        void drawOpenCore0WASDA(int startX, int startY, int buttonSize, int buttonPadding);
        void drawOpenCore0WASDB(int startX, int startY, int buttonSize, int buttonPadding);
        void drawMAMEB(int startX, int startY, int buttonSize, int buttonPadding);
        void drawMAME8B(int startX, int startY, int buttonSize, int buttonPadding);
        void drawKeyboardAngled(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawVewlix(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawVewlix7(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawSega2p(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawNoir8(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawCapcom(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawCapcom6(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawSticklessButtons(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawWasdButtons(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawArcadeButtons(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawDancepadA(int startX, int startY, int buttonSize, int buttonPadding);
        void drawDancepadB(int startX, int startY, int buttonSize, int buttonPadding);
        void drawTwinStickA(int startX, int startY, int buttonSize, int buttonPadding);
        void drawTwinStickB(int startX, int startY, int buttonSize, int buttonPadding);
        void drawBlankA(int startX, int startY, int buttonSize, int buttonPadding);
        void drawBlankB(int startX, int startY, int buttonSize, int buttonPadding);
        void drawVLXA(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawVLXB(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawButtonLayoutLeft(ButtonLayoutParamsLeft& options);
        void drawButtonLayoutRight(ButtonLayoutParamsRight& options);
        void drawFightboard(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawFightboardMirrored(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawFightboardStick(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawFightboardStickMirrored(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawStickless13A(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawSticklessButtons13B(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawStickless16A(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawSticklessButtons16B(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawStickless14A(int startX, int startY, int buttonRadius, int buttonPadding);
        void drawSticklessButtons14B(int startX, int startY, int buttonRadius, int buttonPadding);
        bool isInputHistoryEnabled = false;
        bool hasInitialized = false;

        uint16_t prevButtonState = 0;

        uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
};

#endif