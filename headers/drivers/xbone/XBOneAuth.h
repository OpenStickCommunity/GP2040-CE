#ifndef _XBONEAUTH_H_
#define _XBONEAUTH_H_

#include "drivers/shared/gpauthdriver.h"
#include "drivers/shared/xgip_protocol.h"

#include <vector>

typedef enum {
    auth_idle_state = 0,
    send_auth_console_to_dongle = 1,
    send_auth_dongle_to_console = 2,
    wait_auth_console_to_dongle = 3,
    wait_auth_dongle_to_console = 4,
} XboxOneState;

typedef enum {
    from_console = 0,
    from_dongle = 1,
} XboxOneSource;

// Xbox One Auth Buffer
typedef struct {
    uint8_t * buffer;
    uint8_t sequence;
    uint16_t len;
    uint8_t type;
    XboxOneSource source;
} XboxOneAuthBuffer;

typedef struct {
    XboxOneState xboneState;

    // Console-to-Host e.g. Xbox One to MagicBoots
    //  Note: the Xbox One Passthrough can call send_xbone_report() directly but not the other way around
    bool authCompleted;

    // Auth Buffer Queue
    std::vector<XboxOneAuthBuffer> authBufferQueue;
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
