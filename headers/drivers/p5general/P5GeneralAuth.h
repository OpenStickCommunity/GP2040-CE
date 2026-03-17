#ifndef _P5GENERALAUTH_H_
#define _P5GENERALAUTH_H_

#include "drivers/shared/gpauthdriver.h"

typedef enum {
    p5g_auth_idle = 0,
    p5g_auth_send_f0,
    p5g_auth_send_f0_wait,
    p5g_auth_recv_f1,
    p5g_auth_recv_f1_wait,
    p5g_auth_recv_f2_delay_500mS,
    p5g_auth_recv_f2,
    p5g_auth_recv_f2_wait,
} P5GeneralGPAuthState;

typedef struct {
    uint8_t hash_pending_buffer[64];
    uint8_t hash_finish_buffer[64];

    uint8_t auth_buffer[64];

    uint64_t auth_recv_f2_us;

    bool dongle_ready = false;
    bool hash_pending = false;
    bool hash_ready = false;
    P5GeneralGPAuthState passthrough_state;
} P5GeneralAuthData;

class P5GeneralAuth : public GPAuthDriver {
public:
    virtual void initialize();
    virtual bool available();
    void process();
    P5GeneralAuthData * getAuthData() { return &p5GeneralAuthData; }
private:
    P5GeneralAuthData p5GeneralAuthData;
};

#endif
