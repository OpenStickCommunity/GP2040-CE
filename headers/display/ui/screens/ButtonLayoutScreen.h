#ifndef _BUTTONLAYOUTSCREEN_H_
#define _BUTTONLAYOUTSCREEN_H_

#include "GPGFX_UI_widgets.h"
#include "GPGFX_UI_layouts.h"

class ButtonLayoutScreen : public GPScreen {
    public:
        typedef std::vector<GPButtonLayout> LayoutList;
        typedef std::function<LayoutList()> LayoutFunction;

        ButtonLayoutScreen() {}
        ButtonLayoutScreen(GPGFX* renderer) { setRenderer(renderer); }
        int8_t update();

        // old layout methods
        LayoutList drawStickless();
        LayoutList drawWasdBox();
        LayoutList drawArcadeStick();
        LayoutList drawUDLR();
        LayoutList drawMAMEA();
        LayoutList drawOpenCore0WASDA();
        LayoutList drawOpenCore0WASDB();
        LayoutList drawMAMEB();
        LayoutList drawMAME8B();
        LayoutList drawKeyboardAngled();
        LayoutList drawVewlix();
        LayoutList drawVewlix7();
        LayoutList drawSega2p();
        LayoutList drawNoir8();
        LayoutList drawCapcom();
        LayoutList drawCapcom6();
        LayoutList drawSticklessButtons();
        LayoutList drawWasdButtons();
        LayoutList drawArcadeButtons();
        LayoutList drawDancepadA();
        LayoutList drawDancepadB();
        LayoutList drawTwinStickA();
        LayoutList drawTwinStickB();
        LayoutList drawBlankA();
        LayoutList drawBlankB();
        LayoutList drawVLXA();
        LayoutList drawVLXB();
        LayoutList drawFightboard();
        LayoutList drawFightboardMirrored();
        LayoutList drawFightboardStick();
        LayoutList drawFightboardStickMirrored();
        LayoutList drawStickless13A();
        LayoutList drawSticklessButtons13B();
        LayoutList drawStickless16A();
        LayoutList drawSticklessButtons16B();
        LayoutList drawStickless14A();
        LayoutList drawSticklessButtons14B();
        LayoutList drawButtonLayoutLeft();
        LayoutList drawButtonLayoutRight();
        LayoutList drawBoardDefinedA();
        LayoutList drawBoardDefinedB();
    protected:
        void drawScreen();
    private:
        // new layout methods
        GPLever* drawLever(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, uint16_t inputType);
        GPButton* drawButton(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, int16_t inputMask = -1);
        GPWidget* drawElement(GPButtonLayout element);
        LayoutList adjustByCustomSettings(LayoutList layout, ButtonLayoutParamsCommon common, uint16_t originX = 0, uint16_t originY = 0);

        std::map<uint16_t,LayoutFunction> GPButtonLayouts_LeftLayouts = {
            // levers
            // parameters: x, y, radiusX/width, radiusY/height, stroke, fill, DpadMode
            {BUTTON_LAYOUT_STICK,                     [this]() { return this->drawArcadeStick(); }},
            {BUTTON_LAYOUT_TWINSTICKA,                [this]() { return this->drawTwinStickA(); }},
            {BUTTON_LAYOUT_VLXA,                      [this]() { return this->drawVLXA(); }},
            {BUTTON_LAYOUT_FIGHTBOARD_STICK,          [this]() { return this->drawFightboardStick(); }},

            // buttons
            // parameters: x, y, radiusX/width, radiusY/height, stroke, fill, Gamepad button mask, shape
            {BUTTON_LAYOUT_STICKLESS,                 [this]() { return this->drawStickless(); }},
            {BUTTON_LAYOUT_BUTTONS_ANGLED,            [this]() { return this->drawUDLR(); }},
            {BUTTON_LAYOUT_BUTTONS_BASIC,             [this]() { return this->drawMAMEA(); }},
            {BUTTON_LAYOUT_KEYBOARD_ANGLED,           [this]() { return this->drawKeyboardAngled(); }},
            {BUTTON_LAYOUT_KEYBOARDA,                 [this]() { return this->drawWasdBox(); }},
            {BUTTON_LAYOUT_DANCEPADA,                 [this]() { return this->drawDancepadA(); }},
            {BUTTON_LAYOUT_BLANKA,                    [this]() { return this->drawBlankA(); }},
            {BUTTON_LAYOUT_FIGHTBOARD_MIRRORED,       [this]() { return this->drawFightboardMirrored(); }},
            {BUTTON_LAYOUT_CUSTOMA,                   [this]() { return this->drawButtonLayoutLeft(); }},
            {BUTTON_LAYOUT_OPENCORE0WASDA,            [this]() { return this->drawOpenCore0WASDA(); }},
            {BUTTON_LAYOUT_STICKLESS_13,              [this]() { return this->drawStickless13A(); }},
            {BUTTON_LAYOUT_STICKLESS_16,              [this]() { return this->drawStickless16A(); }},
            {BUTTON_LAYOUT_STICKLESS_14,              [this]() { return this->drawStickless14A(); }},
            {BUTTON_LAYOUT_BOARD_DEFINED_A,           [this]() { return this->drawBoardDefinedA(); }},
        };

