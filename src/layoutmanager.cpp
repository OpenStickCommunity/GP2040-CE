#include "layoutmanager.h"
#include "storagemanager.h"
#include "buttonlayouts.h"
#include "enums.pb.h"

LayoutManager::LayoutList LayoutManager::getLayoutA() {
    uint16_t layoutLeft = Storage::getInstance().getDisplayOptions().buttonLayout;
    return GPButtonLayouts_LeftLayouts.at(layoutLeft)();
}

LayoutManager::LayoutList LayoutManager::getLayoutB() {
    uint16_t layoutRight = Storage::getInstance().getDisplayOptions().buttonLayoutRight;
    return GPButtonLayouts_RightLayouts.at(layoutRight)();
}

std::string LayoutManager::getLayoutAName() {
    uint16_t layoutLeft = Storage::getInstance().getDisplayOptions().buttonLayout;
    return getButtonLayoutName((ButtonLayout)layoutLeft);
}

std::string LayoutManager::getLayoutBName() {
    uint16_t layoutRight = Storage::getInstance().getDisplayOptions().buttonLayoutRight;
    return getButtonLayoutRightName((ButtonLayoutRight)layoutRight);
}

std::string LayoutManager::getButtonLayoutName(ButtonLayout layout) {
    #define ENUM_CASE(name, value) case name: return #name;
    switch (layout) {
        ButtonLayout_VALUELIST(ENUM_CASE)
        default: return "BUTTON_LAYOUT_UNKNOWN";
    }
    #undef ENUM_CASE
}

std::string LayoutManager::getButtonLayoutRightName(ButtonLayoutRight layout) {
    #define ENUM_CASE(name, value) case name: return #name;
    switch (layout) {
        ButtonLayoutRight_VALUELIST(ENUM_CASE)
        default: return "BUTTON_LAYOUT_UNKNOWN";
    }
    #undef ENUM_CASE
}

LayoutManager::LayoutList LayoutManager::drawButtonLayoutLeft()
{
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    ButtonLayoutCustomOptions buttonLayoutCustomOptions = options.buttonLayoutCustomOptions;
    ButtonLayoutParamsLeft leftOptions = buttonLayoutCustomOptions.paramsLeft;
    return adjustByCustomSettings(GPButtonLayouts_LeftLayouts.at(leftOptions.layout)(), leftOptions.common);
}

LayoutManager::LayoutList LayoutManager::drawButtonLayoutRight()
{
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    ButtonLayoutCustomOptions buttonLayoutCustomOptions = options.buttonLayoutCustomOptions;
    ButtonLayoutParamsRight rightOptions = buttonLayoutCustomOptions.paramsRight;
    return adjustByCustomSettings(GPButtonLayouts_RightLayouts.at(rightOptions.layout)(), rightOptions.common, 64);
}

LayoutManager::LayoutList LayoutManager::adjustByCustomSettings(LayoutManager::LayoutList layout, ButtonLayoutParamsCommon common, uint16_t originX, uint16_t originY) {
    if (layout.size() > 0) {
        int32_t startX = layout[0].parameters.x1;
        int32_t startY = layout[0].parameters.y1;
        int32_t customX = common.startX;
        int32_t customY = common.startY;
        int32_t offsetX = customX-startX;
        int32_t offsetY = customY-startY;
        for (uint16_t elementCtr = 0; elementCtr < layout.size(); elementCtr++) {
            if (layout[elementCtr].elementType == GP_ELEMENT_BTN_BUTTON) {
                layout[elementCtr].parameters.x1 += originX+(offsetX+common.buttonPadding);
                layout[elementCtr].parameters.y1 += originY+(offsetY+common.buttonPadding);
            } else {
                layout[elementCtr].parameters.x1 += originX+offsetX;
                layout[elementCtr].parameters.y1 += originY+offsetY;
            }

            if (((GPShape_Type)layout[elementCtr].parameters.shape == GP_SHAPE_ELLIPSE) || ((GPShape_Type)layout[elementCtr].parameters.shape == GP_SHAPE_POLYGON)) {
                // radius
                layout[elementCtr].parameters.x2 = common.buttonRadius;
                layout[elementCtr].parameters.y2 = common.buttonRadius;
            }
        }
    }
    return layout;
}

