#ifndef CONFIG_LEGACY_H
#define CONFIG_LEGACY_H

// TODO: Once migration is complete the content of this file can be moved into config_legacy.cpp after which this file
//       can be deleted

#include <stdint.h>
#include <stddef.h>

#include "mbedtls/rsa.h"
#include "NeoPico.hpp"
#include "Animation.hpp"

namespace ConfigLegacy
{
    enum InputMode
    {
        INPUT_MODE_XINPUT,
        INPUT_MODE_SWITCH,
        INPUT_MODE_HID,
        INPUT_MODE_KEYBOARD,
        INPUT_MODE_PS4,
        INPUT_MODE_CONFIG = 255,
    };

    enum DpadMode
    {
        DPAD_MODE_DIGITAL,
        DPAD_MODE_LEFT_ANALOG,
        DPAD_MODE_RIGHT_ANALOG,
    };

    enum SOCDMode
    {
        SOCD_MODE_UP_PRIORITY,
        SOCD_MODE_NEUTRAL,
        SOCD_MODE_SECOND_INPUT_PRIORITY,
        SOCD_MODE_FIRST_INPUT_PRIORITY,
        SOCD_MODE_BYPASS,
    };

    enum DpadDirection
    {
        DIRECTION_NONE,
        DIRECTION_UP,
        DIRECTION_DOWN,
        DIRECTION_LEFT,
        DIRECTION_RIGHT,
    };

    enum GamepadHotkey
    {
        HOTKEY_NONE,
        HOTKEY_DPAD_DIGITAL,
        HOTKEY_DPAD_LEFT_ANALOG,
        HOTKEY_DPAD_RIGHT_ANALOG,
        HOTKEY_HOME_BUTTON,
        HOTKEY_CAPTURE_BUTTON,
        HOTKEY_SOCD_UP_PRIORITY,
        HOTKEY_SOCD_NEUTRAL,
        HOTKEY_SOCD_LAST_INPUT,
        HOTKEY_INVERT_X_AXIS,
        HOTKEY_INVERT_Y_AXIS,
        HOTKEY_SOCD_FIRST_INPUT,
        HOTKEY_SOCD_BYPASS,
    };

    enum ButtonLayout
    {
        BUTTON_LAYOUT_STICK,
        BUTTON_LAYOUT_STICKLESS,
        BUTTON_LAYOUT_BUTTONS_ANGLED,
        BUTTON_LAYOUT_BUTTONS_BASIC,
        BUTTON_LAYOUT_KEYBOARD_ANGLED,
        BUTTON_LAYOUT_KEYBOARDA,
        BUTTON_LAYOUT_DANCEPADA,
        BUTTON_LAYOUT_TWINSTICKA,
        BUTTON_LAYOUT_BLANKA,
        BUTTON_LAYOUT_VLXA,
        BUTTON_LAYOUT_FIGHTBOARD_STICK,
        BUTTON_LAYOUT_FIGHTBOARD_MIRRORED,
        BUTTON_LAYOUT_CUSTOMA,
    };

    enum ButtonLayoutRight
    {
        BUTTON_LAYOUT_ARCADE,
        BUTTON_LAYOUT_STICKLESSB,
        BUTTON_LAYOUT_BUTTONS_ANGLEDB,
        BUTTON_LAYOUT_VEWLIX,
        BUTTON_LAYOUT_VEWLIX7,
        BUTTON_LAYOUT_CAPCOM,
        BUTTON_LAYOUT_CAPCOM6,
        BUTTON_LAYOUT_SEGA2P,
        BUTTON_LAYOUT_NOIR8,
        BUTTON_LAYOUT_KEYBOARDB,
        BUTTON_LAYOUT_DANCEPADB,
        BUTTON_LAYOUT_TWINSTICKB,
        BUTTON_LAYOUT_BLANKB,
        BUTTON_LAYOUT_VLXB,
        BUTTON_LAYOUT_FIGHTBOARD,
        BUTTON_LAYOUT_FIGHTBOARD_STICK_MIRRORED,
        BUTTON_LAYOUT_CUSTOMB,
    };

    enum SplashMode
    {
        STATICSPLASH,
        CLOSEIN,
        CLOSEINCUSTOM,
        NOSPLASH,
    };

    enum SplashChoice
    {
        MAIN,
        X,
        Y,
        Z,
        CUSTOM,
        LEGACY
    };
    
