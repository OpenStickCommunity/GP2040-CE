#ifndef _BUTTONLAYOUTSCREEN_H_
#define _BUTTONLAYOUTSCREEN_H_

#include <map>
#include <vector>
#include <string>
#include <deque>
#include <array>
#include <functional>
#include "layoutmanager.h"
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
#define INPUT_HISTORY_MAX_MODES 11

// Static to ensure memory is never doubled
static const char * displayNames[INPUT_HISTORY_MAX_MODES][INPUT_HISTORY_MAX_INPUTS] = {
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

class ButtonLayoutScreen : public GPScreen {
    public:
        ButtonLayoutScreen() {}
        ButtonLayoutScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();
    protected:
        virtual void drawScreen();
    private:
        // new layout methods
        GPLever* addLever(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, uint16_t inputType);
        GPButton* addButton(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, int16_t inputMask = -1);
        GPSprite* addSprite(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY);
        GPShape* addShape(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor);
        GPWidget* pushElement(GPButtonLayout element);
        void generateHeader();

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

        Gamepad* gamepad;
        InputMode inputMode;
        std::string statusBar;
        std::string footer;

        bool isInputHistoryEnabled = false;
        uint16_t inputHistoryX = 0;
        uint16_t inputHistoryY = 0;
        size_t inputHistoryLength = 0;
        std::string historyString;
        std::deque<std::string> inputHistory;
        std::array<bool, INPUT_HISTORY_MAX_INPUTS> lastInput;

        bool profileModeDisplay;
        uint8_t profileDelay = 2;
        int profileDelayStart = 0;
        uint16_t prevButtonState = 0;
        uint8_t prevLayoutLeft = 0;
        uint8_t prevLayoutRight = 0;
        uint8_t prevProfileNumber = 0;
        ButtonLayoutParamsLeft prevLeftOptions;
        ButtonLayoutParamsRight prevRightOptions;

        bool macroEnabled;

        uint16_t map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max);
        void processInputHistory();
        bool compareCustomLayouts();
        bool pressedUp();
        bool pressedDown();
        bool pressedLeft();
        bool pressedRight();
        bool pressedUpLeft();
        bool pressedUpRight();
        bool pressedDownLeft();
        bool pressedDownRight();
};

#endif