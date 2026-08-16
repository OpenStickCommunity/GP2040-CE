#ifndef _MAYFLASHS5AUTHUSBLISTENER_H_
#define _MAYFLASHS5AUTHUSBLISTENER_H_

#include "usblistener.h"
#include "drivers/mayflashs5/MayflashS5Driver.h"
#include "drivers/mayflashs5/MayflashS5Auth.h"

class MayflashS5AuthUSBListener : public USBListener {
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
    enum class S5ResetState : uint8_t {
        idle,
        waiting_before_reset,
        reset_asserted,
        awaiting_remount,
    };

    bool host_get_report(uint8_t report_id, void* report, uint16_t len);
    bool host_set_report(uint8_t report_id, void* report, uint16_t len);
    uint64_t mount_probe_us = 0;
    uint8_t probe_retries = 0;
    uint64_t last_response_us = 0;
    void performS5Encryption(uint8_t *inData, uint8_t *outData);
    bool generateMayflashBuffer();
    void copyMayflashToFinish(uint8_t const* report);
    uint8_t ps_dev_addr;
    uint8_t ps_instance;
    PS5AuthData * ps5AuthData;
    //uint8_t f1_num;
    uint8_t report_buffer[PS5_ENDPOINT_SIZE];   // Report buffer
    bool awaiting_cb; // Waiting for callback
    bool local_auth_complete;
    S5ResetState reset_state;
    uint64_t reset_action_us;
};

#endif // _MAYFLASHS5AUTHUSBLISTENER_H_