    struct ButtonLayoutParams
    {
        union {
            ButtonLayout layout;
            ButtonLayoutRight layoutRight;
        };
        int startX;
        int startY;
        int buttonRadius;
        int buttonPadding;
    };

    struct ButtonLayoutCustomOptions
    {
        ButtonLayoutParams params;
        ButtonLayoutParams paramsRight;
    }; // 76 bytes


    struct BoardOptions
    {
        bool hasBoardOptions;
        uint8_t pinDpadUp;
        uint8_t pinDpadDown;
        uint8_t pinDpadLeft;
        uint8_t pinDpadRight;
        uint8_t pinButtonB1;
        uint8_t pinButtonB2;
        uint8_t pinButtonB3;
        uint8_t pinButtonB4;
        uint8_t pinButtonL1;
        uint8_t pinButtonR1;
        uint8_t pinButtonL2;
        uint8_t pinButtonR2;
        uint8_t pinButtonS1;
        uint8_t pinButtonS2;
        uint8_t pinButtonL3;
        uint8_t pinButtonR3;
        uint8_t pinButtonA1;
        uint8_t pinButtonA2;
        ButtonLayout buttonLayout;
        ButtonLayoutRight buttonLayoutRight;
        SplashMode splashMode;
        SplashChoice splashChoice;
        int splashDuration; // -1 = Always on
        uint8_t i2cSDAPin;
        uint8_t i2cSCLPin;
        int i2cBlock;
        uint32_t i2cSpeed;
        bool hasI2CDisplay;
        int displayI2CAddress;
        uint8_t displaySize;
        uint8_t displayFlip;
        bool displayInvert;
        int displaySaverTimeout;
        ButtonLayoutCustomOptions buttonLayoutCustomOptions;
        char boardVersion[32]; // 32-char limit to board name
        uint32_t checksum;
    };

    struct LEDOptions
    {
        bool useUserDefinedLEDs;
        int dataPin;
        LEDFormat ledFormat;
        ButtonLayout ledLayout;
        uint8_t ledsPerButton;
        uint8_t brightnessMaximum;
        uint8_t brightnessSteps;
        int indexUp;
        int indexDown;
        int indexLeft;
        int indexRight;
        int indexB1;
        int indexB2;
        int indexB3;
        int indexB4;
        int indexL1;
        int indexR1;
        int indexL2;
        int indexR2;
        int indexS1;
        int indexS2;
        int indexL3;
        int indexR3;
        int indexA1;
        int indexA2;
        int pledType;
        int pledPin1;
        int pledPin2;
        int pledPin3;
        int pledPin4;
        RGB pledColor;
        uint32_t checksum;
    };

    struct GamepadHotkeyEntry
    {
        uint8_t dpadMask;
        GamepadHotkey action;
    };

    struct GamepadOptions
    {
        InputMode inputMode {InputMode::INPUT_MODE_XINPUT}; 
        DpadMode dpadMode {DpadMode::DPAD_MODE_DIGITAL};
        SOCDMode socdMode {SOCDMode::SOCD_MODE_NEUTRAL};
        bool invertXAxis;
        bool invertYAxis;

        uint8_t keyDpadUp;
        uint8_t keyDpadDown;
        uint8_t keyDpadLeft;
        uint8_t keyDpadRight;
        uint8_t keyButtonB1;
        uint8_t keyButtonB2;
        uint8_t keyButtonB3;
        uint8_t keyButtonB4;
        uint8_t keyButtonL1;
        uint8_t keyButtonR1;
        uint8_t keyButtonL2;
        uint8_t keyButtonR2;
        uint8_t keyButtonS1;
        uint8_t keyButtonS2;
        uint8_t keyButtonL3;
        uint8_t keyButtonR3;
        uint8_t keyButtonA1;
        uint8_t keyButtonA2;

        GamepadHotkeyEntry hotkeyF1Up;
        GamepadHotkeyEntry hotkeyF1Down;
        GamepadHotkeyEntry hotkeyF1Left;
        GamepadHotkeyEntry hotkeyF1Right;
        GamepadHotkeyEntry hotkeyF2Up;
        GamepadHotkeyEntry hotkeyF2Down;
        GamepadHotkeyEntry hotkeyF2Left;
        GamepadHotkeyEntry hotkeyF2Right;

        uint32_t checksum;
    };
}

#endif
