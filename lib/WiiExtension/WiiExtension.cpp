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
        } else if (idRead[5] == 0x11) {
            extensionType = WII_EXTENSION_TAIKO;
            extensionController = new TaikoExtension();
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
            
            doI2CRead(idRead, 16);
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

void WiiExtension::reset(){
    uint8_t regWrite[16];
    int8_t result;
    bool canContinue = true;

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::reset\n");
#endif

    if (canContinue) {
        result = doI2CTest();
        canContinue = (result == 1);
    }

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::i2C tested? %1d\n", result);
#endif

#if WII_EXTENSION_ENCRYPTION==true
    if (canContinue) {
        regWrite[0] = 0x40;
        regWrite[1] = 0x00;
        result = doI2CWrite(regWrite, 2);
        canContinue = (result > -1);
    }
#endif

#if WII_EXTENSION_ENCRYPTION==false
    if (canContinue) {
        regWrite[0] = 0xF0;
        regWrite[1] = 0x55;
        result = doI2CWrite(regWrite, 2);
        canContinue = (result > -1);
    }

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::reset 0xF0? %1d\n", result);
#endif

    if (canContinue) {
        regWrite[0] = 0xFB;
        regWrite[1] = 0x00;
        result = doI2CWrite(regWrite, 2);
        canContinue = (result > -1);
    }
#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::reset 0xFB? %1d\n", result);
#endif
#endif

    if (canContinue) {
        // set data format
        regWrite[0] = 0xFE;
        regWrite[1] = 0x03;
        result = doI2CWrite(regWrite, 2);
        canContinue = (result > -1);
    }

#if WII_EXTENSION_DEBUG==true
    printf("WiiExtension::reset 0xFE? %1d\n", result);
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
            extensionController->process(regRead);
            extensionController->postProcess();

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

int WiiExtension::doI2CWrite(uint8_t *pData, int iLen) {
    int result = i2c_write_blocking(picoI2C, address, pData, iLen, false);
    waitUntil_us(WII_EXTENSION_DELAY);
    return result;
}

int WiiExtension::doI2CRead(uint8_t *pData, int iLen) {
    int result = i2c_read_blocking(picoI2C, address, pData, iLen, false);
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