LayoutManager::LayoutList LayoutManager::drawStickless()
{
	return BUTTON_GROUP_STICKLESS;
}

LayoutManager::LayoutList LayoutManager::drawWasdBox()
{
    return BUTTON_GROUP_WASD_BOX;
}

LayoutManager::LayoutList LayoutManager::drawUDLR()
{
    return BUTTON_GROUP_UDLR;
}

LayoutManager::LayoutList LayoutManager::drawArcadeStick()
{
    return BUTTON_GROUP_ARCADE_STICK;
}

LayoutManager::LayoutList LayoutManager::drawVLXA()
{
    return BUTTON_GROUP_VLXA;
}

LayoutManager::LayoutList LayoutManager::drawFightboardMirrored()
{
    return BUTTON_GROUP_FIGHTBOARD_MIRRORED;
}

LayoutManager::LayoutList LayoutManager::drawFightboardStick()
{
    return BUTTON_GROUP_FIGHTBOARD_STICK;
}

LayoutManager::LayoutList LayoutManager::drawFightboardStickMirrored()
{
    return BUTTON_GROUP_FIGHTBOARD_STICK_MIRRORED;
}

LayoutManager::LayoutList LayoutManager::drawTwinStickA()
{
    return BUTTON_GROUP_TWINSTICK_A;
}

LayoutManager::LayoutList LayoutManager::drawTwinStickB()
{
    return BUTTON_GROUP_TWINSTICK_B;
}

LayoutManager::LayoutList LayoutManager::drawMAMEA()
{
    return BUTTON_GROUP_MAME_A;
}

LayoutManager::LayoutList LayoutManager::drawOpenCore0WASDA()
{
    return BUTTON_GROUP_OPEN_CORE_WASD_A;
}

LayoutManager::LayoutList LayoutManager::drawMAMEB()
{
    return BUTTON_GROUP_MAME_B;
}

LayoutManager::LayoutList LayoutManager::drawMAME8B()
{
    return BUTTON_GROUP_MAME_8B;
}

LayoutManager::LayoutList LayoutManager::drawOpenCore0WASDB()
{
    return BUTTON_GROUP_OPEN_CORE_WASD_B;
}

LayoutManager::LayoutList LayoutManager::drawKeyboardAngled()
{
    return BUTTON_GROUP_KEYBOARD_ANGLED;
}

LayoutManager::LayoutList LayoutManager::drawVewlix()
{
    return BUTTON_GROUP_VEWLIX;
}

LayoutManager::LayoutList LayoutManager::drawVLXB()
{
    return BUTTON_GROUP_VLXB;
}

LayoutManager::LayoutList LayoutManager::drawFightboard()
{
    return BUTTON_GROUP_FIGHTBOARD;
}

LayoutManager::LayoutList LayoutManager::drawVewlix7()
{
    return BUTTON_GROUP_VEWLIX7;
}

LayoutManager::LayoutList LayoutManager::drawSega2p()
{
    return BUTTON_GROUP_SEGA_2P;
}

LayoutManager::LayoutList LayoutManager::drawNoir8()
{
    return BUTTON_GROUP_NOIR8;
}

LayoutManager::LayoutList LayoutManager::drawCapcom()
{
    return BUTTON_GROUP_CAPCOM;
}

LayoutManager::LayoutList LayoutManager::drawCapcom6()
{
    return BUTTON_GROUP_CAPCOM6;
}

LayoutManager::LayoutList LayoutManager::drawSticklessButtons()
{
    return BUTTON_GROUP_STICKLESS_BUTTONS;
}

LayoutManager::LayoutList LayoutManager::drawWasdButtons()
{
    return BUTTON_GROUP_WASD_BUTTONS;
}

LayoutManager::LayoutList LayoutManager::drawArcadeButtons()
{
    return BUTTON_GROUP_ARCADE_BUTTONS;
}

LayoutManager::LayoutList LayoutManager::drawStickless13A()
{
    return BUTTON_GROUP_STICKLESS13A;
}

LayoutManager::LayoutList LayoutManager::drawSticklessButtons13B()
{
    return BUTTON_GROUP_STICKLESS_BUTTONS13B;
}

LayoutManager::LayoutList LayoutManager::drawStickless16A()
{
    return BUTTON_GROUP_STICKLESS16A;
}

