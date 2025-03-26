#ifndef _MAINMENUSCREEN_H_
#define _MAINMENUSCREEN_H_

#include "GPGFX_UI_widgets.h"
#include "GPGFX_UI_types.h"
#include "enums.pb.h"
#include "animationstation.h"
#include "eventmanager.h"

#define INPUT_MODE_XINPUT_NAME "XInput"
#define INPUT_MODE_SWITCH_NAME "Nintendo Switch"
#define INPUT_MODE_PS3_NAME "PS3"
#define INPUT_MODE_KEYBOARD_NAME "Keyboard"
#define INPUT_MODE_PS4_NAME "PS4"
#define INPUT_MODE_XBONE_NAME "Xbox One"
#define INPUT_MODE_MDMINI_NAME "Sega Genesis Mini"
#define INPUT_MODE_NEOGEO_NAME "NEOGEO mini"
#define INPUT_MODE_PCEMINI_NAME "PC Engine Mini"
#define INPUT_MODE_EGRET_NAME "EGRET II mini"
#define INPUT_MODE_ASTRO_NAME "ASTROCITY Mini"
#define INPUT_MODE_PSCLASSIC_NAME "Playstation Classic"
#define INPUT_MODE_XBOXORIGINAL_NAME "Original Xbox"
#define INPUT_MODE_PS5_NAME "PS5"
#define INPUT_MODE_GENERIC_NAME "Generic HID"
#define INPUT_MODE_CONFIG_NAME "Web Config"

#define SOCD_MODE_UP_PRIORITY_NAME "Up Priority"
#define SOCD_MODE_NEUTRAL_NAME "Neutral"
#define SOCD_MODE_SECOND_INPUT_PRIORITY_NAME "Last Win"
#define SOCD_MODE_FIRST_INPUT_PRIORITY_NAME "First Win"
#define SOCD_MODE_BYPASS_NAME "Off"

#define DPAD_MODE_DIGITAL_NAME "D-Pad"
#define DPAD_MODE_LEFT_ANALOG_NAME "Left Analog"
#define DPAD_MODE_RIGHT_ANALOG_NAME "Right Analog"

#define MAIN_MENU_NAME "GP2040-CE Mini Menu"

class MainMenuScreen : public GPScreen {
    public:
        MainMenuScreen() {}
        MainMenuScreen(GPGFX* renderer) { setRenderer(renderer); }
        virtual ~MainMenuScreen() {}
        void setMenu(std::vector<MenuEntry>* menu);
        virtual int8_t update();
        virtual void init();
        virtual void shutdown();

        void testMenu() {}
        void saveAndExit();
        void exitOnly();
        int32_t modeValue();

        void selectInputMode();
        int32_t currentInputMode();

        void selectDPadMode();
        int32_t currentDpadMode();

        void selectSOCDMode();
        int32_t currentSOCDMode();

        void selectProfile();
        int32_t currentProfile();

        void selectFocusMode();
        int32_t currentFocusMode();

        void selectTurboMode();
        int32_t currentTurboMode();

        void updateMenuNavigation(GpioAction action);
        void updateEventMenuNavigation(GpioAction action);
        void chooseAndReturn();

        void setMenuHome();
    protected:
        virtual void drawScreen();
    private:
        bool isPressed = false;
        uint32_t checkDebounce;
        std::vector<MenuEntry>* currentMenu;
        std::vector<MenuEntry>* previousMenu;
        uint16_t prevButtonState = 0;
        uint8_t prevDpadState = 0;
        Mask_t prevValues;
        GPMenu* gpMenu = nullptr;
        const uint8_t menuLineSize = 4;

        GpioAction eventAction;

        bool screenIsPrompting = false;
        bool promptChoice = false;
        bool isMenuReady = false;

        int8_t exitToScreenBeforePrompt = -1;
        int8_t exitToScreen = -1;

        GamepadButtonMapping *mapMenuUp;
        GamepadButtonMapping *mapMenuDown;
        GamepadButtonMapping *mapMenuLeft;
        GamepadButtonMapping *mapMenuRight;
        GamepadButtonMapping *mapMenuSelect;
        GamepadButtonMapping *mapMenuBack;
        GamepadButtonMapping *mapMenuToggle;

        void saveOptions();
        void resetOptions();
        bool changeRequiresReboot = false;
        bool changeRequiresSave = false;

        #define INPUT_MODE_ENTRIES(name, value) {name##_NAME, NULL, nullptr, std::bind(&MainMenuScreen::currentInputMode, this), std::bind(&MainMenuScreen::selectInputMode, this), value},
        #define DPAD_MODE_ENTRIES(name, value)  {name##_NAME, NULL, nullptr, std::bind(&MainMenuScreen::currentDpadMode,  this), std::bind(&MainMenuScreen::selectDPadMode,  this), value},
        #define SOCD_MODE_ENTRIES(name, value)  {name##_NAME, NULL, nullptr, std::bind(&MainMenuScreen::currentSOCDMode,  this), std::bind(&MainMenuScreen::selectSOCDMode,  this), value},

        std::vector<MenuEntry> inputModeMenu = {
            InputMode_VALUELIST(INPUT_MODE_ENTRIES)
        };
        InputMode prevInputMode;
        InputMode updateInputMode;

        std::vector<MenuEntry> dpadModeMenu = {
            DpadMode_VALUELIST(DPAD_MODE_ENTRIES)
        };
        DpadMode prevDpadMode;
        DpadMode updateDpadMode;

        std::vector<MenuEntry> socdModeMenu = {
            SOCDMode_VALUELIST(SOCD_MODE_ENTRIES)
        };
        SOCDMode prevSocdMode;
        SOCDMode updateSocdMode;

        std::vector<MenuEntry> profilesMenu = {};
        uint8_t prevProfile;
        uint8_t updateProfile;

        std::vector<MenuEntry> focusModeMenu = {
            {"Off",        NULL, nullptr,        std::bind(&MainMenuScreen::currentFocusMode, this), std::bind(&MainMenuScreen::selectFocusMode, this), 0},
            {"On",         NULL, nullptr,        std::bind(&MainMenuScreen::currentFocusMode, this), std::bind(&MainMenuScreen::selectFocusMode, this), 1},
        };
        bool prevFocus;
        bool updateFocus;

        std::vector<MenuEntry> turboModeMenu = {
            {"Off",        NULL, nullptr,        std::bind(&MainMenuScreen::currentTurboMode, this), std::bind(&MainMenuScreen::selectTurboMode, this), 0},
            {"On",         NULL, nullptr,        std::bind(&MainMenuScreen::currentTurboMode, this), std::bind(&MainMenuScreen::selectTurboMode, this), 1},
        };
        bool prevTurbo;
        bool updateTurbo;

        std::vector<MenuEntry> mainMenu = {
            {"Input Mode", NULL, &inputModeMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)},
            {"D-Pad Mode", NULL, &dpadModeMenu,  std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)},
            {"SOCD Mode",  NULL, &socdModeMenu,  std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)},
            {"Profile",    NULL, &profilesMenu,  std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)},
            {"Focus Mode", NULL, &focusModeMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)},
            {"Turbo",      NULL, &turboModeMenu, std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)},
            {"Exit",       NULL, &saveMenu,      std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::testMenu, this)},
        };

        std::vector<MenuEntry> saveMenu = {
            {"Save & Exit",NULL, nullptr,        std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::saveAndExit, this), 1},
            {"Exit",       NULL, nullptr,        std::bind(&MainMenuScreen::modeValue, this), std::bind(&MainMenuScreen::exitOnly, this), 0},
        };
};

#endif