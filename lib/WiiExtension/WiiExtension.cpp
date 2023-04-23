#include "WiiExtension.h"

#include <cstring>
#include <cstdio>

WiiExtension::WiiExtension(int bWire, int sda, int scl, i2c_inst_t *picoI2C, int32_t speed, uint8_t addr) {
    bbi2c.iSDA = sda;
    bbi2c.iSCL = scl;
    bbi2c.picoI2C = picoI2C;
    bbi2c.bWire = bWire;
    iSpeed = speed;
    address = addr;
    config = 0x00;
    singleShot = true;
}

void WiiExtension::begin() {
    I2CInit(&bbi2c, iSpeed);
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::begin\n");
#endif
    isReady = false;
    reset();
}

void WiiExtension::start(){
    uint8_t idRead[16];
    uint8_t regWrite[16];
    int8_t result;

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start\n");
    printf("WiiExtension::start isReady? %1d\n", isReady);
#endif

    if (!isReady) return;

    regWrite[0] = 0xFA;
    //result = I2CWrite(&bbi2c, address, regWrite, 1);
    result = i2c_write_blocking(bbi2c.picoI2C, address, &regWrite[0], 1, false);
    sleep_us(WII_EXTENSION_DELAY);

    extensionType = WII_EXTENSION_NONE;

    // continue if the write was successful
    if (result > -1) {
        //I2CRead(&bbi2c, address, idRead, 6);
        i2c_read_blocking(bbi2c.picoI2C, address, idRead, 6, false);
        sleep_us(WII_EXTENSION_DELAY);

        if (idRead[5] == 0x00) {
            extensionType = WII_EXTENSION_NUNCHUCK;
        } else if (idRead[5] == 0x01) {
            extensionType = WII_EXTENSION_CLASSIC;
            if (idRead[0] == 0x01) {
                extensionType = WII_EXTENSION_CLASSIC_PRO;
            }
        } else if (idRead[5] == 0x03) {
            extensionType = WII_EXTENSION_GUITAR;
            if (idRead[0] == 0x01) {
                extensionType = WII_EXTENSION_DRUMS;
            }
        }

        // in certain situations (eg. Nunchuck), setting the data type in reset() does not affect what this value will be
        // Classic/Classic Pro are the only devices that report multiple data modes
        dataType = idRead[4];

#if WII_EXTENSION_DEBUG==true
        printf("Extension ID: %02x%02x %02x%02x %02x%02x\n", idRead[0], idRead[1], idRead[2], idRead[3], idRead[4], idRead[5]);
        printf("Data Format: %02x\n",idRead[4]);
#endif

        if (extensionType != WII_EXTENSION_NONE) {
#if WII_EXTENSION_DEBUG==true
            //printf("Extension Type: %d\n", extensionType);
#endif

            if (extensionType == WII_EXTENSION_NUNCHUCK) {
                // read calibration
                regWrite[0] = 0x20;
//                i2c_write_blocking(bbi2c.picoI2C, address, &regWrite[0], 1, false);
//                sleep_us(WII_EXTENSION_DELAY);
//
//                //I2CRead(&bbi2c, address, idRead, 16);
//                i2c_read_blocking(bbi2c.picoI2C, address, idRead, 6, false);
//                sleep_us(WII_EXTENSION_DELAY);
//
//                _maxX = idRead[8];
//                _minX = idRead[9];
//                _cenX = idRead[10];
//
//                _maxY = idRead[11];
//                _minY = idRead[12];
//                _cenY = idRead[13];
//
//                _accelX0G = ((idRead[0] << 2) | ((idRead[3] >> 2) & 0x03));
//                _accelY0G = ((idRead[1] << 2) | ((idRead[3] >> 4) & 0x03));
//                _accelZ0G = ((idRead[2] << 2) | ((idRead[3] >> 6) & 0x03));
//
//                _accelX1G = ((idRead[4] << 2) | ((idRead[7] >> 2) & 0x03));
//                _accelY1G = ((idRead[5] << 2) | ((idRead[7] >> 4) & 0x03));
//                _accelZ1G = ((idRead[6] << 2) | ((idRead[7] >> 6) & 0x03));

#if WII_EXTENSION_DEBUG==true
                //printf("Calibration:\n");
                //printf("X0G: %d\n", _accelX0G);
                //printf("Y0G: %d\n", _accelY0G);
                //printf("Z0G: %d\n", _accelZ0G);
                //printf("X1G: %d\n", _accelX1G);
                //printf("Y1G: %d\n", _accelY1G);
                //printf("YZG: %d\n", _accelZ1G);
                //printf("X Min: %d\n", _minX);
                //printf("X Max: %d\n", _maxX);
                //printf("X Center: %d\n", _cenX);
                //printf("Y Min: %d\n", _minY);
                //printf("Y Max: %d\n", _maxY);
                //printf("Y Center: %d\n", _cenY);
#endif
            }

            // reset to default input values in the event of a removal/hotswap
//            joy1X           = 0;
//            joy1Y           = 0;
//            joy2X           = 0;
//            joy2Y           = 0;
//            accelX          = 0;
//            accelY          = 0;
//            accelZ          = 0;
//
//            buttonZ         = 0;
//            buttonC         = 0;
//            buttonZR        = 0;
//            buttonZL        = 0;
//            buttonA         = 0;
//            buttonB         = 0;
//            buttonX         = 0;
//            buttonY         = 0;
//            buttonPlus      = 0;
//            buttonHome      = 0;
//            buttonMinus     = 0;
//            buttonLT        = 0;
//            buttonRT        = 0;
//
//            directionUp     = 0;
//            directionDown   = 0;
//            directionLeft   = 0;
//            directionRight  = 0;
//
//            triggerLeft     = 0;
//            triggerRight    = 0;
//
//            fretGreen       = 0;
//            fretRed         = 0;
//            fretYellow      = 0;
//            fretBlue        = 0;
//            fretOrange      = 0;
//
//            whammyBar       = 0;
        } else {
#if WII_EXTENSION_DEBUG==true
            printf("Unknown Extension: %02x%02x %02x%02x %02x%02x\n", idRead[0], idRead[1], idRead[2], idRead[3], idRead[4], idRead[5]);
#endif
        }

        regWrite[0] = 0x00;
        //I2CWrite(&bbi2c, address, regWrite, 2);
        result = i2c_write_blocking(bbi2c.picoI2C, address, regWrite, 1, false);
        sleep_us(WII_EXTENSION_DELAY);
    }
}

