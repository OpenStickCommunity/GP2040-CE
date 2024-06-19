#ifndef _PS4AUTHUSBLISTENER_H_
#define _PS4AUTHUSBLISTENER_H_

#include "usblistener.h"
#include "PS4Driver.h"

class PS4AuthUSBListener : public USBListener {
public:
    virtual void setup();
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
    virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype){}
    virtual void unmount(uint8_t dev_addr);
    virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
    virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
    void process(PS4State pState, uint8_t pNonceId, uint8_t * pNonceBuffer); // add things to process
    void setAuthBuffer(uint8_t * buffer) { ps4_auth_buffer = buffer; }
    bool getHostAuthReady() { return ps4_auth_host_ready; }
    void resetHostAuth();
private:
    bool host_get_report(uint8_t report_id, void* report, uint16_t len);
    bool host_set_report(uint8_t report_id, void* report, uint16_t len);
    uint8_t ps_dev_addr;
    uint8_t ps_instance;
    int8_t nonce_page;
    PS4State passthrough_state;
    int8_t send_nonce_part;
    uint8_t report_buffer[64];
    bool awaiting_cb;
    bool ps4_auth_host_ready;
    uint8_t * ps4_auth_buffer;
    bool ps_mounted;
};

#endif // _PS4AUTHUSBLISTENER_H_
