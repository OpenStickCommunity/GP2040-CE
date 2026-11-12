#ifndef _XBONEAUTH_H_
#define _XBONEAUTH_H_

#include "drivers/shared/gpauthdriver.h"
#include "drivers/shared/xgip_protocol.h"
#include "pico/util/queue.h"

#define XBONE_RELAY_PACKET_SIZE 64
#define XBONE_RELAY_QUEUE_DEPTH 24

typedef struct {
    uint8_t data[XBONE_RELAY_PACKET_SIZE];
    uint16_t len;
} XBOneRelayPacket;

// GIP_AUTH / GIP_FINAL_AUTH, or an ACK of one (acked command in byte 5)
static inline bool xbone_is_auth_packet(const uint8_t * data, uint16_t len) {
    if ( len < 4 ) return false;
    if ( data[0] == 0x06 || data[0] == 0x1E ) return true;
    return ( data[0] == 0x01 && len >= 6 && (data[5] == 0x06 || data[5] == 0x1E) );
}

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

    void setBuffer(const uint8_t * inData, uint16_t inLen, uint8_t inSeq = 0, uint8_t inType = 0) {
        reset();
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

    bool auth_passthrough_enabled = false;

    volatile bool auth_passthrough = false;

    queue_t relayToDevice;
    queue_t relayToConsole;
    uint32_t relayDropped = 0;

    // Console only authenticates once per connection: reconnect on late plug-in
    volatile bool deviceMounted = false;
    volatile bool consoleAuthOrphaned = false;
    volatile bool reconnectRequested = false;
} XboxOneAuthData;

class XBOneAuth : public GPAuthDriver {
public:
    virtual void initialize();
    virtual bool available();
    void process();
    void processHost(); // core 0 only
    XboxOneAuthData * getAuthData() { return &xboxOneAuthData; }
private:
    XboxOneAuthData xboxOneAuthData;
};

#endif
