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

#include "buttonlayouts.h"

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
        LayoutList adjustByOffset(LayoutList layout, int16_t originX = 0, int16_t originY = 0);
        LayoutList flipHorizontally(LayoutList layout, int16_t startX = 0, int16_t startY = 0, int16_t endX = 0, int16_t endY = 0);

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
        LayoutList drawSega2p6b();
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
        LayoutList drawVLXB6B();
        LayoutList drawFightboard();
        LayoutList drawFightboardMirrored();
        LayoutList drawFightboardStick();
        LayoutList drawFightboardStickMirrored();
        LayoutList drawStickless13A();
        LayoutList drawSticklessButtons13B();
        LayoutList drawStickless16A();
        LayoutList drawSticklessButtons16B();
        LayoutList drawSticklessR16A();
        LayoutList drawSticklessButtonsR16B();
        LayoutList drawStickless14A();
        LayoutList drawSticklessButtons14B();
        LayoutList drawButtonLayoutLeft();
        LayoutList drawButtonLayoutRight();
        LayoutList drawBoardDefinedA();
        LayoutList drawBoardDefinedAlt0A();
        LayoutList drawBoardDefinedAlt1A();
        LayoutList drawBoardDefinedAlt2A();
        LayoutList drawBoardDefinedAlt3A();
        LayoutList drawBoardDefinedAlt4A();
        LayoutList drawBoardDefinedAlt5A();
        LayoutList drawBoardDefinedB();
        LayoutList drawBoardDefinedAlt0B();
        LayoutList drawBoardDefinedAlt1B();
        LayoutList drawBoardDefinedAlt2B();
        LayoutList drawBoardDefinedAlt3B();
        LayoutList drawBoardDefinedAlt4B();
        LayoutList drawBoardDefinedAlt5B();
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
        LayoutList drawBandHeroFretA();
        LayoutList drawBandHeroStrumA();
        LayoutList draw6GAWDLeft();
        LayoutList draw6GAWDAllButtonLeft();
        LayoutList draw6GAWDAllButtonPlusLeft();
        LayoutList drawDancepadDDRRight();
        LayoutList drawDancepadPIURight();
        LayoutList drawPopnB();
        LayoutList drawTaikoB();
        LayoutList drawBMTurntableB();
        LayoutList drawBM5KeyB();
        LayoutList drawBM7KeyB();
        LayoutList drawGitadoraFretB();
        LayoutList drawGitadoraStrumB();
        LayoutList drawBandHeroFretB();
        LayoutList drawBandHeroStrumB();
        LayoutList draw6GAWDRight();
        LayoutList draw6GAWDAllButtonRight();
        LayoutList draw6GAWDAllButtonPlusRight();

        LayoutManager::LayoutList getLeftLayout(uint16_t index);
        LayoutManager::LayoutList getRightLayout(uint16_t index);
    private:
        LayoutManager(){}

        std::string getLayoutNameByID();
};

#endif