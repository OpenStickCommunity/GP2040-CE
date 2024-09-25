#ifndef _XINPUTAUTH_H_
#define _XINPUTAUTH_H_

#include "drivers/shared/gpauthdriver.h"

class XInputAuthBuffer {
public:
    XInputAuthBuffer() {
        data = nullptr;
        length = 0;
    }
    ~XInputAuthBuffer(){
        if ( data != nullptr ) {
            delete [] data;
        }
    }

    void setBuffer(uint8_t * inData, uint16_t inLen) {
        data = new uint8_t[inLen];
        length = inLen;
        memcpy(data, inData, inLen);
    }

    void reset() {
        if ( data != nullptr ) {
            delete [] data;
        }
        data = nullptr;
        length = 0;
    }

    uint8_t * data;
    uint16_t length;
};

#define X360_AUTHLEN_CONSOLE_INIT 34
#define X360_AUTHLEN_DONGLE_SERIAL 29
#define X360_AUTHLEN_DONGLE_INIT 46
#define X360_AUTHLEN_CHALLENGE 22

// We need to keep track of:
//     Xbox 360 Console Auth Init 34 bytes
//     Dongle Serial 29 bytes
//     Console-Dongle Back and Forth 46 bytes & 22 bytes
typedef struct {
    GPAuthState xinputState;
    uint8_t consoleInitialAuth[X360_AUTHLEN_CONSOLE_INIT]; // Console Init (Keep when Dongle Reboots)
    uint8_t dongleSerial[X360_AUTHLEN_DONGLE_SERIAL];       // Dongle Serial
    uint8_t passthruBuffer[X360_AUTHLEN_DONGLE_INIT];     // Back-and-Forth Buffer (46 or 22 bytes)
    uint8_t passthruBufferLen;      // Length of Passthru (do we need this?)
    uint8_t passthruBufferID;       // ID of vendor request
    bool authCompleted = false;
    bool hasInitAuth = false;
    bool dongle_ready = false;
} XInputAuthData;

class XInputAuth : public GPAuthDriver {
public:
    virtual void initialize();
    virtual bool available();
    void process();
    XInputAuthData * getAuthData() { return &xinputAuthData; }
private:
    XInputAuthData xinputAuthData;
};

#endif
