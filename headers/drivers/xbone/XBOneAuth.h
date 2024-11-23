#ifndef _XBONEAUTH_H_
#define _XBONEAUTH_H_

#include "drivers/shared/gpauthdriver.h"
#include "drivers/shared/xgip_protocol.h"

class XBOneAuthBuffer {
public:
    XBOneAuthBuffer() {
        data = nullptr;
        sequence = 0;
        length = 0;
        type = 0;
    }
    ~XBOneAuthBuffer(){
        if ( data != nullptr ) {
            delete [] data;
        }
    }

    void setBuffer(uint8_t * inData, uint16_t inLen, uint8_t inSeq, uint8_t inType) {
        data = new uint8_t[inLen];
        length = inLen;
        sequence = inSeq;
        type = inType;        
        memcpy(data, inData, inLen);
    }

    void reset() {
        if ( data != nullptr ) {
            delete [] data;
        }
        data = nullptr;
        sequence = 0;
        length = 0;
        type = 0;
    }

    uint8_t * data;
    uint8_t sequence;
    uint16_t length;
    uint8_t type;
};

typedef struct {
    GPAuthState xboneState;

    // Auth Buffer Queue
    XBOneAuthBuffer consoleBuffer;
    XBOneAuthBuffer dongleBuffer;
    
    // Console-to-Host e.g. Xbox One to MagicBoots
    //  Note: the Xbox One Passthrough can call send_xbone_report() directly but not the other way around
    bool authCompleted = false;

    // Send announce to console AFTER the dongle is established
    bool dongle_ready = false;
} XboxOneAuthData;

class XBOneAuth : public GPAuthDriver {
public:
    virtual void initialize();
    virtual bool available();
    void process();
    XboxOneAuthData * getAuthData() { return &xboxOneAuthData; }
private:
    XboxOneAuthData xboxOneAuthData;
};

#endif
