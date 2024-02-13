#include "ButtonLayoutScreen.h"

void ButtonLayoutScreen::drawScreen() {
    getRenderer()->drawText(0, 0, header);
    getRenderer()->drawText(0, 57, footer);
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawButtonLayoutLeft()
{
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    ButtonLayoutCustomOptions buttonLayoutCustomOptions = options.buttonLayoutCustomOptions;
    ButtonLayoutParamsLeft leftOptions = buttonLayoutCustomOptions.paramsLeft;
    return adjustByCustomSettings(GPButtonLayouts_LeftLayouts.at(leftOptions.layout)(), leftOptions.common);
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawButtonLayoutRight()
{
    const DisplayOptions& options = Storage::getInstance().getDisplayOptions();
    ButtonLayoutCustomOptions buttonLayoutCustomOptions = options.buttonLayoutCustomOptions;
    ButtonLayoutParamsRight rightOptions = buttonLayoutCustomOptions.paramsRight;
    return adjustByCustomSettings(GPButtonLayouts_RightLayouts.at(rightOptions.layout)(), rightOptions.common, 64);
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::adjustByCustomSettings(ButtonLayoutScreen::LayoutList layout, ButtonLayoutParamsCommon common, uint16_t originX, uint16_t originY) {
    if (layout.size() > 0) {
        int32_t startX = layout[0].parameters[0];
        int32_t startY = layout[0].parameters[1];
        int32_t customX = common.startX;
        int32_t customY = common.startY;
        int32_t offsetX = customX-startX;
        int32_t offsetY = customY-startY;
        for (uint16_t elementCtr = 0; elementCtr < layout.size(); elementCtr++) {
            if (layout[elementCtr].elementType == GP_ELEMENT_BUTTON) {
                layout[elementCtr].parameters[0] += originX+(offsetX+common.buttonPadding);
                layout[elementCtr].parameters[1] += originY+(offsetY+common.buttonPadding);
            } else {
                layout[elementCtr].parameters[0] += originX+offsetX;
                layout[elementCtr].parameters[1] += originY+offsetY;
            }

            if (((GPButton_Shape)layout[elementCtr].parameters[7] == GP_BUTTON_ELLIPSE) || ((GPButton_Shape)layout[elementCtr].parameters[7] == GP_BUTTON_POLYGON)) {
                // radius
                layout[elementCtr].parameters[2] = common.buttonRadius;
                layout[elementCtr].parameters[3] = common.buttonRadius;
            }
        }
    }
    return layout;
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawStickless()
{
	return {
        {GP_ELEMENT_DIR_BUTTON, {8,  20, 8, 8, 1, 0, GAMEPAD_MASK_LEFT,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {26, 20, 8, 8, 1, 0, GAMEPAD_MASK_DOWN,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {41, 29, 8, 8, 1, 0, GAMEPAD_MASK_RIGHT,   GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {48, 53, 8, 8, 1, 0, GAMEPAD_MASK_UP,      GP_BUTTON_ELLIPSE}}
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawWasdBox()
{
    return {
        {GP_ELEMENT_DIR_BUTTON, {8,  39, 18, 49, 1, 0, GAMEPAD_MASK_LEFT,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_DIR_BUTTON, {19, 39, 29, 49, 1, 0, GAMEPAD_MASK_DOWN,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_DIR_BUTTON, {19, 28, 29, 38, 1, 0, GAMEPAD_MASK_UP,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_DIR_BUTTON, {30, 39, 40, 49, 1, 0, GAMEPAD_MASK_RIGHT, GP_BUTTON_SQUARE}}
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawUDLR()
{
    return {
        {GP_ELEMENT_DIR_BUTTON, {8,  36, 7, 7, 1, 0, GAMEPAD_MASK_LEFT,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {25, 42, 7, 7, 1, 0, GAMEPAD_MASK_DOWN,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {33, 25, 7, 7, 1, 0, GAMEPAD_MASK_UP,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {42, 49, 7, 7, 1, 0, GAMEPAD_MASK_RIGHT,   GP_BUTTON_ELLIPSE}}
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawArcadeStick()
{
    return {{GP_ELEMENT_LEVER, {17, 37, 10, 10, 1, 0, 0}}};
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawVLXA()
{
    return {{GP_ELEMENT_LEVER, {15, 36,  8,  8, 1, 0, 0}}};
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawFightboardMirrored()
{
    return {
        {GP_ELEMENT_BUTTON,     {9,  18, 7, 7, 1, 0, GAMEPAD_MASK_L1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {25, 18, 7, 7, 1, 0, GAMEPAD_MASK_R1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {41, 18, 7, 7, 1, 0, GAMEPAD_MASK_B4,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {57, 27, 7, 7, 1, 0, GAMEPAD_MASK_B3,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {9,  34, 7, 7, 1, 0, GAMEPAD_MASK_L2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {25, 34, 7, 7, 1, 0, GAMEPAD_MASK_R2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {41, 34, 7, 7, 1, 0, GAMEPAD_MASK_B2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {57, 42, 7, 7, 1, 0, GAMEPAD_MASK_B1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {8,  46, 3, 3, 1, 0, GAMEPAD_MASK_L3,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {17, 46, 3, 3, 1, 0, GAMEPAD_MASK_S1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {26, 46, 3, 3, 1, 0, GAMEPAD_MASK_A1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {34, 46, 3, 3, 1, 0, GAMEPAD_MASK_S2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {44, 46, 3, 3, 1, 0, GAMEPAD_MASK_R3,      GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawFightboardStick()
{
    return {{GP_ELEMENT_LEVER, {27, 31, 10, 10, 1, 0, 0}}};
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawFightboardStickMirrored()
{
    return {
        {GP_ELEMENT_LEVER,      {99, 31, 10, 10, 1, 0, 0}}
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawTwinStickA()
{
    return {{GP_ELEMENT_LEVER, {17, 37, 10, 10, 1, 0, 0}}};
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawTwinStickB()
{
    return {
        {GP_ELEMENT_LEVER,      {109, 37, 10, 10, 1, 0}}
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawMAMEA()
{
    return {
        {GP_ELEMENT_DIR_BUTTON, {8,  37, 7, 7, 1, 0, GAMEPAD_MASK_LEFT,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {23, 24, 7, 7, 1, 0, GAMEPAD_MASK_UP,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {23, 50, 7, 7, 1, 0, GAMEPAD_MASK_DOWN,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {37, 37, 7, 7, 1, 0, GAMEPAD_MASK_RIGHT,   GP_BUTTON_ELLIPSE}}
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawOpenCore0WASDA()
{
    return {
        {GP_ELEMENT_DIR_BUTTON, {16, 39, 26, 49, 1, 0, GAMEPAD_MASK_LEFT,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_DIR_BUTTON, {27, 39, 37, 49, 1, 0, GAMEPAD_MASK_DOWN,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_DIR_BUTTON, {27, 28, 37, 38, 1, 0, GAMEPAD_MASK_UP,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_DIR_BUTTON, {38, 39, 48, 49, 1, 0, GAMEPAD_MASK_RIGHT, GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {6,  19,  3,  3, 1, 0, GAMEPAD_MASK_S1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {14, 19,  3,  3, 1, 0, GAMEPAD_MASK_S1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {23, 19,  3,  3, 1, 0, GAMEPAD_MASK_A1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {31, 19,  3,  3, 1, 0, GAMEPAD_MASK_A2,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {39, 19,  3,  3, 1, 0, GAMEPAD_MASK_L3,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {47, 19,  3,  3, 1, 0, GAMEPAD_MASK_R3,    GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawMAMEB()
{
    return {
        {GP_ELEMENT_BUTTON,     {68, 28, 78, 38, 1, 0, GAMEPAD_MASK_B3,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {79, 28, 89, 38, 1, 0, GAMEPAD_MASK_B4,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {90, 28, 100, 38, 1, 0, GAMEPAD_MASK_R1,   GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {68, 39, 78, 49, 1, 0, GAMEPAD_MASK_B1,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {79, 39, 89, 49, 1, 0, GAMEPAD_MASK_B2,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {90, 39, 100, 49, 1, 0, GAMEPAD_MASK_R2,   GP_BUTTON_SQUARE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawMAME8B()
{
    return {
        {GP_ELEMENT_BUTTON,     {68, 28, 78, 38, 1, 0, GAMEPAD_MASK_B3,   GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {79, 25, 89, 35, 1, 0, GAMEPAD_MASK_B4,   GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {90, 25, 100, 35, 1, 0, GAMEPAD_MASK_R1,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {101,28, 111, 38, 1, 0, GAMEPAD_MASK_L1,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {68, 39, 78, 49, 1, 0, GAMEPAD_MASK_B1,   GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {79, 36, 89, 46, 1, 0, GAMEPAD_MASK_B2,   GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {90, 36, 100, 46, 1, 0, GAMEPAD_MASK_R2,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {101,39, 111, 49, 1, 0, GAMEPAD_MASK_L2,  GP_BUTTON_SQUARE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawOpenCore0WASDB()
{
    return {
        {GP_ELEMENT_BUTTON,     {68, 28, 78, 38, 1, 0, GAMEPAD_MASK_B3,   GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {79, 25, 89, 35, 1, 0, GAMEPAD_MASK_B4,   GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {90, 25, 100, 35, 1, 0, GAMEPAD_MASK_R1,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {101,28, 111, 38, 1, 0, GAMEPAD_MASK_L1,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {68, 39, 78, 49, 1, 0, GAMEPAD_MASK_B1,   GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {79, 36, 89, 46, 1, 0, GAMEPAD_MASK_B2,   GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {90, 36, 100, 46, 1, 0, GAMEPAD_MASK_R2,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {101,39, 111, 49, 1, 0, GAMEPAD_MASK_L2,  GP_BUTTON_SQUARE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawKeyboardAngled()
{
    return {
        {GP_ELEMENT_DIR_BUTTON, {8,  37, 7, 7, 1, 0, GAMEPAD_MASK_LEFT,    GP_BUTTON_DIAMOND}},
        {GP_ELEMENT_DIR_BUTTON, {23, 24, 7, 7, 1, 0, GAMEPAD_MASK_UP,      GP_BUTTON_DIAMOND}},
        {GP_ELEMENT_DIR_BUTTON, {23, 50, 7, 7, 1, 0, GAMEPAD_MASK_DOWN,    GP_BUTTON_DIAMOND}},
        {GP_ELEMENT_DIR_BUTTON, {37, 37, 7, 7, 1, 0, GAMEPAD_MASK_RIGHT,   GP_BUTTON_DIAMOND}}
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawVewlix()
{
    return {
        {GP_ELEMENT_BUTTON,     {57, 31, 8, 8, 1, 0, GAMEPAD_MASK_B3,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {75, 24, 8, 8, 1, 0, GAMEPAD_MASK_B4,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 24, 8, 8, 1, 0, GAMEPAD_MASK_R1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {111,24, 8, 8, 1, 0, GAMEPAD_MASK_L1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {51, 49, 8, 8, 1, 0, GAMEPAD_MASK_B1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {69, 42, 8, 8, 1, 0, GAMEPAD_MASK_B2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {87, 42, 8, 8, 1, 0, GAMEPAD_MASK_R2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {105,42, 8, 8, 1, 0, GAMEPAD_MASK_L2,      GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawVLXB()
{
    return {
        {GP_ELEMENT_BUTTON,     {50, 31, 7, 7, 1, 0, GAMEPAD_MASK_B3,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {66, 24, 7, 7, 1, 0, GAMEPAD_MASK_B4,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {82, 24, 7, 7, 1, 0, GAMEPAD_MASK_R1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {98, 24, 7, 7, 1, 0, GAMEPAD_MASK_L1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {45, 47, 7, 7, 1, 0, GAMEPAD_MASK_B1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {61, 40, 7, 7, 1, 0, GAMEPAD_MASK_B2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {77, 40, 7, 7, 1, 0, GAMEPAD_MASK_R2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 40, 7, 7, 1, 0, GAMEPAD_MASK_L2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {119,33, 5, 5, 1, 0, GAMEPAD_MASK_S2,     GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawFightboard()
{
    return {
        {GP_ELEMENT_BUTTON,     {69, 27, 7, 7, 1, 0, GAMEPAD_MASK_B3,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {86, 18, 7, 7, 1, 0, GAMEPAD_MASK_B4,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {103,18, 7, 7, 1, 0, GAMEPAD_MASK_R1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {120,18, 7, 7, 1, 0, GAMEPAD_MASK_L1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {69, 43, 7, 7, 1, 0, GAMEPAD_MASK_B1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {86, 35, 7, 7, 1, 0, GAMEPAD_MASK_B2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {103,35, 7, 7, 1, 0, GAMEPAD_MASK_R2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {120,35, 7, 7, 1, 0, GAMEPAD_MASK_L2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {84, 47, 3, 3, 1, 0, GAMEPAD_MASK_L3,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {94, 47, 3, 3, 1, 0, GAMEPAD_MASK_S1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {103,47, 3, 3, 1, 0, GAMEPAD_MASK_A1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {112,47, 3, 3, 1, 0, GAMEPAD_MASK_S2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {122,47, 3, 3, 1, 0, GAMEPAD_MASK_R3,     GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawVewlix7()
{
    return {
        {GP_ELEMENT_BUTTON,     {57, 31, 8, 8, 1, 0, GAMEPAD_MASK_B3,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {75, 24, 8, 8, 1, 0, GAMEPAD_MASK_B4,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 24, 8, 8, 1, 0, GAMEPAD_MASK_R1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {111,24, 8, 8, 1, 0, GAMEPAD_MASK_L1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {51, 49, 8, 8, 1, 0, GAMEPAD_MASK_B1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {69, 42, 8, 8, 1, 0, GAMEPAD_MASK_B2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {87, 42, 8, 8, 1, 0, GAMEPAD_MASK_R2,      GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSega2p()
{
    return {
        {GP_ELEMENT_BUTTON,     {57, 34, 8, 8, 1, 0, GAMEPAD_MASK_B3,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {75, 24, 8, 8, 1, 0, GAMEPAD_MASK_B4,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 24, 8, 8, 1, 0, GAMEPAD_MASK_R1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {111,28, 8, 8, 1, 0, GAMEPAD_MASK_L1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {57, 52, 8, 8, 1, 0, GAMEPAD_MASK_B1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {75, 42, 8, 8, 1, 0, GAMEPAD_MASK_B2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 42, 8, 8, 1, 0, GAMEPAD_MASK_R2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {111,46, 8, 8, 1, 0, GAMEPAD_MASK_L2,      GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawNoir8()
{
    return {
        {GP_ELEMENT_BUTTON,     {57, 33, 8, 8, 1, 0, GAMEPAD_MASK_B3,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {75, 24, 8, 8, 1, 0, GAMEPAD_MASK_B4,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 24, 8, 8, 1, 0, GAMEPAD_MASK_R1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {111,28, 8, 8, 1, 0, GAMEPAD_MASK_L1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {57, 51, 8, 8, 1, 0, GAMEPAD_MASK_B1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {75, 42, 8, 8, 1, 0, GAMEPAD_MASK_B2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 42, 8, 8, 1, 0, GAMEPAD_MASK_R2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {111,46, 8, 8, 1, 0, GAMEPAD_MASK_L2,      GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawCapcom()
{
    return {
        {GP_ELEMENT_BUTTON,     {64, 28, 8, 8, 1, 0, GAMEPAD_MASK_B3,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {82, 28, 8, 8, 1, 0, GAMEPAD_MASK_B4,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {100,28, 8, 8, 1, 0, GAMEPAD_MASK_R1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {118,28, 8, 8, 1, 0, GAMEPAD_MASK_L1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {64, 46, 8, 8, 1, 0, GAMEPAD_MASK_B1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {82, 46, 8, 8, 1, 0, GAMEPAD_MASK_B2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {100,46, 8, 8, 1, 0, GAMEPAD_MASK_R2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {118,46, 8, 8, 1, 0, GAMEPAD_MASK_L2,      GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawCapcom6()
{
    return {
        {GP_ELEMENT_BUTTON,     {74, 28, 8, 8, 1, 0, GAMEPAD_MASK_B3,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {92, 28, 8, 8, 1, 0, GAMEPAD_MASK_B4,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {110,28, 8, 8, 1, 0, GAMEPAD_MASK_R1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {74, 46, 8, 8, 1, 0, GAMEPAD_MASK_B1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {92, 46, 8, 8, 1, 0, GAMEPAD_MASK_B2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {110,46, 8, 8, 1, 0, GAMEPAD_MASK_R2,      GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSticklessButtons()
{
    return {
        {GP_ELEMENT_BUTTON,     {57, 20,  8,  8, 1, 0, GAMEPAD_MASK_B3,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {75, 16,  8,  8, 1, 0, GAMEPAD_MASK_B4,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 16,  8,  8, 1, 0, GAMEPAD_MASK_R1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {111,20,  8,  8, 1, 0, GAMEPAD_MASK_L1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {57, 38,  8,  8, 1, 0, GAMEPAD_MASK_B1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {75, 34,  8,  8, 1, 0, GAMEPAD_MASK_B2,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 34,  8,  8, 1, 0, GAMEPAD_MASK_R2,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {111,38,  8,  8, 1, 0, GAMEPAD_MASK_L2,    GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawWasdButtons()
{
    return {
        {GP_ELEMENT_BUTTON,     {69, 28, 7, 7, 1, 0, GAMEPAD_MASK_B3,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {86, 24, 7, 7, 1, 0, GAMEPAD_MASK_B4,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {103,24, 7, 7, 1, 0, GAMEPAD_MASK_R1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {120,28, 7, 7, 1, 0, GAMEPAD_MASK_L1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {63, 45, 7, 7, 1, 0, GAMEPAD_MASK_B1,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {80, 41, 7, 7, 1, 0, GAMEPAD_MASK_B2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {97, 41, 7, 7, 1, 0, GAMEPAD_MASK_R2,      GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {114,45, 7, 7, 1, 0, GAMEPAD_MASK_L2,      GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawArcadeButtons()
{
    return {
        {GP_ELEMENT_BUTTON,     {64, 28,  8,  8, 1, 0, GAMEPAD_MASK_B3,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {82, 24,  8,  8, 1, 0, GAMEPAD_MASK_B4,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {100,24,  8,  8, 1, 0, GAMEPAD_MASK_R1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {118,28,  8,  8, 1, 0, GAMEPAD_MASK_L1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {59, 46,  8,  8, 1, 0, GAMEPAD_MASK_B1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {77, 42,  8,  8, 1, 0, GAMEPAD_MASK_B2,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {95, 42,  8,  8, 1, 0, GAMEPAD_MASK_R2,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {113,46,  8,  8, 1, 0, GAMEPAD_MASK_L2,    GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawStickless13A()
{
    return {
        {GP_ELEMENT_DIR_BUTTON, {39, 15,  6,  6, 1, 0, GAMEPAD_MASK_UP,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {18, 27,  6,  6, 1, 0, GAMEPAD_MASK_LEFT,  GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {32, 27,  6,  6, 1, 0, GAMEPAD_MASK_DOWN,  GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {44, 34,  6,  6, 1, 0, GAMEPAD_MASK_RIGHT, GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {49, 53,  6,  6, 1, 0, GAMEPAD_MASK_UP,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {65, 13,  2,  2, 1, 0, GAMEPAD_MASK_L3,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {72, 13,  2,  2, 1, 0, GAMEPAD_MASK_R3,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {79, 13,  2,  2, 1, 0, GAMEPAD_MASK_A2,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {86, 13,  2,  2, 1, 0, GAMEPAD_MASK_A1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {93, 13,  2,  2, 1, 0, GAMEPAD_MASK_S1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {100, 13, 2,  2, 1, 0, GAMEPAD_MASK_S2,    GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSticklessButtons13B()
{
    return {
        {GP_ELEMENT_BUTTON,     {56, 27, 6, 6, 1, 0, GAMEPAD_MASK_B3,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {70, 24, 6, 6, 1, 0, GAMEPAD_MASK_B4,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {84, 24, 6, 6, 1, 0, GAMEPAD_MASK_R1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {98, 27, 6, 6, 1, 0, GAMEPAD_MASK_L1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {56, 41, 6, 6, 1, 0, GAMEPAD_MASK_B1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {70, 38, 6, 6, 1, 0, GAMEPAD_MASK_B2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {84, 38, 6, 6, 1, 0, GAMEPAD_MASK_R2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {98, 41, 6, 6, 1, 0, GAMEPAD_MASK_L2,     GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawStickless16A()
{
    return {
        {GP_ELEMENT_DIR_BUTTON, {47, 19,  4,  4, 1, 0, GAMEPAD_MASK_UP,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {32, 27,  4,  4, 1, 0, GAMEPAD_MASK_LEFT,  GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {42, 27,  4,  4, 1, 0, GAMEPAD_MASK_DOWN,  GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {50, 32,  4,  4, 1, 0, GAMEPAD_MASK_RIGHT, GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {52, 47,  4,  4, 1, 0, GAMEPAD_MASK_UP,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {64, 17,  4,  4, 1, 0, GAMEPAD_MASK_L3,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {42, 45,  4,  4, 1, 0, GAMEPAD_MASK_L3,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {66, 45,  4,  4, 1, 0, GAMEPAD_MASK_R3,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {77, 15,  2,  2, 1, 0, GAMEPAD_MASK_A2,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {82, 15,  2,  2, 1, 0, GAMEPAD_MASK_A1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {87, 15,  2,  2, 1, 0, GAMEPAD_MASK_S1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {92, 15,  2,  2, 1, 0, GAMEPAD_MASK_S2,    GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSticklessButtons16B()
{
    return {
        {GP_ELEMENT_BUTTON,     {59, 27, 4, 4, 1, 0, GAMEPAD_MASK_B3,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {69, 25, 4, 4, 1, 0, GAMEPAD_MASK_B4,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {79, 25, 4, 4, 1, 0, GAMEPAD_MASK_R1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {89, 27, 4, 4, 1, 0, GAMEPAD_MASK_L1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {59, 37, 4, 4, 1, 0, GAMEPAD_MASK_B1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {69, 35, 4, 4, 1, 0, GAMEPAD_MASK_B2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {79, 35, 4, 4, 1, 0, GAMEPAD_MASK_R2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {89, 37, 4, 4, 1, 0, GAMEPAD_MASK_L2,     GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawStickless14A()
{
    return {
        {GP_ELEMENT_DIR_BUTTON, {26, 20,  7,  7, 1, 0, GAMEPAD_MASK_LEFT,  GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {42, 20,  7,  7, 1, 0, GAMEPAD_MASK_DOWN,  GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {56, 28,  7,  7, 1, 0, GAMEPAD_MASK_RIGHT, GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_DIR_BUTTON, {62, 50,  7,  7, 1, 0, GAMEPAD_MASK_UP,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {10, 22,  7,  7, 1, 0, GAMEPAD_MASK_L3,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {84, 50,  7,  7, 1, 0, GAMEPAD_MASK_R3,    GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawSticklessButtons14B()
{
    return {
        {GP_ELEMENT_BUTTON,     {70, 20, 7, 7, 1, 0, GAMEPAD_MASK_B3,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {86, 16, 7, 7, 1, 0, GAMEPAD_MASK_B4,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {102, 16, 7, 7, 1, 0, GAMEPAD_MASK_R1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {118, 20, 7, 7, 1, 0, GAMEPAD_MASK_L1,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {70, 36, 7, 7, 1, 0, GAMEPAD_MASK_B1,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {86, 32, 7, 7, 1, 0, GAMEPAD_MASK_B2,     GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {102, 32, 7, 7, 1, 0, GAMEPAD_MASK_R2,    GP_BUTTON_ELLIPSE}},
        {GP_ELEMENT_BUTTON,     {118, 36, 7, 7, 1, 0, GAMEPAD_MASK_L2,    GP_BUTTON_ELLIPSE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawDancepadA()
{
    return {
        {GP_ELEMENT_DIR_BUTTON, {39, 29, 54, 44, 1, 0, GAMEPAD_MASK_LEFT,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_DIR_BUTTON, {56, 46, 71, 61, 1, 0, GAMEPAD_MASK_DOWN,  GP_BUTTON_SQUARE}},
        {GP_ELEMENT_DIR_BUTTON, {56, 12, 71, 27, 1, 0, GAMEPAD_MASK_UP,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_DIR_BUTTON, {73, 29, 88, 44, 1, 0, GAMEPAD_MASK_RIGHT, GP_BUTTON_SQUARE}}
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawDancepadB()
{
    return {
        {GP_ELEMENT_BUTTON,     {39, 12, 54, 27, 1, 0, GAMEPAD_MASK_B2,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {39, 46, 54, 61, 1, 0, GAMEPAD_MASK_B4,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {73, 12, 88, 27, 1, 0, GAMEPAD_MASK_B1,    GP_BUTTON_SQUARE}},
        {GP_ELEMENT_BUTTON,     {73, 46, 88, 61, 1, 0, GAMEPAD_MASK_B3,    GP_BUTTON_SQUARE}},
    };
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBlankA()
{
    return {};
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBlankB()
{
    return {};
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBoardDefinedA() {
#ifdef DEFAULT_BOARD_LAYOUT_A
    return DEFAULT_BOARD_LAYOUT_A;
#else
    return drawBlankA();
#endif
}

ButtonLayoutScreen::LayoutList ButtonLayoutScreen::drawBoardDefinedB() {
#ifdef DEFAULT_BOARD_LAYOUT_B
    return DEFAULT_BOARD_LAYOUT_B;
#else
    return drawBlankA();
#endif
}


GPLever* ButtonLayoutScreen::drawLever(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, uint16_t inputType) {
    GPLever* lever = new GPLever();
    lever->setRenderer(getRenderer());
    
    lever->setPosition(startX, startY);
    lever->setStrokeColor(strokeColor);
    lever->setRadius(sizeX);
    lever->setInputType(inputType);

    return (GPLever*)addElement(lever);
}

GPButton* ButtonLayoutScreen::drawButton(uint16_t startX, uint16_t startY, uint16_t sizeX, uint16_t sizeY, uint16_t strokeColor, uint16_t fillColor, int16_t inputMask) {
    GPButton* button = new GPButton();
    button->setRenderer(getRenderer());

    button->setPosition(startX, startY);
    button->setStrokeColor(strokeColor);
    button->setSizeX(sizeX);
    button->setSizeY(sizeY);
    button->setInputMask(inputMask);

    return (GPButton*)addElement(button);
}

GPWidget* ButtonLayoutScreen::drawElement(GPButtonLayout element) {
    if (element.elementType == GP_ELEMENT_LEVER) {
        return drawLever(element.parameters[0], element.parameters[1], element.parameters[2], element.parameters[3], element.parameters[4], element.parameters[5], element.parameters[6]);
    } else if (element.elementType == GP_ELEMENT_BUTTON) {
        return drawButton(element.parameters[0], element.parameters[1], element.parameters[2], element.parameters[3], element.parameters[4], element.parameters[5], element.parameters[6])
            ->setInputDirection(false)
            ->setShape((GPButton_Shape)element.parameters[7]);
    } else if (element.elementType == GP_ELEMENT_DIR_BUTTON) {
        return drawButton(element.parameters[0], element.parameters[1], element.parameters[2], element.parameters[3], element.parameters[4], element.parameters[5], element.parameters[6])
            ->setInputDirection(true)
            ->setShape((GPButton_Shape)element.parameters[7]);
    }
    return NULL;
}

int8_t ButtonLayoutScreen::update() {
    GPWidget::update();
    bool configMode = getConfigMode();

    if (!hasInitialized) {
        // load layout
        uint16_t elementCtr = 0;
        uint16_t layoutLeft = Storage::getInstance().getDisplayOptions().buttonLayout;
        uint16_t layoutRight = Storage::getInstance().getDisplayOptions().buttonLayoutRight;
        LayoutList currLayoutLeft = GPButtonLayouts_LeftLayouts.at(layoutLeft)();
        LayoutList currLayoutRight = GPButtonLayouts_RightLayouts.at(layoutRight)();
        for (elementCtr = 0; elementCtr < currLayoutLeft.size(); elementCtr++) {
            drawElement(currLayoutLeft[elementCtr]);
        }
        for (elementCtr = 0; elementCtr < currLayoutRight.size(); elementCtr++) {
            drawElement(currLayoutRight[elementCtr]);
        }
        hasInitialized = true;
    }

    if (!configMode) {
        return DisplayMode::BUTTONS;
    } else {
        uint16_t buttonState = getGamepad()->state.buttons;

        if (prevButtonState && !buttonState) {
            if (prevButtonState == GAMEPAD_MASK_B1) {
                prevButtonState = 0;
                return -1;
            }
        }

        prevButtonState = buttonState;
  
        return DisplayMode::BUTTONS;
    }
}
