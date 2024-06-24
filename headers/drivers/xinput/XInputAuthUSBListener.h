#ifndef _XINPUTAUTHUSBLISTENER_H_
#define _XINPUTAUTHUSBLISTENER_H_

#include "usblistener.h"

class XInputAuthUSBListener : public USBListener {
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
private:
    uint8_t xinput_dev_addr;
    uint8_t xinput_instance;
    bool mounted;
};

#endif // _XINPUTAUTHUSBLISTENER_H_
