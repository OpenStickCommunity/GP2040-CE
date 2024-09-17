#include "WiiExtension.h"

#include <cstring>
#include <cstdio>

WiiExtension::WiiExtension(PeripheralI2C *i2cController, uint8_t addr) {
    i2c = i2cController;
    address = addr;
}

void WiiExtension::begin() {
    doI2CInit();
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::begin\n");
#endif

    isReady = false;
}

void WiiExtension::start() {
    uint8_t idRead[32];
    uint8_t regWrite[16];
    bool canContinue = true;
    int8_t result, retryCtr;

    // we need to determine which address gets used
    if (isMotionPlus && !isExtension) {
        // Motion Plus only
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start Motion Plus detected\n");
#endif
        address = WII_MOTIONPLUS_I2C_ADDR;
    } else if (!isMotionPlus && isExtension) {
        // Extension only
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start Extension detected\n");
#endif
        address = WII_EXTENSION_I2C_ADDR;
    } else if (isMotionPlus && isExtension) {
        // Both attached
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start Motion Plus & Extension detected\n");
#endif
        address = WII_MOTIONPLUS_I2C_ADDR;
    } else {
        // Nothing attached
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start No Extension devices detected\n");
#endif
        canContinue = false;
        isReady = false;
        return;
    }

    if (!isReady) {
#if WII_EXTENSION_ENCRYPTION==false
        if (canContinue) {
            regWrite[0] = 0xF0;
            regWrite[1] = 0x55;
            result = doI2CWrite(regWrite, 2);
            canContinue = (result > -1);

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start 0xF0? %1d\n", result);
#endif
        } else {
            canContinue = false;
        }
        result = doI2CRead(regWrite, 1);

        if (canContinue) {
            regWrite[0] = 0xFB;
            regWrite[1] = 0x00;
            result = doI2CWrite(regWrite, 2);
            canContinue = (result > -1);

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start 0xFB? %1d\n", result);
#endif
        } else {
            canContinue = false;
        }
#endif

        result = doI2CRead(regWrite, 1);

        if (canContinue) {
            for (retryCtr = 0; retryCtr < 3; retryCtr++) {
                // set data format
                regWrite[0] = 0xFE;
                if (isMotionPlus) {
                    //regWrite[1] = 0x04;
                    regWrite[1] = 0x05;
                } else {
                    regWrite[1] = 0x03;
                }
                result = doI2CWrite(regWrite, 2);

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start 0xFE? %1d\n", result);
#endif

                // no error returned, check the device ID
                if (result > -1) {
                    if (isMotionPlus) {
                        // switch address back
                        address = WII_EXTENSION_I2C_ADDR;
                    }
                    
                    result = doI2CRead(idRead, 2);
                    isReady = true;

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start 0xFE Check %02x %02x\n", idRead[0], idRead[1]);
#endif
                    break;
                }
            }
        }

        result = doI2CRead(regWrite, 1);
    }

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start Is Ready? %01d\n", isReady);
#endif

    if (isReady) {
        // fetch the extension ID
        regWrite[0] = 0xFA;
        result = doI2CWrite(&regWrite[0], 1);

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::start Extension Check? %1d\n", result);
#endif

        extensionType = WII_EXTENSION_NONE;

        // continue if the write was successful
        if (result > -1) {
            result = doI2CRead(idRead, 6);

            if (idRead[2] != 0xA4 || idRead[3] != 0x20) return;

            if (idRead[5] == 0x00) {
                extensionType = WII_EXTENSION_NUNCHUCK;
                extensionController = new NunchuckExtension();
            } else if (idRead[5] == 0x01) {
                extensionType = WII_EXTENSION_CLASSIC;
                //if (idRead[0] == 0x01) {
                //    extensionType = WII_EXTENSION_CLASSIC_PRO;
                //}
                extensionController = new ClassicExtension();
            } else if (idRead[5] == 0x03) {
                extensionType = WII_EXTENSION_GUITAR;
                if (idRead[0] == 0x01) {
                    extensionType = WII_EXTENSION_DRUMS;
                    extensionController = new DrumExtension();
                } else if (idRead[0] == 0x03) {
                    extensionType = WII_EXTENSION_TURNTABLE;
                    extensionController = new TurntableExtension();
                } else {
                    extensionController = new GuitarExtension();
                }
            } else if (idRead[5] == 0x05) {
                extensionType = WII_EXTENSION_MOTION_PLUS;
                extensionController = new MotionPlusExtension();
            } else if (idRead[5] == 0x11) {
                extensionType = WII_EXTENSION_TAIKO;
                extensionController = new TaikoExtension();
            } else if (idRead[5] == 0x12) {
                extensionType = WII_EXTENSION_UDRAW;
                extensionController = new UDrawExtension();
            }

            // in certain situations (eg. Nunchuck), setting the data type in reset() does not affect what this value will be
            dataType = idRead[4];
            if (dataType == WII_DATA_TYPE_0) dataType = WII_DATA_TYPE_1;
            extensionController->init(dataType);
            extensionController->setExtensionType(extensionType);

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::Extension ID: %02x%02x %02x%02x %02x%02x\n", idRead[0], idRead[1], idRead[2], idRead[3], idRead[4], idRead[5]);
    printf("WiiExtension::Data Format: %02x\n",idRead[4]);
#endif

            if (extensionType != WII_EXTENSION_NONE) {
#if WII_EXTENSION_CALIBRATION==true
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::Calibration Data\n");
#endif
                regWrite[0] = 0x20;
                doI2CWrite(regWrite, 1);

                doI2CRead(idRead, 32);
                extensionController->calibrate(idRead);
#endif
            } else {
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::Unknown Extension: %02x%02x %02x%02x %02x%02x\n", idRead[0], idRead[1], idRead[2], idRead[3], idRead[4], idRead[5]);
#endif
            }

            regWrite[0] = 0x00;
            result = doI2CWrite(regWrite, 1);
        }
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
            // Motion Plus data types
            case WII_DATA_TYPE_4:
            case WII_DATA_TYPE_5:
            case WII_DATA_TYPE_6:
            case WII_DATA_TYPE_7:
                result = doI2CRead(regRead, 16);
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
            extensionController->process(regRead);
            if (!extensionController->skipPostProcess) extensionController->postProcess();

#if WII_EXTENSION_DEBUG==true
            for (int i = 0; i < result; ++i) {
                _lastRead[i] = regRead[i];
            }
#endif

            if (extensionType == WII_EXTENSION_TURNTABLE) {
                regWrite[0] = 0xFB;
                regWrite[1] = ((TurntableExtension*)extensionController)->getLED();
                result = doI2CWrite(regWrite, 2);
            }

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

void WiiExtension::reset() {
    isReady = false;
}

int WiiExtension::doI2CWrite(uint8_t *pData, int iLen) {
    int result = i2c->write(address, pData, iLen, false);
    waitUntil_us(WII_EXTENSION_DELAY);
    return result;
}

int WiiExtension::doI2CRead(uint8_t *pData, int iLen) {
    int result = i2c->read(address, pData, iLen, false);
    waitUntil_us(WII_EXTENSION_DELAY);
#if WII_EXTENSION_ENCRYPTION==true
    for (int i = 0; i < iLen; ++i) {
        pData[i] = WII_DECRYPT_BYTE(pData[i]);
    }
#endif
    return result;
}

uint8_t WiiExtension::doI2CTest() {
    int result;
    uint8_t rxdata;
    result = doI2CRead(&rxdata, 1);
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::doI2CTest: %d\n", result);
#endif
    return (result >= 0);
}

void WiiExtension::doI2CInit() {
    isMotionPlus = false;
    isExtension = false;

    bool extensionCheck = i2c->test(WII_EXTENSION_I2C_ADDR);
    waitUntil_us(WII_EXTENSION_DELAY);

    bool motionPlusCheck = i2c->test(WII_MOTIONPLUS_I2C_ADDR);
    waitUntil_us(WII_EXTENSION_DELAY);

    // since init is unused, let's use this to detect normal or MotionPlus mode
    if (extensionCheck) isExtension = true;
    if (motionPlusCheck) isMotionPlus = true;
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