LayoutManager::LayoutList LayoutManager::drawSticklessButtons16B()
{
    return BUTTON_GROUP_STICKLESS_BUTTONS16B;
}

LayoutManager::LayoutList LayoutManager::drawStickless14A()
{
    return BUTTON_GROUP_STICKLESS14A;
}

LayoutManager::LayoutList LayoutManager::drawSticklessButtons14B()
{
    return BUTTON_GROUP_STICKLESS_BUTTONS14B;
}

LayoutManager::LayoutList LayoutManager::drawDancepadA()
{
    return BUTTON_GROUP_DANCEPAD_A;
}

LayoutManager::LayoutList LayoutManager::drawDancepadB()
{
    return BUTTON_GROUP_DANCEPAD_B;
}

LayoutManager::LayoutList LayoutManager::drawDancepadDDRLeft()
{
    return BUTTON_GROUP_DANCEPAD_DDR_LEFT;
}

LayoutManager::LayoutList LayoutManager::drawDancepadDDRSolo()
{
    return BUTTON_GROUP_DANCEPAD_DDR_SOLO;
}

LayoutManager::LayoutList LayoutManager::drawDancepadPIULeft()
{
    return BUTTON_GROUP_DANCEPAD_PIU_LEFT;
}

LayoutManager::LayoutList LayoutManager::drawPopnA()
{
    return BUTTON_GROUP_POPN_A;
}

LayoutManager::LayoutList LayoutManager::drawTaikoA()
{
    return BUTTON_GROUP_TAIKO_A;
}

LayoutManager::LayoutList LayoutManager::drawBMTurntableA()
{
    return BUTTON_GROUP_BM_TURNTABLE_A;
}

LayoutManager::LayoutList LayoutManager::drawBM5KeyA()
{
    return BUTTON_GROUP_BM_5KEY_A;
}

LayoutManager::LayoutList LayoutManager::drawBM7KeyA()
{
    return BUTTON_GROUP_BM_7KEY_A;
}

LayoutManager::LayoutList LayoutManager::drawGitadoraFretA()
{
    return BUTTON_GROUP_GITADORA_FRET_A;
}

LayoutManager::LayoutList LayoutManager::drawGitadoraStrumA()
{
    return BUTTON_GROUP_GITADORA_STRUM_A;
}

LayoutManager::LayoutList LayoutManager::drawDancepadDDRRight()
{
    return BUTTON_GROUP_DANCEPAD_DDR_RIGHT;
}

LayoutManager::LayoutList LayoutManager::drawDancepadPIURight()
{
    return BUTTON_GROUP_DANCEPAD_PIU_RIGHT;
}

LayoutManager::LayoutList LayoutManager::drawPopnB()
{
    return BUTTON_GROUP_POPN_B;
}

LayoutManager::LayoutList LayoutManager::drawTaikoB()
{
    return BUTTON_GROUP_TAIKO_B;
}

LayoutManager::LayoutList LayoutManager::drawBMTurntableB()
{
    return BUTTON_GROUP_BM_TURNTABLE_B;
}

LayoutManager::LayoutList LayoutManager::drawBM5KeyB()
{
    return BUTTON_GROUP_BM_5KEY_B;
}

LayoutManager::LayoutList LayoutManager::drawBM7KeyB()
{
    return BUTTON_GROUP_BM_7KEY_B;
}

LayoutManager::LayoutList LayoutManager::drawGitadoraFretB()
{
    return BUTTON_GROUP_GITADORA_FRET_B;
}

LayoutManager::LayoutList LayoutManager::drawGitadoraStrumB()
{
    return BUTTON_GROUP_GITADORA_STRUM_B;
}

LayoutManager::LayoutList LayoutManager::drawBlankA()
{
    return {};
}

LayoutManager::LayoutList LayoutManager::drawBlankB()
{
    return {};
}

LayoutManager::LayoutList LayoutManager::drawBoardDefinedA() {
#ifdef DEFAULT_BOARD_LAYOUT_A
    return DEFAULT_BOARD_LAYOUT_A;
#else
    return drawBlankA();
#endif
}

LayoutManager::LayoutList LayoutManager::drawBoardDefinedB() {
#ifdef DEFAULT_BOARD_LAYOUT_B
    return DEFAULT_BOARD_LAYOUT_B;
#else
    return drawBlankA();
#endif
}