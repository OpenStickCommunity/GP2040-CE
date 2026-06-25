#include "ExtensionBase.h"
#include "TrainExtension.h"

#include "WiiExtension.h"

void TrainExtension::process(uint8_t *inputData) {
#if WII_EXTENSION_DEBUG==true
//    //if (_lastRead[0] != regRead[0]) printf("Byte0    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[0]));
//    //if (_lastRead[1] != regRead[1]) printf("Byte1    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[1]));
//    //if (_lastRead[2] != regRead[2]) printf("Byte2    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[2]));
//    //if (_lastRead[3] != regRead[3]) printf("Byte3    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[3]));
//    //if (_lastRead[4] != regRead[4]) printf("Byte4    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[4]));
//    //if (_lastRead[5] != regRead[5]) printf("Byte5    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[5]));
#endif
}

uint8_t TrainExtension::prepareOutput() {
    controllerData[0x00] = 0xFF;
    controllerData[0x01] = 0xFF;
    controllerData[0x02] = 0xFF;
    controllerData[0x03] = 0xFF;
    controllerData[0x04] = 0xFF;
    controllerData[0x05] = 0xFF;
    controllerData[0x06] = 0xFF;
    controllerData[0x07] = 0xFF;
    return 8;
}

