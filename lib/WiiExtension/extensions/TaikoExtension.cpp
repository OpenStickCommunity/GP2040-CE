#include "ExtensionBase.h"
#include "TaikoExtension.h"

#include "WiiExtension.h"

void TaikoExtension::process(uint8_t *inputData) {
    if (getDataType() == WII_DATA_TYPE_1) {
        buttons[TaikoButtons::TATA_DON_LEFT]  = !((inputData[5] & 0x40) >> 6);
        buttons[TaikoButtons::TATA_KAT_LEFT]  = !((inputData[5] & 0x20) >> 5);
        buttons[TaikoButtons::TATA_DON_RIGHT] = !((inputData[5] & 0x10) >> 4);
        buttons[TaikoButtons::TATA_KAT_RIGHT] = !((inputData[5] & 0x08) >> 3);
    } else if (getDataType() == WII_DATA_TYPE_2) {
        buttons[TaikoButtons::TATA_DON_LEFT]  = !((inputData[8] & 0x40) >> 6);
        buttons[TaikoButtons::TATA_KAT_LEFT]  = !((inputData[8] & 0x20) >> 5);
        buttons[TaikoButtons::TATA_DON_RIGHT] = !((inputData[8] & 0x10) >> 4);
        buttons[TaikoButtons::TATA_KAT_RIGHT] = !((inputData[8] & 0x08) >> 3);
    } else if (getDataType() == WII_DATA_TYPE_3) {
        buttons[TaikoButtons::TATA_DON_LEFT]  = !((inputData[7] & 0x40) >> 6);
        buttons[TaikoButtons::TATA_KAT_LEFT]  = !((inputData[7] & 0x20) >> 5);
        buttons[TaikoButtons::TATA_DON_RIGHT] = !((inputData[7] & 0x10) >> 4);
        buttons[TaikoButtons::TATA_KAT_RIGHT] = !((inputData[7] & 0x08) >> 3);
    }

#if WII_EXTENSION_DEBUG==true
//    //if (_lastRead[0] != regRead[0]) printf("Byte0    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[0]));
//    //if (_lastRead[1] != regRead[1]) printf("Byte1    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[1]));
//    //if (_lastRead[2] != regRead[2]) printf("Byte2    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[2]));
//    //if (_lastRead[3] != regRead[3]) printf("Byte3    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[3]));
//    //if (_lastRead[4] != regRead[4]) printf("Byte4    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[4]));
//    //if (_lastRead[5] != regRead[5]) printf("Byte5    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[5]));
//    //
//    if (_lastRead[7] != regRead[7]) {
//        printf("DL=%1d RL=%1d DR=%1d RR=%1d\n", drumLeft, rimLeft, drumRight, rimRight);
//    }
    printf("DL=%1d RL=%1d DR=%1d RR=%1d\n", buttons[TaikoButtons::TATA_DON_LEFT], buttons[TaikoButtons::TATA_KAT_LEFT], buttons[TaikoButtons::TATA_DON_RIGHT], buttons[TaikoButtons::TATA_KAT_RIGHT]);
#endif
}