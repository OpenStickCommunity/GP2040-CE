#ifndef _XINPUTAUTHUSB_H_
#define _XINPUTAUTHUSB_H_

#include "drivers/shared/gpauthdriver.h"
#include "usblistener.h"

class XInputAuthUSB : public GPAuthDriver, public USBListener {
public:
    virtual void initialize();
    virtual bool available();
    virtual void process();
    virtual void mount(uint8_t dev_addr, uint8_t instance, uint8_t const* desc_report, uint16_t desc_len){}
	virtual void xmount(uint8_t dev_addr, uint8_t instance, uint8_t controllerType, uint8_t subtype);
	virtual void unmount(uint8_t dev_addr);
	virtual void report_received(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len);
	virtual void report_sent(uint8_t dev_addr, uint8_t instance, uint8_t const* report, uint16_t len){}
	virtual void set_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len){}
	virtual void get_report_complete(uint8_t dev_addr, uint8_t instance, uint8_t report_id, uint8_t report_type, uint16_t len){}
private:
    void queue_host_report(void* report, uint16_t len);
	uint8_t xinput_dev_addr;
	uint8_t xinput_instance;
	bool dongle_ready;
	bool mounted;
};

#endif
