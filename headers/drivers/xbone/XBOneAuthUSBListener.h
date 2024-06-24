#ifndef _XBONEAUTHUSBLISTENER_H_
#define _XBONEAUTHUSBLISTENER_H_

#include "drivers/shared/xgip_protocol.h"
#include "usblistener.h"

#include "drivers/xbone/XBOneAuth.h"

class XBOneAuthUSBListener : public USBListener {
public:
    virtual void setup();
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len){}
    virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype);
    virtual void unmount(uint8_t dev_addr);
    virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
    virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len){}
    virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len){}
    virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len){}
    void process();
    void setAuthData(XboxOneAuthData *);
private:
    void queue_host_report(void* report, uint16_t len);
    void process_report_queue();
    uint8_t xbone_dev_addr;
    uint8_t xbone_instance;
    bool mounted;
    XGIPProtocol incomingXGIP;
    XGIPProtocol outgoingXGIP;
    XboxOneAuthData * xboxOneAuthData;
};

#endif // _XBONEAUTHUSBLISTENER_H_
