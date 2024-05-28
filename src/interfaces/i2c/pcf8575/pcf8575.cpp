#include "pcf8575.h"

#include <cstring>

void PCF8575::begin() {
    reset();
}

void PCF8575::reset(){
    send(initialValue);
}

void PCF8575::send(uint16_t value) {
    dataSent = value;
    uc[0] = ((dataSent >> 0) & 0x00FF);
    uc[1] = ((dataSent >> 8) & 0x00FF);
    i2c->write(address, uc, 2);
}

uint16_t PCF8575::receive() {
    uint8_t readBytes[2];
    i2c->read(address, readBytes, 2);
    dataReceived = ((readBytes[0] << 0) | (readBytes[1] << 8));
    return dataReceived;
}

void PCF8575::setPin(uint8_t pinNumber, uint8_t value) {
    if (pinNumber < 16) {
        if (value == 0) {
            dataSent &= ~(1 << pinNumber);
        } else {
            dataSent |= (1 << pinNumber);
        }
        send(dataSent);
    }
}

bool PCF8575::getPin(uint8_t pinNumber) {
    uint16_t value = receive();

    return (value & (1 << pinNumber)) > 0;
}