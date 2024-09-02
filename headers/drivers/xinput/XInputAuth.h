#ifndef _XINPUTAUTH_H_
#define _XINPUTAUTH_H_

#include "drivers/shared/gpauthdriver.h"

typedef enum {
    auth_idle_state = 0,
    send_auth_console_to_dongle = 1,
    send_auth_dongle_to_console = 2,
    wait_auth_console_to_dongle = 3,
    wait_auth_dongle_to_console = 4,
} XInputAuthState;

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

typedef struct {
    XInputAuthState xinputState;
    XInputAuthBuffer consoleBuffer;
    XInputAuthBuffer dongleBuffer;
    bool authCompleted = false;
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