void WiiExtension::reset(){
    uint8_t regWrite[16];
    int8_t result;
    bool canContinue = true;

//    if (canContinue) {
//        result = I2CTest(&bbi2c, address);
//        sleep_us(WII_EXTENSION_DELAY);
//        canContinue = (result == 1);
//    }

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::reset\n");
#endif

#if WII_ENCRYPTION==false
    if (canContinue) {
        regWrite[0] = 0xF0;
        regWrite[1] = 0x55;
        //result = I2CWrite(&bbi2c, address, regWrite, 2);
        result = i2c_write_blocking(bbi2c.picoI2C, address, regWrite, 2, false);
        sleep_us(WII_EXTENSION_DELAY);
        canContinue = (result > -1);
    }

    if (canContinue) {
        regWrite[0] = 0xFB;
        regWrite[1] = 0x00;
        //result = I2CWrite(&bbi2c, address, regWrite, 2);
        result = i2c_write_blocking(bbi2c.picoI2C, address, regWrite, 2, false);
        sleep_us(WII_EXTENSION_DELAY);
        canContinue = (result > -1);
    }

    if (canContinue) {
        // set data format
        regWrite[0] = 0xFE;
        regWrite[1] = 0x03;
        //result = I2CWrite(&bbi2c, address, regWrite, 2);
        result = i2c_write_blocking(bbi2c.picoI2C, address, regWrite, 2, false);
        sleep_us(WII_EXTENSION_DELAY);
        canContinue = (result > -1);
    }
#elif WII_ENCRYPTION==true
    if (canContinue) {
        regWrite[0] = 0x40;
        regWrite[1] = 0x00;
        //result = I2CWrite(&bbi2c, address, regWrite, 2);
        result = i2c_write_blocking(bbi2c.picoI2C, address, regWrite, 2, false);
        sleep_us(WII_EXTENSION_DELAY);
        canContinue = (result > -1);
    }
#endif

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::reset canContinue? %1d\n", canContinue);
#endif

    if (canContinue) {
#if WII_EXTENSION_DEBUG==true
        //printf("Reset Sent\n");
#endif
        isReady = true;
    } else {
#if WII_EXTENSION_DEBUG==true
        //printf("Device not found\n");
#endif
    }
}

