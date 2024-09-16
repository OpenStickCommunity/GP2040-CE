#ifndef _XINPUTAUTHUSBLISTENER_H_
#define _XINPUTAUTHUSBLISTENER_H_

#include "usblistener.h"
#include "XInputAuth.h"
#include "usbhostmanager.h"

#include "drivers/shared/xinput_host.h"
#include "drivers/xinput/XInputDescriptors.h"

typedef enum {
    DONGLE_AUTH_IDLE = 0,
    DONGLE_AUTH_WAIT_STATE
} DONGLE_AUTH_STATE;

class XInputAuthUSBListener : public USBListener {
public:
    virtual void setup();
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len){}
    virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype);
    virtual void unmount(uint8_t dev_addr);
    virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
    virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len){}
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len){}
    void process();
    void setAuthData(XInputAuthData *);
private:
    bool xinputh_vendor_report(tusb_dir_t dir, uint8_t request, uint16_t value, uint16_t length, uint8_t * recvBuf, uintptr_t user_data);
    // Helper functions for Xbox 360 Authentication
    bool auth_dongle_get_serial();
    bool auth_dongle_init_challenge();
    bool auth_dongle_challenge_verify();
    bool auth_dongle_data_reply(uint8_t replyLen);
    bool auth_dongle_wait_get_state();
    bool auth_dongle_keepalive();
    void auth_dongle_wait(uint8_t waitID);
    uint8_t xinput_dev_addr;
    uint8_t xinput_instance;
    bool sending;
    XInputAuthData * xinputAuthData;
    uint32_t wait_time;
    uint8_t wait_count;
    uint8_t waitBuffer[64]; // wait buffer
    uint8_t waitBufferID;
    DONGLE_AUTH_STATE dongleAuthState;
};

#endif // _XINPUTAUTHUSBLISTENER_H_
