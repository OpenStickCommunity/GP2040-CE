#ifndef _PINVIEWERSCREEN_H_
#define _PINVIEWERSCREEN_H_

#include <map>
#include "GPGFX_UI_widgets.h"

class PinViewerScreen : public GPScreen {
    public:
        PinViewerScreen() {}
        PinViewerScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual ~PinViewerScreen(){}
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();
    protected:
        virtual void drawScreen();
        uint16_t prevButtonState = 0;

        std::map<GpioAction,const char*> buttonLookup = {
            {GpioAction::BUTTON_PRESS_UP, CHAR_UP},
            {GpioAction::BUTTON_PRESS_DOWN, CHAR_DOWN},
            {GpioAction::BUTTON_PRESS_LEFT, CHAR_LEFT},
            {GpioAction::BUTTON_PRESS_RIGHT, CHAR_RIGHT},
            {GpioAction::BUTTON_PRESS_B1, "B1"},
            {GpioAction::BUTTON_PRESS_B2, "B2"},
            {GpioAction::BUTTON_PRESS_B3, "B3"},
            {GpioAction::BUTTON_PRESS_B4, "B4"},
            {GpioAction::BUTTON_PRESS_L1, "L1"},
            {GpioAction::BUTTON_PRESS_R1, "R1"},
            {GpioAction::BUTTON_PRESS_L2, "L2"},
            {GpioAction::BUTTON_PRESS_R2, "R2"},
            {GpioAction::BUTTON_PRESS_S1, "S1"},
            {GpioAction::BUTTON_PRESS_S2, "S2"},
            {GpioAction::BUTTON_PRESS_A1, "A1"},
            {GpioAction::BUTTON_PRESS_A2, "A2"},
            {GpioAction::BUTTON_PRESS_L3, "L3"},
            {GpioAction::BUTTON_PRESS_R3, "R3"},
            {GpioAction::BUTTON_PRESS_FN, "FN"},
            {GpioAction::BUTTON_PRESS_DDI_UP, "DDI" CHAR_UP},
            {GpioAction::BUTTON_PRESS_DDI_DOWN, "DDI" CHAR_DOWN},
            {GpioAction::BUTTON_PRESS_DDI_LEFT, "DDI" CHAR_LEFT},
            {GpioAction::BUTTON_PRESS_DDI_RIGHT, "DDI" CHAR_RIGHT},
            {GpioAction::SUSTAIN_DP_MODE_DP, "DP"},
            {GpioAction::SUSTAIN_DP_MODE_LS, "LS"},
            {GpioAction::SUSTAIN_DP_MODE_RS, "RS"},
            {GpioAction::SUSTAIN_SOCD_MODE_UP_PRIO, "SOCD-UP"},
            {GpioAction::SUSTAIN_SOCD_MODE_NEUTRAL, "SOCD-N"},
            {GpioAction::SUSTAIN_SOCD_MODE_SECOND_WIN, "SOCD-2W"},
            {GpioAction::SUSTAIN_SOCD_MODE_FIRST_WIN, "SOCD-1W"},
            {GpioAction::SUSTAIN_SOCD_MODE_BYPASS, "SOCD-BP"},
            {GpioAction::BUTTON_PRESS_TURBO, "TRB"},
            {GpioAction::BUTTON_PRESS_MACRO, "M"},
            {GpioAction::BUTTON_PRESS_MACRO_1, "M1"},
            {GpioAction::BUTTON_PRESS_MACRO_2, "M2"},
            {GpioAction::BUTTON_PRESS_MACRO_3, "M3"},
            {GpioAction::BUTTON_PRESS_MACRO_4, "M4"},
            {GpioAction::BUTTON_PRESS_MACRO_5, "M5"},
            {GpioAction::BUTTON_PRESS_MACRO_6, "M6"},
            {GpioAction::BUTTON_PRESS_A3, "A3"},
            {GpioAction::BUTTON_PRESS_A4, "A4"},
            {GpioAction::BUTTON_PRESS_E1, "E1"},
            {GpioAction::BUTTON_PRESS_E2, "E2"},
            {GpioAction::BUTTON_PRESS_E3, "E3"},
            {GpioAction::BUTTON_PRESS_E4, "E4"},
            {GpioAction::BUTTON_PRESS_E5, "E5"},
            {GpioAction::BUTTON_PRESS_E6, "E6"},
            {GpioAction::BUTTON_PRESS_E7, "E7"},
            {GpioAction::BUTTON_PRESS_E8, "E8"},
            {GpioAction::BUTTON_PRESS_E9, "E9"},
            {GpioAction::BUTTON_PRESS_E10, "E10"},
            {GpioAction::BUTTON_PRESS_E11, "E11"},
            {GpioAction::BUTTON_PRESS_E12, "E12"},
        };
};

#endif