#include "WiiExtension.h"

#include <cstring>
#include <cstdio>

WiiExtension::WiiExtension(int sda, int scl, i2c_inst_t *i2cCtl, int32_t speed, uint8_t addr) {
    iSDA = sda;
    iSCL = scl;
    picoI2C = i2cCtl;
    iSpeed = speed;
    address = addr;
}

void WiiExtension::begin() {
    doI2CInit();
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::begin\n");
#endif
    isReady = false;
    reset();
}

void WiiExtension::start(){
    uint8_t idRead[32];
    uint8_t regWrite[16];
    int8_t result;

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start\n");
    printf("WiiExtension::start isReady? %1d\n", isReady);
#endif

    if (!isReady) return;

    regWrite[0] = 0xFA;
    result = doI2CWrite(&regWrite[0], 1);

    extensionType = WII_EXTENSION_NONE;

    // continue if the write was successful
    if (result > -1) {
        doI2CRead(idRead, 6);

        if (idRead[2] != 0xA4 || idRead[3] != 0x20) return;

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
        } else if (idRead[5] == 0x11) {
            extensionType = WII_EXTENSION_TAIKO;
        }

        // in certain situations (eg. Nunchuck), setting the data type in reset() does not affect what this value will be
        dataType = idRead[4];
        if (dataType == WII_DATA_TYPE_0) dataType = WII_DATA_TYPE_1;

#if WII_EXTENSION_DEBUG==true
        printf("WiiExtension::Extension ID: %02x%02x %02x%02x %02x%02x\n", idRead[0], idRead[1], idRead[2], idRead[3], idRead[4], idRead[5]);
        printf("WiiExtension::Data Format: %02x\n",idRead[4]);
#endif

        if (extensionType != WII_EXTENSION_NONE) {
#if WII_EXTENSION_DEBUG==true
            //printf("Extension Type: %d\n", extensionType);
#endif

#if WII_EXTENSION_DEBUG==true
            printf("WiiExtension::Calibration Data\n");
#endif
            if (extensionType == WII_EXTENSION_NUNCHUCK) {
                _analogPrecision1From = WII_ANALOG_PRECISION_2;
                _analogPrecision1To = WII_ANALOG_PRECISION_3;

#if WII_EXTENSION_CALIBRATION==true
                // read calibration
                regWrite[0] = 0x20;
                doI2CWrite(regWrite, 1);

                doI2CRead(idRead, 16);

                _maxX1 = idRead[8];
                _minX1 = idRead[9];
                _cenX1 = idRead[10];

                _maxY1 = idRead[11];
                _minY1 = idRead[12];
                _cenY1 = idRead[13];

                _accelX0G = ((idRead[0] << 2) | ((idRead[3] >> 2) & 0x03));
                _accelY0G = ((idRead[1] << 2) | ((idRead[3] >> 4) & 0x03));
                _accelZ0G = ((idRead[2] << 2) | ((idRead[3] >> 6) & 0x03));

                _accelX1G = ((idRead[4] << 2) | ((idRead[7] >> 2) & 0x03));
                _accelY1G = ((idRead[5] << 2) | ((idRead[7] >> 4) & 0x03));
                _accelZ1G = ((idRead[6] << 2) | ((idRead[7] >> 6) & 0x03));

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
#endif
            } else {
                if (dataType == WII_DATA_TYPE_1) {
                    _analogPrecision1From = WII_ANALOG_PRECISION_1;
                    _analogPrecision1To = WII_ANALOG_PRECISION_3;
                    _analogPrecision2From = WII_ANALOG_PRECISION_0;
                    _analogPrecision2To = WII_ANALOG_PRECISION_3;

                    _triggerPrecision1From = WII_ANALOG_PRECISION_0;
                    _triggerPrecision1To = WII_ANALOG_PRECISION_2;
                    _triggerPrecision2From = WII_ANALOG_PRECISION_0;
                    _triggerPrecision2To = WII_ANALOG_PRECISION_2;
                } else if (dataType == WII_DATA_TYPE_2) {
                    _analogPrecision1From = WII_ANALOG_PRECISION_3;
                    _analogPrecision1To = WII_ANALOG_PRECISION_3;
                    _analogPrecision2From = WII_ANALOG_PRECISION_3;
                    _analogPrecision2To = WII_ANALOG_PRECISION_3;

                    _triggerPrecision1From = WII_ANALOG_PRECISION_2;
                    _triggerPrecision1To = WII_ANALOG_PRECISION_2;
                    _triggerPrecision2From = WII_ANALOG_PRECISION_2;
                    _triggerPrecision2To = WII_ANALOG_PRECISION_2;
                } else if (dataType == WII_DATA_TYPE_3) {
                    _analogPrecision1From = WII_ANALOG_PRECISION_2;
                    _analogPrecision1To = WII_ANALOG_PRECISION_3;
                    _analogPrecision2From = WII_ANALOG_PRECISION_2;
                    _analogPrecision2To = WII_ANALOG_PRECISION_3;

                    _triggerPrecision1From = WII_ANALOG_PRECISION_2;
                    _triggerPrecision1To = WII_ANALOG_PRECISION_2;
                    _triggerPrecision2From = WII_ANALOG_PRECISION_2;
                    _triggerPrecision2To = WII_ANALOG_PRECISION_2;
                }

#if WII_EXTENSION_CALIBRATION==true
                regWrite[0] = 0x20;
                doI2CWrite(regWrite, 1);
                
                doI2CRead(idRead, 16);

                if (dataType == WII_DATA_TYPE_1) {
                    _calibrationPrecision1From = WII_ANALOG_PRECISION_2;
                    _calibrationPrecision1To   = WII_ANALOG_PRECISION_1;
                    _calibrationPrecision2From = WII_ANALOG_PRECISION_2;
                    _calibrationPrecision2To   = WII_ANALOG_PRECISION_0;
                } else if (dataType == WII_DATA_TYPE_2) {
                    _calibrationPrecision1From = WII_ANALOG_PRECISION_2;
                    _calibrationPrecision1To   = WII_ANALOG_PRECISION_3;
                    _calibrationPrecision2From = WII_ANALOG_PRECISION_2;
                    _calibrationPrecision2To   = WII_ANALOG_PRECISION_3;
                } else if (dataType == WII_DATA_TYPE_3) {
                    _calibrationPrecision1From = WII_ANALOG_PRECISION_2;
                    _calibrationPrecision1To   = WII_ANALOG_PRECISION_2;
                    _calibrationPrecision2From = WII_ANALOG_PRECISION_2;
                    _calibrationPrecision2To   = WII_ANALOG_PRECISION_2;
                }

                _maxX1 = map(idRead[0],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
                _minX1 = map(idRead[1],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
                _cenX1 = map(idRead[2],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
                
                _maxY1 = map(idRead[3],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
                _minY1 = map(idRead[4],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
                _cenY1 = map(idRead[5],0,(_calibrationPrecision1From-1),0,(_calibrationPrecision1To-1));
                
                _maxX2 = map(idRead[6],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
                _minX2 = map(idRead[7],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
                _cenX2 = map(idRead[8],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
                
                _maxY2 = map(idRead[9],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
                _minY2 = map(idRead[10],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));
                _cenY2 = map(idRead[11],0,(_calibrationPrecision2From-1),0,(_calibrationPrecision2To-1));

#if WII_EXTENSION_DEBUG==true
                printf("X1 Min: %d\n", _minX1);
                printf("X1 Max: %d\n", _maxX1);
                printf("X1 Center: %d\n", _cenX1);
                printf("Y1 Min: %d\n", _minY1);
                printf("Y1 Max: %d\n", _maxY1);
                printf("Y1 Center: %d\n", _cenY1);
                printf("X2 Min: %d\n", _minX2);
                printf("X2 Max: %d\n", _maxX2);
                printf("X2 Center: %d\n", _cenX2);
                printf("Y2 Min: %d\n", _minY2);
                printf("Y2 Max: %d\n", _maxY2);
                printf("Y2 Center: %d\n", _cenY2);
#endif
#endif
            }

            // reset to default input values in the event of a removal/hotswap
            joy1X           = 0;
            joy1Y           = 0;
            joy2X           = 0;
            joy2Y           = 0;
            accelX          = 0;
            accelY          = 0;
            accelZ          = 0;

            buttonZ         = 0;
            buttonC         = 0;
            buttonZR        = 0;
            buttonZL        = 0;
            buttonA         = 0;
            buttonB         = 0;
            buttonX         = 0;
            buttonY         = 0;
            buttonPlus      = 0;
            buttonHome      = 0;
            buttonMinus     = 0;
            buttonLT        = 0;
            buttonRT        = 0;

            directionUp     = 0;
            directionDown   = 0;
            directionLeft   = 0;
            directionRight  = 0;

            triggerLeft     = 0;
            triggerRight    = 0;

            fretGreen       = 0;
            fretRed         = 0;
            fretYellow      = 0;
            fretBlue        = 0;
            fretOrange      = 0;
            pedalButton     = 0;

            rimLeft         = 0;
            rimRight        = 0;
            drumLeft        = 0;
            drumRight       = 0;

            whammyBar       = 0;
            touchBar        = 0;

            _guitarType     = WII_GUITAR_UNSET;
        } else {
#if WII_EXTENSION_DEBUG==true
            printf("WiiExtension::Unknown Extension: %02x%02x %02x%02x %02x%02x\n", idRead[0], idRead[1], idRead[2], idRead[3], idRead[4], idRead[5]);
#endif
        }

        regWrite[0] = 0x00;
        result = doI2CWrite(regWrite, 1);
    }
}

void WiiExtension::reset(){
    uint8_t regWrite[16];
    int8_t result;
    bool canContinue = true;

    if (canContinue) {
        result = doI2CTest();
        canContinue = (result == 1);
    }

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::reset\n");
#endif

    if (canContinue) {
        regWrite[0] = 0xF0;
        regWrite[1] = 0x55;
        result = doI2CWrite(regWrite, 2);
        canContinue = (result > -1);
    }

    if (canContinue) {
        regWrite[0] = 0xFB;
        regWrite[1] = 0x00;
        result = doI2CWrite(regWrite, 2);
        canContinue = (result > -1);
    }

    if (canContinue) {
        // set data format
        regWrite[0] = 0xFE;
        regWrite[1] = 0x03;
        result = doI2CWrite(regWrite, 2);
        canContinue = (result > -1);
    }

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
    uint8_t regWrite[16];
    uint8_t regRead[16];
    int8_t result;

#if WII_EXTENSION_DEBUG==true
    //printf("WiiExtension::poll\n");
    //printf("WiiExtension::poll isReady? %1d\n", isReady);
#endif

    if (!isReady) return;

    if (extensionType != WII_EXTENSION_NONE) {
        switch (dataType) {
            case WII_DATA_TYPE_1:
                result = doI2CRead(regRead, 6);
                break;
            case WII_DATA_TYPE_2:
                result = doI2CRead(regRead, 9);
                break;
            case WII_DATA_TYPE_3:
                result = doI2CRead(regRead, 8);
                break;
            default:
                // unknown. TBD
                result = -1;
#if WII_EXTENSION_DEBUG==true
                printf("WiiExtension::poll Unknown data type: %1d\n", dataType);
#endif
                break;
        }

        if (result > 0) {
            switch (extensionType) {
                case WII_EXTENSION_NUNCHUCK:
                    joy1X = (regRead[0] & 0xFF);
                    joy1Y = (regRead[1] & 0xFF);

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
                    if (dataType == WII_DATA_TYPE_1) {
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
                    } else if (dataType == WII_DATA_TYPE_2) {
                        joy1X =          ((regRead[0] << 2) | ((regRead[4] & 0x03) >> 0));
                        joy1Y =          ((regRead[2] << 2) | ((regRead[4] & 0x30) >> 4));
                        joy2X =          ((regRead[1] << 2) | ((regRead[4] & 0x0C) >> 2));
                        joy2Y =          ((regRead[3] << 2) | ((regRead[4] & 0xC0) >> 6));

                        triggerLeft =    (regRead[5] & 0xFF);
                        triggerRight =   (regRead[6] & 0xFF);

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
                    } else if (dataType == WII_DATA_TYPE_3) {
                        joy1X =          (regRead[0] & 0xFF);
                        joy1Y =          (regRead[2] & 0xFF);
                        joy2X =          (regRead[1] & 0xFF);
                        joy2Y =          (regRead[3] & 0xFF);

                        triggerLeft =    (regRead[4] & 0xFF);
                        triggerRight =   (regRead[5] & 0xFF);

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
                //if ((_lastRead[0] != regRead[0]) || (_lastRead[1] != regRead[1]) || (_lastRead[2] != regRead[2]) || (_lastRead[3] != regRead[3])) {
                    printf("Joy1 X=%4d Y=%4d  Joy2 X=%4d Y=%4d\n", joy1X, joy1Y, joy2X, joy2Y);
                //}
                //printf("Joy1 X=%4d Y=%4d  Joy2 X=%4d Y=%4d  U=%1d D=%1d L=%1d R=%1d TL=%4d TR=%4d\n", joy1X, joy1Y, joy2X, joy2Y, directionUp, directionDown, directionLeft, directionRight, triggerLeft, triggerRight);
                //printf("A=%1d B=%1d X=%1d Y=%1d ZL=%1d ZR=%1d LT=%1d RT=%1d -=%1d H=%1d +=%1d\n", buttonA, buttonB, buttonX, buttonY, buttonZL, buttonZR, buttonLT, buttonRT, buttonMinus, buttonHome, buttonPlus);
#endif

                    break;
                case WII_EXTENSION_GUITAR:
                    // on first read, check the status of the guitar flag
                    if (_guitarType == WII_GUITAR_UNSET) {
                        if (((regRead[0] & 0x80) >> 7) == 0) {
                            _guitarType = WII_GUITAR_GHWT;
                        } else {
                            _guitarType = WII_GUITAR_GH3;
                        }
                        // force the data type to 1 when a World Tour guitar is detected
                        if ((_guitarType == WII_GUITAR_GHWT) && (dataType != WII_DATA_TYPE_1)) {
                            dataType = WII_DATA_TYPE_1;
                            _analogPrecision1From = WII_ANALOG_PRECISION_1;
                            _analogPrecision1To = WII_ANALOG_PRECISION_3;
                            _analogPrecision2From = WII_ANALOG_PRECISION_0;
                            _analogPrecision2To = WII_ANALOG_PRECISION_3;
                        }
                    }
                    if (_guitarType != WII_GUITAR_UNSET) {
                        // as defined works for GH3 guitar
                        if (dataType == WII_DATA_TYPE_1) {
                            joy1X =          (regRead[0] & 0x3F);
                            joy1Y =          (regRead[1] & 0x3F);

                            touchBar =       ((_guitarType == WII_GUITAR_GHWT) ? (regRead[2] & 0x1F) : 0);

                            whammyBar =      (regRead[3] & 0x1F);
                            joy2X =          (regRead[3] & 0x1F);

                            directionDown =  !((regRead[4] & 0x40) >> 6);
                            buttonMinus =    !((regRead[4] & 0x10) >> 4);
                            buttonPlus =     !((regRead[4] & 0x04) >> 2);

                            fretOrange =     !((regRead[5] & 0x80) >> 7);
                            fretRed =        !((regRead[5] & 0x40) >> 6);
                            fretBlue =       !((regRead[5] & 0x20) >> 5);
                            fretGreen =      !((regRead[5] & 0x10) >> 4);
                            fretYellow =     !((regRead[5] & 0x08) >> 3);
                            pedalButton =    !((regRead[5] & 0x04) >> 2);
                            directionUp =    !((regRead[5] & 0x01) >> 0);

                            isTouched        = (touchBar != WII_GUITAR_TOUCHPAD_NONE);

                            // process the touch bar for button states
                            // touch only seems to exist in GHWT, and GHWT always reports data type 1 format regardless of setting
                            if (isTouched) {
                                // touched
                                fretGreen     = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_GREEN,WII_GUITAR_TOUCHPAD_RED));
                                fretRed       = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_RED,WII_GUITAR_TOUCHPAD_YELLOW));
                                fretYellow    = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_YELLOW,WII_GUITAR_TOUCHPAD_BLUE));
                                fretBlue      = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_BLUE,WII_GUITAR_TOUCHPAD_ORANGE));
                                fretOrange    = (TOUCH_BETWEEN_RANGE(touchBar,WII_GUITAR_TOUCHPAD_ORANGE,WII_GUITAR_TOUCHPAD_MAX));
                                directionDown = isTouched;
                            }
                        } else if (dataType == WII_DATA_TYPE_2) {
                            joy1X =          ((regRead[0] << 2) | ((regRead[4] & 0x03) >> 0));
                            joy1Y =          ((regRead[2] << 2) | ((regRead[4] & 0x30) >> 4));

                            touchBar =       0;

                            whammyBar =      (regRead[6] & 0xFF);
                            joy2X =          (regRead[6] & 0xFF);

                            directionDown =  !((regRead[7] & 0x40) >> 6);
                            buttonMinus =    !((regRead[7] & 0x10) >> 4);
                            buttonPlus =     !((regRead[7] & 0x04) >> 2);

                            fretOrange =     !((regRead[8] & 0x80) >> 7);
                            fretRed =        !((regRead[8] & 0x40) >> 6);
                            fretBlue =       !((regRead[8] & 0x20) >> 5);
                            fretGreen =      !((regRead[8] & 0x10) >> 4);
                            fretYellow =     !((regRead[8] & 0x08) >> 3);
                            pedalButton =    !((regRead[8] & 0x04) >> 2);
                            directionUp =    !((regRead[8] & 0x01) >> 0);
                        } else if (dataType == WII_DATA_TYPE_3) {
                            joy1X =          (regRead[0] & 0xFF);
                            joy1Y =          (regRead[2] & 0xFF);

                            touchBar =       0;

                            whammyBar =      (regRead[5] & 0xFF);
                            joy2X =          (regRead[5] & 0xFF);

                            directionDown =  !((regRead[6] & 0x40) >> 6);
                            buttonMinus =    !((regRead[6] & 0x10) >> 4);
                            buttonPlus =     !((regRead[6] & 0x04) >> 2);

                            fretOrange =     !((regRead[7] & 0x80) >> 7);
                            fretRed =        !((regRead[7] & 0x40) >> 6);
                            fretBlue =       !((regRead[7] & 0x20) >> 5);
                            fretGreen =      !((regRead[7] & 0x10) >> 4);
                            fretYellow =     !((regRead[7] & 0x08) >> 3);
                            pedalButton =    !((regRead[7] & 0x04) >> 2);
                            directionUp =    !((regRead[7] & 0x01) >> 0);
                        }
                    }
#if WII_EXTENSION_DEBUG==true
//                printf("Joy1 X=%4d Y=%4d  Whammy=%4d  U=%1d D=%1d -=%1d +=%1d\n", joy1X, joy1Y, whammyBar, directionUp, directionDown, buttonMinus, buttonPlus);
//                printf("Joy1 X=%4d Y=%4d  Whammy=%4d  U=%1d D=%1d -=%1d +=%1d\n", joy1X, joy1Y, whammyBar, directionUp, directionDown, buttonMinus, buttonPlus);
//                printf("O=%1d B=%1d Y=%1d R=%1d G=%1d\n", fretOrange, fretBlue, fretYellow, fretRed, fretGreen);
#endif
                    break;
                case WII_EXTENSION_TAIKO:
                    if (dataType == WII_DATA_TYPE_1) {
                        drumLeft        = !((regRead[5] & 0x40) >> 6);
                        rimLeft         = !((regRead[5] & 0x20) >> 5);
                        drumRight       = !((regRead[5] & 0x10) >> 4);
                        rimRight        = !((regRead[5] & 0x08) >> 3);
                    } else if (dataType == WII_DATA_TYPE_2) {
                        drumLeft        = !((regRead[8] & 0x40) >> 6);
                        rimLeft         = !((regRead[8] & 0x20) >> 5);
                        drumRight       = !((regRead[8] & 0x10) >> 4);
                        rimRight        = !((regRead[8] & 0x08) >> 3);
                    } else if (dataType == WII_DATA_TYPE_3) {
                        drumLeft        = !((regRead[7] & 0x40) >> 6);
                        rimLeft         = !((regRead[7] & 0x20) >> 5);
                        drumRight       = !((regRead[7] & 0x10) >> 4);
                        rimRight        = !((regRead[7] & 0x08) >> 3);
                    }

#if WII_EXTENSION_DEBUG==true
                //if (_lastRead[0] != regRead[0]) printf("Byte0    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[0]));
                //if (_lastRead[1] != regRead[1]) printf("Byte1    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[1]));
                //if (_lastRead[2] != regRead[2]) printf("Byte2    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[2]));
                //if (_lastRead[3] != regRead[3]) printf("Byte3    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[3]));
                //if (_lastRead[4] != regRead[4]) printf("Byte4    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[4]));
                //if (_lastRead[5] != regRead[5]) printf("Byte5    " BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY(regRead[5]));
                //
                //if (_lastRead[7] != regRead[7]) {
                //    printf("DL=%1d RL=%1d DR=%1d RR=%1d\n", drumLeft, rimLeft, drumRight, rimRight);
                //}
#endif

                    break;
            }

            // calibrate and remap
            joy1X   = map(
                calibrate(joy1X, _minX1, _maxX1, _cenX1),
                0+_minX1,
                (_analogPrecision1From-_maxX1),
                0,
                (_analogPrecision1To-1)
            );
            joy1Y   = map(
                calibrate(joy1Y, _minY1, _maxY1, _cenY1),
                0+_minY1,
                (_analogPrecision1From-_maxY1),
                0,
                (_analogPrecision1To-1)
            );

            joy2X   = map(
                calibrate(joy2X, _minX2, _maxX2, _cenX2),
                0+_minX2,
                (_analogPrecision2From-_maxX2),
                0,
                (_analogPrecision2To-1)
            );
            joy2Y   = map(
                calibrate(joy2Y, _minY2, _maxY2, _cenY2),
                0+_minY2,
                (_analogPrecision2From-_maxY2),
                0,
                (_analogPrecision2To-1)
            );

            triggerLeft  = map(
                triggerLeft,
                0,
                (_triggerPrecision1From-1),
                0,
                (_triggerPrecision1To-1)
            );
            triggerRight = map(
                triggerRight,
                0,
                (_triggerPrecision2From-1),
                0,
                (_triggerPrecision2To-1)
            );

#if WII_EXTENSION_DEBUG==true
            //if ((_lastRead[0] != regRead[0]) || (_lastRead[1] != regRead[1]) || (_lastRead[2] != regRead[2]) || (_lastRead[3] != regRead[3])) {
            //    printf("Joy1 X=%4d Y=%4d  Joy2 X=%4d Y=%4d\n", joy1X, joy1Y, joy2X, joy2Y);
            //}
            //printf("Joy1 X=%4d Y=%4d  Joy2 X=%4d Y=%4d  U=%1d D=%1d L=%1d R=%1d TL=%4d TR=%4d\n", joy1X, joy1Y, joy2X, joy2Y, directionUp, directionDown, directionLeft, directionRight, triggerLeft, triggerRight);
            //printf("A=%1d B=%1d X=%1d Y=%1d ZL=%1d ZR=%1d LT=%1d RT=%1d -=%1d H=%1d +=%1d\n", buttonA, buttonB, buttonX, buttonY, buttonZL, buttonZR, buttonLT, buttonRT, buttonMinus, buttonHome, buttonPlus);
            for (int i = 0; i < result; ++i) {
                _lastRead[i] = regRead[i];
            }
#endif
            // continue poll
            regWrite[0] = 0x00;
            result = doI2CWrite(regWrite, 1);
        } else {
            // device disconnected or invalid read
            extensionType = WII_EXTENSION_NONE;
            reset();
            start();
        }
    } else {
        reset();
        start();
    }
}

