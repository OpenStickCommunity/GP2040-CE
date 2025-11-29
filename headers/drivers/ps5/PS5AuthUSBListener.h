#ifndef _PS5AUTHUSBLISTENER_H_
#define _PS5AUTHUSBLISTENER_H_

#include "usblistener.h"
#include "PS5Driver.h"
#include "PS5Auth.h"

typedef enum _PS5_DONGLE_TYPE {
    PS5_NONE = 0,       // No dongle defined
    P5General = 1,      // Besavior P5General
    MAYFLASH_S5 = 2,    // Mayflash S5
} PS5_DONGLE_TYPE;

// Only for Mayflash S5
typedef enum {
	mfs5_no_auth = 0,
	mfs5_auth_requested = 1,
	mfs5_auth_complete = 2,
	mfs5_key_encryption_ready = 3,
} MFS5AuthState; // Mayflash S5 Auth State

class PS5AuthUSBListener : public USBListener {
public:
    virtual void setup();
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len);
    virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype){}
    virtual void unmount(uint8_t dev_addr);
    virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
    virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len) {}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len);
    void process();
    void setAuthData(PS5AuthData * authData) { ps5AuthData = authData; }
    void resetHostData();
private:
    bool host_get_report(uint8_t report_id, void* report, uint16_t len);
    bool host_set_report(uint8_t report_id, void* report, uint16_t len);
    void performS5Encryption(uint8_t *inData, uint8_t *outData);
    uint8_t ps_dev_addr;
    uint8_t ps_instance;
    PS5AuthData * ps5AuthData;
    MFS5AuthState mfs5AuthState;
    uint8_t f1_num;
    uint8_t report_buffer[PS5_ENDPOINT_SIZE];   // Report buffer
    PS5_DONGLE_TYPE dongle_type; // which dongle are we using?
    bool awaiting_cb; // Waiting for callback
};

#endif // _PS5AUTHUSBLISTENER_H_
