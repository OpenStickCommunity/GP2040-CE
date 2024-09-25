#ifndef _PS4AUTHUSBLISTENER_H_
#define _PS4AUTHUSBLISTENER_H_

#include "usblistener.h"
#include "PS4Driver.h"
#include "PS4Auth.h"

typedef enum {
    no_nonce = 0,
    receiving_nonce = 1,
    nonce_ready = 2,
    signed_nonce_ready = 3,
    sending_nonce = 4
} PS4State;

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
    void process(); // add things to process
    void setAuthData(PS4AuthData * authData) { ps4AuthData = authData; }
    void resetHostData();
private:
    bool host_get_report(uint8_t report_id, void* report, uint16_t len);
    bool host_set_report(uint8_t report_id, void* report, uint16_t len);
    uint8_t ps_dev_addr; // TinyUSB Address (USB)
    uint8_t ps_instance; // TinyUSB Instance (USB)
    PS4AuthData * ps4AuthData;      // PS4 Authentication Data
    uint8_t nonce_page;             // PS4 Encryption Nonce Page (Max 5)
    uint8_t nonce_chunk;            // PS4 Encryption Nonce Chunk (Max 19)
    uint8_t report_buffer[PS4_ENDPOINT_SIZE];   // Report buffer
    bool awaiting_cb;   // Global call-back wait
    uint8_t noncelen;   // process(): nonce-len
    uint32_t crc32;     // process(): crc32
    PS4State dongle_state;
};

#endif // _PS4AUTHUSBLISTENER_H_