uint16_t WiiExtension::map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint16_t WiiExtension::calibrate(uint16_t pos, uint16_t min, uint16_t max, uint16_t cen) {
    uint16_t result;

#if WII_EXTENSION_CALIBRATION==true
    if (pos >= min && pos <= max) {
        result = pos;
    } else {
        if (pos < min) {
            result = min;
        } else if (pos > max) {
            result = max;
        } else {
            result = cen;
        }
    }
#else
    result = pos;
#endif

    return result;
}

int WiiExtension::doI2CWrite(uint8_t *pData, int iLen) {
    int result = i2c_write_blocking(picoI2C, address, pData, iLen, false);
    waitUntil_us(WII_EXTENSION_DELAY);
    return result;
}

int WiiExtension::doI2CRead(uint8_t *pData, int iLen) {
    int result = i2c_read_blocking(picoI2C, address, pData, iLen, false);
    waitUntil_us(WII_EXTENSION_DELAY);
    return result;
}

uint8_t WiiExtension::doI2CTest() {
	int result;
    uint8_t rxdata;
    result = doI2CRead(&rxdata, 1);
    return (result >= 0);
}

void WiiExtension::doI2CInit() {
	if ((iSDA + 2 * i2c_hw_index(picoI2C))%4 != 0) return;
	if ((iSCL + 3 + 2 * i2c_hw_index(picoI2C))%4 != 0) return;

    i2c_init(picoI2C, iSpeed);
    gpio_set_function(iSDA, GPIO_FUNC_I2C);
    gpio_set_function(iSCL, GPIO_FUNC_I2C);
    gpio_pull_up(iSDA);
    gpio_pull_up(iSCL);

    return;
}

void WiiExtension::waitUntil_us(uint64_t us) {
    WiiExtension_alarmFired = false;

    // Enable the interrupt for our alarm (the timer outputs 4 alarm irqs)
    hw_set_bits(&timer_hw->inte, 1u << WII_ALARM_NUM);
    // Set irq handler for alarm irq
    irq_set_exclusive_handler(WII_ALARM_IRQ, alarmIRQ);
    // Enable the alarm irq
    irq_set_enabled(WII_ALARM_IRQ, true);
    // Enable interrupt in block and at processor

    // Alarm is only 32 bits so if trying to delay more
    // than that need to be careful and keep track of the upper
    // bits
    uint64_t target = timer_hw->timerawl + us;

    // Write the lower 32 bits of the target time to the alarm which
    // will arm it
    timer_hw->alarm[WII_ALARM_NUM] = (uint32_t) target;

    while (!WiiExtension_alarmFired);
}

void WiiExtension::alarmIRQ() {
    // Clear the alarm irq
    hw_clear_bits(&timer_hw->intr, 1u << WII_ALARM_NUM);

    // Assume alarm 0 has fired
    WiiExtension_alarmFired = true;
}
