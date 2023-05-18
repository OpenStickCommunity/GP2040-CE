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

    enum PLEDType
    {
        PLED_TYPE_NONE = -1,
        PLED_TYPE_PWM = 0,
        PLED_TYPE_RGB = 1,
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
}

#endif
