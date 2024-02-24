#ifndef _BUTTONLAYOUTSCREEN_H_
#define _BUTTONLAYOUTSCREEN_H_

#include <map>
#include <vector>
#include <string>
#include <deque>
#include <array>
#include <functional>
#include "GPGFX_UI_widgets.h"
#include "GPGFX_UI_layouts.h"

#define CHAR_TRIANGLE "\x80"
#define CHAR_CIRCLE   "\x81"
#define CHAR_CROSS    "\x82"
#define CHAR_SQUARE   "\x83"

#define CHAR_UP       "\x84"
#define CHAR_DOWN     "\x85"
#define CHAR_LEFT     "\x86"
#define CHAR_RIGHT    "\x87"

#define CHAR_UL       "\x88"
#define CHAR_UR       "\x89"
#define CHAR_DL       "\x8A"
#define CHAR_DR       "\x8B"

#define CHAR_HOME_S   "\x8C"
#define CHAR_CAP_S    "\x8D"

#define CHAR_VIEW_X   "\x8E"
#define CHAR_MENU_X   "\x8F"
#define CHAR_HOME_X   "\x90"

#define CHAR_TPAD_P   "\x91"
#define CHAR_HOME_P   "\x92"
#define CHAR_SHARE_P  "\x93"

#define INPUT_HISTORY_MAX_INPUTS 22

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
    protected:
        void drawScreen();
    private:
        // new layout methods
        GPLever* drawLever(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, uint16_t inputType);
        GPButton* drawButton(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, int16_t inputMask = -1);
        GPSprite* drawSprite(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY);
        GPShape* drawShape(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor);
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

        const std::map<uint16_t, uint16_t> displayModeLookup = {
            {INPUT_MODE_HID, 0},
            {INPUT_MODE_SWITCH, 1},
            {INPUT_MODE_XINPUT, 2},
            {INPUT_MODE_XBONE, 2},
            {INPUT_MODE_KEYBOARD, 3},
            {INPUT_MODE_CONFIG, 3}, 
            {INPUT_MODE_PS4, 4},
            {INPUT_MODE_PSCLASSIC, 4},
            {INPUT_MODE_MDMINI, 5},
            {INPUT_MODE_NEOGEO, 6},
            {INPUT_MODE_PCEMINI, 7},
            {INPUT_MODE_EGRET, 8},
            {INPUT_MODE_ASTRO, 9},
            {INPUT_MODE_XBOXORIGINAL, 10},
        };

        bool isInputHistoryEnabled = false;
        uint16_t inputHistoryX = 0;
        uint16_t inputHistoryY = 0;
        int16_t inputHistoryLength = 0;
        std::string historyString;
        std::deque<std::string> inputHistory;
        std::array<bool, INPUT_HISTORY_MAX_INPUTS> lastInput;

        uint8_t profileDelay = 5;
        int profileDelayStart = 0;
        bool displayProfileBanner = true;

        bool hasInitialized = false;

        uint16_t prevButtonState = 0;

        void showProfileBanner();

        uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
        void processInputHistory();
        bool pressedUp();
        bool pressedDown();
        bool pressedLeft();
        bool pressedRight();
        bool pressedUpLeft();
        bool pressedUpRight();
        bool pressedDownLeft();
        bool pressedDownRight();

        const std::vector<std::vector<std::string>> displayNames {
            {		// HID / DINPUT
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    CHAR_CROSS, CHAR_CIRCLE, CHAR_SQUARE, CHAR_TRIANGLE,
                    "L1", "R1", "L2", "R2",
                    "SL", "ST", "L3", "R3", "PS", "A2"
            },
            {		// Switch
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    "B", "A", "Y", "X",
                    "L", "R", "ZL", "ZR",
                    "-", "+", "LS", "RS", CHAR_HOME_S, CHAR_CAP_S
            },
            {		// XInput
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    "A", "B", "X", "Y",
                    "LB", "RB", "LT", "RT",
                    CHAR_VIEW_X, CHAR_MENU_X, "LS", "RS", CHAR_HOME_X, "A2"
            },
            {		// Keyboard / HID-KB
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    "B1", "B2", "B3", "B4",
                    "L1", "R1", "L2", "R2",
                    "S1", "S2", "L3", "R3", "A1", "A2"
            },
            {		// PS4
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    CHAR_CROSS, CHAR_CIRCLE, CHAR_SQUARE, CHAR_TRIANGLE,
                    "L1", "R1", "L2", "R2",
                    CHAR_SHARE_P, "OP", "L3", "R3", CHAR_HOME_P, CHAR_TPAD_P
            },
            {		// GEN/MD Mini
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    "A", "B", "X", "Y",
                    "", "Z", "", "C",
                    "M", "S", "", "", "", ""
            },
            {		// Neo Geo Mini
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    "B", "D", "A", "C",
                    "", "", "", "",
                    "SE", "ST", "", "", "", ""
            },
            {		// PC Engine/TG16 Mini
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    "I", "II", "", "",
                    "", "", "", "",
                    "SE", "RUN", "", "", "", ""
            },
            {		// Egret II Mini
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    "A", "B", "C", "D",
                    "", "E", "", "F",
                    "CRD", "ST", "", "", "MN", ""
            },
            {		// Astro City Mini
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    "A", "B", "D", "E",
                    "", "C", "", "F",
                    "CRD", "ST", "", "", "", ""
            },
            {		// Original Xbox
                    CHAR_UP, CHAR_DOWN, CHAR_LEFT, CHAR_RIGHT,
                    CHAR_UL, CHAR_UR, CHAR_DL, CHAR_DR,
                    "A", "B", "X", "Y",
                    "BL", "WH", "L", "R",
                    "BK", "ST", "LS", "RS", "", ""
            }
        };
};

#endif