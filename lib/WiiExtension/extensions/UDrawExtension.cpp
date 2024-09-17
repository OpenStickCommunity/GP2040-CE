#include "ExtensionBase.h"
#include "UDrawExtension.h"

#include "WiiExtension.h"

void UDrawExtension::process(uint8_t *inputData) {
    uint16_t xPosition = (uint16_t)((inputData[2] & 0x0F) << 8) | (uint16_t)((inputData[0] >> 4) << 4) | (uint16_t)(inputData[0] & 0x0F);
    uint16_t yPosition = (uint16_t)((inputData[2] >> 4) << 8) | (uint16_t)((inputData[1] >> 4) << 4) | (uint16_t)(inputData[1] & 0x0F);
    uint16_t pressure = ((inputData[3] & 0xFF));
    bool button0 = !((inputData[5] & 0x01));
    bool button1 = !((inputData[5] & 0x02) >> 1);
    bool button2 = ((inputData[5] & 0x04) >> 1);

    motionState[WiiMotions::WII_TOUCH_X] = (xPosition != 0xFFF ? map(xPosition,UDRAW_MIN_X, UDRAW_MAX_X, UDRAW_MIN_X, UDRAW_MAX_X) : 0);
    motionState[WiiMotions::WII_TOUCH_Y] = (yPosition != 0xFFF ? map(yPosition,UDRAW_MIN_Y, UDRAW_MAX_Y, UDRAW_MAX_Y, UDRAW_MIN_Y) : 0);
    motionState[WiiMotions::WII_TOUCH_Z] = pressure;
    motionState[WiiMotions::WII_TOUCH_PRESSED] = !(xPosition == 0xFFF && yPosition == 0xFFF);

    buttons[WiiButtons::WII_BUTTON_A]    = button2;
    buttons[WiiButtons::WII_BUTTON_L]    = button1;
    buttons[WiiButtons::WII_BUTTON_R]    = button0;

#if WII_EXTENSION_DEBUG==true
    printf("\x1B[15;0H" BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(inputData[0]));
    printf("\x1B[16;0H" BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(inputData[1]));
    printf("\x1B[17;0H" BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(inputData[2]));
    printf("\x1B[18;0H" BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(inputData[3]));
    printf("\x1B[19;0H" BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(inputData[4]));
    printf("\x1B[20;0H" BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(inputData[5]));
    printf("\x1B[21;0HX=%5d Y=%5d B0=%1d B1=%1d B2=%1d\n", motionState[WiiMotions::WII_TOUCH_X], motionState[WiiMotions::WII_TOUCH_Y], buttons[WiiButtons::WII_BUTTON_A], buttons[WiiButtons::WII_BUTTON_L], buttons[WiiButtons::WII_BUTTON_R]);
#endif
}