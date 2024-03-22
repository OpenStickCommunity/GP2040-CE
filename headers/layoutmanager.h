#ifndef _LAYOUTMANAGER_H_
#define _LAYOUTMANAGER_H_

#include <map>
#include <vector>
#include <string>
#include <deque>
#include <array>
#include <functional>
#include "config.pb.h"
#include "enums.pb.h"

typedef struct {
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    uint16_t stroke;
    uint16_t fill;
    uint16_t value;
    uint16_t shape;
    uint16_t angleStart;
    uint16_t angleEnd;
    uint16_t closed;
} GPButtonParameters;

typedef struct {
    GPElement elementType;
    GPButtonParameters parameters;
} GPButtonLayout;

#define LAYOUTMGR LayoutManager::getInstance()

class LayoutManager {
    public:
        typedef std::vector<GPButtonLayout> LayoutList;
        typedef std::function<LayoutList()> LayoutFunction;

        LayoutManager(LayoutManager const&) = delete;
        void operator=(LayoutManager const&)  = delete;
        static LayoutManager& getInstance() // Thread-safe storage ensures cross-thread talk
        {
            static LayoutManager instance;
            return instance;
        }

        LayoutList getLayoutA();
        LayoutList getLayoutB();

        std::string getLayoutAName();
        std::string getLayoutBName();

        std::string getButtonLayoutName(ButtonLayout layout);
        std::string getButtonLayoutRightName(ButtonLayoutRight layout);

        LayoutList adjustByCustomSettings(LayoutList layout, ButtonLayoutParamsCommon common, uint16_t originX = 0, uint16_t originY = 0);

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
        LayoutList drawDancepadDDRLeft();
        LayoutList drawDancepadDDRSolo();
        LayoutList drawDancepadPIULeft();
        LayoutList drawPopnA();
        LayoutList drawTaikoA();
        LayoutList drawBMTurntableA();
        LayoutList drawBM5KeyA();
        LayoutList drawBM7KeyA();
        LayoutList drawGitadoraFretA();
        LayoutList drawGitadoraStrumA();
        LayoutList drawDancepadDDRRight();
        LayoutList drawDancepadPIURight();
        LayoutList drawPopnB();
        LayoutList drawTaikoB();
        LayoutList drawBMTurntableB();
        LayoutList drawBM5KeyB();
        LayoutList drawBM7KeyB();
        LayoutList drawGitadoraFretB();
        LayoutList drawGitadoraStrumB();
    private:
        LayoutManager(){}

        std::string getLayoutNameByID();

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
            {BUTTON_LAYOUT_DANCEPADA,                 [this]() { return this->drawDancepadA(); }},

            {BUTTON_LAYOUT_DANCEPAD_DDR_LEFT,         [this]() { return this->drawDancepadDDRLeft(); }},
            {BUTTON_LAYOUT_DANCEPAD_DDR_SOLO,         [this]() { return this->drawDancepadDDRSolo(); }},
            {BUTTON_LAYOUT_DANCEPAD_PIU_LEFT,         [this]() { return this->drawDancepadPIULeft(); }},
            {BUTTON_LAYOUT_POPN_A,                    [this]() { return this->drawPopnA(); }},
            {BUTTON_LAYOUT_TAIKO_A,                   [this]() { return this->drawTaikoA(); }},
            {BUTTON_LAYOUT_BM_TURNTABLE_A,            [this]() { return this->drawBMTurntableA(); }},
            {BUTTON_LAYOUT_BM_5KEY_A,                 [this]() { return this->drawBM5KeyA(); }},
            {BUTTON_LAYOUT_BM_7KEY_A,                 [this]() { return this->drawBM7KeyA(); }},
            {BUTTON_LAYOUT_GITADORA_FRET_A,           [this]() { return this->drawGitadoraFretA(); }},
            {BUTTON_LAYOUT_GITADORA_STRUM_A,          [this]() { return this->drawGitadoraStrumA(); }},

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
            {BUTTON_LAYOUT_DANCEPADB,                 [this]() { return this->drawDancepadB(); }},

            {BUTTON_LAYOUT_DANCEPAD_DDR_RIGHT,        [this]() { return this->drawDancepadDDRRight(); }},
            {BUTTON_LAYOUT_DANCEPAD_PIU_RIGHT,        [this]() { return this->drawDancepadPIURight(); }},
            {BUTTON_LAYOUT_POPN_B,                    [this]() { return this->drawPopnB(); }},
            {BUTTON_LAYOUT_TAIKO_B,                   [this]() { return this->drawTaikoB(); }},
            {BUTTON_LAYOUT_BM_TURNTABLE_B,            [this]() { return this->drawBMTurntableB(); }},
            {BUTTON_LAYOUT_BM_5KEY_B,                 [this]() { return this->drawBM5KeyB(); }},
            {BUTTON_LAYOUT_BM_7KEY_B,                 [this]() { return this->drawBM7KeyB(); }},
            {BUTTON_LAYOUT_GITADORA_FRET_B,           [this]() { return this->drawGitadoraFretB(); }},
            {BUTTON_LAYOUT_GITADORA_STRUM_B,          [this]() { return this->drawGitadoraStrumB(); }},

            {BUTTON_LAYOUT_BOARD_DEFINED_B,           [this]() { return this->drawBoardDefinedB(); }},
        };
};

#endif