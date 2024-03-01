#ifndef _GPGFX_UI_SCREENS_H_
#define _GPGFX_UI_SCREENS_H_

enum DisplayMode {
    CONFIG_INSTRUCTION,
    BUTTONS,
    SPLASH,
    MAIN_MENU
};

#include "ui/screens/ButtonLayoutScreen.h"
#include "ui/screens/ConfigScreen.h"
#include "ui/screens/MainMenuScreen.h"
#include "ui/screens/SplashScreen.h"

#endif