        std::map<uint16_t,LayoutFunction> GPButtonLayouts_RightLayouts = {
            {BUTTON_LAYOUT_ARCADE,                    [this]() { return this->drawArcadeButtons(); }},
            {BUTTON_LAYOUT_STICKLESSB,                [this]() { return this->drawSticklessButtons(); }},
            {BUTTON_LAYOUT_BUTTONS_ANGLEDB,           [this]() { return this->drawMAMEB(); }},
            {BUTTON_LAYOUT_VEWLIX,                    [this]() { return this->drawVewlix(); }},
            {BUTTON_LAYOUT_VEWLIX7,                   [this]() { return this->drawVewlix7(); }},
            {BUTTON_LAYOUT_CAPCOM,                    [this]() { return this->drawCapcom(); }},
            {BUTTON_LAYOUT_CAPCOM6,                   [this]() { return this->drawCapcom6(); }},
            {BUTTON_LAYOUT_SEGA2P,                    [this]() { return this->drawSega2p(); }},
            {BUTTON_LAYOUT_NOIR8,                     [this]() { return this->drawNoir8(); }},
            {BUTTON_LAYOUT_KEYBOARDB,                 [this]() { return this->drawMAMEB(); }},
            {BUTTON_LAYOUT_DANCEPADB,                 [this]() { return this->drawDancepadB(); }},
            {BUTTON_LAYOUT_TWINSTICKB,                [this]() { return this->drawTwinStickB(); }},
            {BUTTON_LAYOUT_BLANKB,                    [this]() { return this->drawBlankB(); }},
            {BUTTON_LAYOUT_VLXB,                      [this]() { return this->drawVLXB(); }},
            {BUTTON_LAYOUT_FIGHTBOARD,                [this]() { return this->drawFightboard(); }},
            {BUTTON_LAYOUT_FIGHTBOARD_STICK_MIRRORED, [this]() { return this->drawFightboardStickMirrored(); }},
            {BUTTON_LAYOUT_CUSTOMB,                   [this]() { return this->drawButtonLayoutRight(); }},
            {BUTTON_LAYOUT_KEYBOARD8B,                [this]() { return this->drawMAME8B(); }},
            {BUTTON_LAYOUT_OPENCORE0WASDB,            [this]() { return this->drawOpenCore0WASDB(); }},
            {BUTTON_LAYOUT_STICKLESS_13B,             [this]() { return this->drawSticklessButtons13B(); }},
            {BUTTON_LAYOUT_STICKLESS_16B,             [this]() { return this->drawSticklessButtons16B(); }},
            {BUTTON_LAYOUT_STICKLESS_14B,             [this]() { return this->drawSticklessButtons14B(); }},
            {BUTTON_LAYOUT_BOARD_DEFINED_B,           [this]() { return this->drawBoardDefinedB(); }},
        };


        bool isInputHistoryEnabled = false;
        bool hasInitialized = false;

        uint16_t prevButtonState = 0;

        uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
};

#endif