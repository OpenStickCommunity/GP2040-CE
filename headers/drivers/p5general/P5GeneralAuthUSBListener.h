#ifndef _P5GENERALAUTHUSBLISTENER_H_
#define _P5GENERALAUTHUSBLISTENER_H_

#include "usblistener.h"
#include "P5GeneralDriver.h"
#include "P5GeneralAuth.h"

class P5GeneralAuthUSBListener : public USBListener {
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
    void setAuthData(P5GeneralAuthData * authData) { p5GeneralAuthData = authData; }
    void resetHostData();
private:
    bool host_get_report(uint8_t report_id, void* report, uint16_t len);
    bool host_set_report(uint8_t report_id, void* report, uint16_t len);
    uint8_t ps_dev_addr;
    uint8_t ps_instance;
    P5GeneralAuthData * p5GeneralAuthData;
    uint8_t f1_num;
    uint8_t report_buffer[P5GENERAL_ENDPOINT_SIZE];   // Report buffer
};

#endif // _P5GENERALAUTHUSBLISTENER_H_
