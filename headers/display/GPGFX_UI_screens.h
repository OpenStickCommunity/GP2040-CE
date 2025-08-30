#ifndef _GPGFX_UI_SCREENS_H_
#define _GPGFX_UI_SCREENS_H_

enum DisplayMode {
    CONFIG_INSTRUCTION,
    BUTTONS,
    SPLASH,
    PIN_VIEWER,
    DISPLAY_SAVER,
    STATS,
    MAIN_MENU,
    RESTART
};

#include "ui/screens/ButtonLayoutScreen.h"
#include "ui/screens/ConfigScreen.h"
#include "ui/screens/DisplaySaverScreen.h"
#include "ui/screens/MainMenuScreen.h"
#include "ui/screens/PinViewerScreen.h"
#include "ui/screens/RestartScreen.h"
#include "ui/screens/SplashScreen.h"
#include "ui/screens/StatsScreen.h"

#endif