void WiiExtension::poll() {
    uint8_t reg = _u(0x08);
    uint8_t regWrite[16];
    uint8_t regRead[16];
    int8_t result;

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::poll\n");
    printf("WiiExtension::poll isReady? %1d\n", isReady);
#endif

    buttonZ = (extensionType == WII_EXTENSION_NUNCHUCK);

    if (!isReady) return;

    if (extensionType != WII_EXTENSION_NONE) {
        switch (extensionType) {
            case WII_EXTENSION_NUNCHUCK:
                result = i2c_read_blocking(bbi2c.picoI2C, address, regRead, 6, false);
                //result = I2CRead(&bbi2c, address, regRead, 6);
                //result = i2c_read_blocking(bbi2c.picoI2C, address, regRead, 6, true);
                sleep_us(WII_EXTENSION_DELAY);
                if (result < 0) break;

                joy1X   = regRead[0]-_cenX;
                joy1Y   = regRead[1]-_cenY;
                accelX = (((regRead[2] << 2) | ((regRead[5] >> 2) & 0x03)));
                accelY = (((regRead[3] << 2) | ((regRead[5] >> 4) & 0x03)));
                accelZ = (((regRead[4] << 2) | ((regRead[5] >> 6) & 0x03)));
                buttonZ  = (!(regRead[5] & 0x01));
                buttonC  = (!(regRead[5] & 0x02));

#if WII_EXTENSION_DEBUG==true
                printf("Joy X=%4d Y=%4d   Acc X=%4d Y=%4d Z=%4d   Btn Z=%1d C=%1d\n", joy1X, joy1Y, accelX, accelY, accelZ, buttonZ, buttonC);
#endif

                break;
            case WII_EXTENSION_CLASSIC:
            case WII_EXTENSION_CLASSIC_PRO:
                // write data format to return
                // see wiki for data types
                if (dataType == 0x01) {
                    //result = I2CRead(&bbi2c, address, regRead, 6);
                    //result = i2c_read_blocking(bbi2c.picoI2C, address, regRead, 6, true);
                    result = i2c_read_blocking(bbi2c.picoI2C, address, regRead, 6, false);
                    sleep_us(WII_EXTENSION_DELAY);
                    if (result < 0) break;

                    joy1X =          (regRead[0] & 0x3F);
                    joy1Y =          (regRead[1] & 0x3F);

                    joy2X =          ((regRead[0] & 0xC0) >> 3) | ((regRead[1] & 0xC0) >> 5) | ((regRead[2] & 0x80) >> 7);
                    joy2Y =          (regRead[2] & 0x1F);

                    triggerLeft =    (((regRead[2] & 0x60) >> 2) | ((regRead[3] & 0xE0) >> 5));
                    triggerRight =   ((regRead[3] & 0x1F) >> 0);

                    directionRight = !((regRead[4] & 0x80) >> 7);
                    directionDown =  !((regRead[4] & 0x40) >> 6);
                    buttonLT =       !((regRead[4] & 0x20) >> 5);
                    buttonMinus =    !((regRead[4] & 0x10) >> 4);
                    buttonHome =     !((regRead[4] & 0x08) >> 3);
                    buttonPlus =     !((regRead[4] & 0x04) >> 2);
                    buttonRT =       !((regRead[4] & 0x02) >> 1);

                    buttonZL =       !((regRead[5] & 0x80) >> 7);
                    buttonB =        !((regRead[5] & 0x40) >> 6);
                    buttonY =        !((regRead[5] & 0x20) >> 5);
                    buttonA =        !((regRead[5] & 0x10) >> 4);
                    buttonX =        !((regRead[5] & 0x08) >> 3);
                    buttonZR =       !((regRead[5] & 0x04) >> 2);
                    directionLeft =  !((regRead[5] & 0x02) >> 1);
                    directionUp =    !((regRead[5] & 0x01) >> 0);
                } else if (dataType == 0x02) {
                    //result = I2CRead(&bbi2c, address, regRead, 9);
                    result = i2c_read_blocking(bbi2c.picoI2C, address, regRead, 9, false);
                    sleep_us(WII_EXTENSION_DELAY);
                    if (result < 0) break;

                    joy1X =          ((regRead[0] << 2) | ((regRead[4] & 0x03) >> 0));
                    joy1Y =          ((regRead[2] << 2) | ((regRead[4] & 0x30) >> 4));

                    joy2X =          ((regRead[1] << 2) | ((regRead[4] & 0x0C) >> 2));
                    joy2Y =          ((regRead[3] << 2) | ((regRead[4] & 0xC0) >> 6));

                    triggerLeft =    regRead[5];
                    triggerRight =   regRead[6];

                    directionRight = !((regRead[7] & 0x80) >> 7);
                    directionDown =  !((regRead[7] & 0x40) >> 6);
                    buttonLT =       !((regRead[7] & 0x20) >> 5);
                    buttonMinus =    !((regRead[7] & 0x10) >> 4);
                    buttonHome =     !((regRead[7] & 0x08) >> 3);
                    buttonPlus =     !((regRead[7] & 0x04) >> 2);
                    buttonRT =       !((regRead[7] & 0x02) >> 1);

                    buttonZL =       !((regRead[8] & 0x80) >> 7);
                    buttonB =        !((regRead[8] & 0x40) >> 6);
                    buttonY =        !((regRead[8] & 0x20) >> 5);
                    buttonA =        !((regRead[8] & 0x10) >> 4);
                    buttonX =        !((regRead[8] & 0x08) >> 3);
                    buttonZR =       !((regRead[8] & 0x04) >> 2);
                    directionLeft =  !((regRead[8] & 0x02) >> 1);
                    directionUp =    !((regRead[8] & 0x01) >> 0);
                } else if (dataType == 0x03) {
                    //result = I2CRead(&bbi2c, address, regRead, 8);
                    result = i2c_read_blocking(bbi2c.picoI2C, address, regRead, 8, false);
                    sleep_us(WII_EXTENSION_DELAY);
                    if (result < 0) break;

                    joy1X =          regRead[0];
                    joy1Y =          regRead[2];

                    joy2X =          regRead[1];
                    joy2Y =          regRead[3];

                    triggerLeft =    regRead[4];
                    triggerRight =   regRead[5];

                    directionRight = !((regRead[6] & 0x80) >> 7);
                    directionDown =  !((regRead[6] & 0x40) >> 6);
                    buttonLT =       !((regRead[6] & 0x20) >> 5);
                    buttonMinus =    !((regRead[6] & 0x10) >> 4);
                    buttonHome =     !((regRead[6] & 0x08) >> 3);
                    buttonPlus =     !((regRead[6] & 0x04) >> 2);
                    buttonRT =       !((regRead[6] & 0x02) >> 1);

                    buttonZL =       !((regRead[7] & 0x80) >> 7);
                    buttonB =        !((regRead[7] & 0x40) >> 6);
                    buttonY =        !((regRead[7] & 0x20) >> 5);
                    buttonA =        !((regRead[7] & 0x10) >> 4);
                    buttonX =        !((regRead[7] & 0x08) >> 3);
                    buttonZR =       !((regRead[7] & 0x04) >> 2);
                    directionLeft =  !((regRead[7] & 0x02) >> 1);
                    directionUp =    !((regRead[7] & 0x01) >> 0);
                } else {
                    // unknown
                }

#if WII_EXTENSION_DEBUG==true
                //printf("Joy1 X=%4d Y=%4d  Joy2 X=%4d Y=%4d  U=%1d D=%1d L=%1d R=%1d TL=%4d TR=%4d\n", joy1X, joy1Y, joy2X, joy2Y, directionUp, directionDown, directionLeft, directionRight, triggerLeft, triggerRight);
                //printf("A=%1d B=%1d X=%1d Y=%1d ZL=%1d ZR=%1d LT=%1d RT=%1d -=%1d H=%1d +=%1d\n", buttonA, buttonB, buttonX, buttonY, buttonZL, buttonZR, buttonLT, buttonRT, buttonMinus, buttonHome, buttonPlus);
#endif

                break;
            case WII_EXTENSION_GUITAR:
                //result = I2CRead(&bbi2c, address, regRead, 6);
                result = i2c_read_blocking(bbi2c.picoI2C, address, regRead, 6, false);
                sleep_us(WII_EXTENSION_DELAY);
                if (result < 0) break;

                joy1X =          (regRead[0] & 0x3F);
                joy1Y =          (regRead[1] & 0x3F);

                whammyBar =      (regRead[3] & 0x1F);

                directionDown =  !((regRead[4] & 0x40) >> 6);
                buttonMinus =    !((regRead[4] & 0x10) >> 4);
                buttonPlus =     !((regRead[4] & 0x04) >> 2);

                fretOrange =     !((regRead[5] & 0x80) >> 7);
                fretRed =        !((regRead[5] & 0x40) >> 6);
                fretBlue =       !((regRead[5] & 0x20) >> 5);
                fretGreen =      !((regRead[5] & 0x10) >> 4);
                fretYellow =     !((regRead[5] & 0x08) >> 3);
                directionUp =    !((regRead[5] & 0x01) >> 0);

#if WII_EXTENSION_DEBUG==true
                //printf("Joy1 X=%4d Y=%4d  Whammy=%4d  U=%1d D=%1d -=%1d +=%1d\n", joy1X, joy1Y, whammyBar, directionUp, directionDown, buttonMinus, buttonPlus);
                //printf("O=%1d B=%1d Y=%1d R=%1d G=%1d\n", fretOrange, fretBlue, fretYellow, fretRed, fretGreen);
#endif
                break;
        }

        if (result > 0) {
            // continue poll
            regWrite[0] = 0x00;
            //result = I2CWrite(&bbi2c, address, regWrite, 2);
            result = i2c_write_blocking(bbi2c.picoI2C, address, regWrite, 1, false);
            sleep_us(WII_EXTENSION_DELAY);
        } else {
            // device disconnected
            extensionType = WII_EXTENSION_NONE;
            //reset();
            //start();
        }
    } else {
        //reset();
        //start();
    }
}

int8_t WiiExtension::decodeByte(int8_t val) {
    return (val ^ 0x17) + 0x17;
}
